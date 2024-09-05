# Copyright 2018-2024 Xanadu Quantum Technologies Inc.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
r"""
Internal methods for adjoint Jacobian differentiation method.
"""

try:
    from pennylane_lightning.lightning_kokkos_ops.algorithms import (
        AdjointJacobianC64,
        AdjointJacobianC128,
        create_ops_listC64,
        create_ops_listC128,
    )
except ImportError:
    pass

from os import getenv
from typing import List

import numpy as np
import pennylane as qml
from pennylane import BasisState, QuantumFunctionError, StatePrep
from pennylane.measurements import Expectation, MeasurementProcess, State
from pennylane.operation import Operation
from pennylane.tape import QuantumTape

# pylint: disable=import-error, no-name-in-module, ungrouped-imports
from pennylane_lightning.core._serialize import QuantumScriptSerializer

from ._state_vector import LightningKokkosStateVector


class LightningKokkosAdjointJacobian:
    """Check and execute the adjoint Jacobian differentiation method.

    Args:
        qubit_state(LightningKokkosStateVector): State Vector to calculate the adjoint Jacobian with.
        batch_obs(bool): If serialized tape is to be batched or not.
    """

    def __init__(self, kokkos_state: LightningKokkosStateVector, batch_obs: bool = False) -> None:
        self._qubit_state = kokkos_state
        self._state = kokkos_state.state_vector
        self._dtype = kokkos_state.dtype
        self._jacobian_lightning = (
            AdjointJacobianC64() if self._dtype == np.complex64 else AdjointJacobianC128()
        )
        self._create_ops_list_lightning = (
            create_ops_listC64 if self._dtype == np.complex64 else create_ops_listC128
        )
        self._batch_obs = batch_obs

    @property
    def qubit_state(self):
        """Returns a handle to the LightningKokkosStateVector object."""
        return self._qubit_state

    @property
    def state(self):
        """Returns a handle to the Lightning internal data object."""
        return self._state

    @property
    def dtype(self):
        """Returns the simulation data type."""
        return self._dtype

    @staticmethod
    def _get_return_type(
        measurements: List[MeasurementProcess],
    ):
        """Get the measurement return type.

        Args:
            measurements (List[MeasurementProcess]): a list of measurement processes to check.

        Returns:
            None, Expectation or State: a common return type of measurements.
        """
        if not measurements:
            return None

        if len(measurements) == 1 and measurements[0].return_type is State:
            return State

        return Expectation

    def _process_jacobian_tape(
        self, tape: QuantumTape, use_mpi: bool = False, split_obs: bool = False
    ):
        """Process a tape, serializing and building a dictionary proper for
        the adjoint Jacobian calculation in the C++ layer.

        Args:
            tape (QuantumTape): Operations and measurements that represent instructions for execution on Lightning.
            use_mpi (bool, optional): If using MPI to accelerate calculation. Defaults to False.
            split_obs (bool, optional): If splitting the observables in a list. Defaults to False.

        Returns:
            dictionary: dictionary providing serialized data for Jacobian calculation.
        """
        use_csingle = self._dtype == np.complex64

        obs_serialized, obs_idx_offsets = QuantumScriptSerializer(
            self._qubit_state.device_name, use_csingle, use_mpi, split_obs
        ).serialize_observables(tape)

        ops_serialized, use_sp = QuantumScriptSerializer(
            self._qubit_state.device_name, use_csingle, use_mpi, split_obs
        ).serialize_ops(tape)

        ops_serialized = self._create_ops_list_lightning(*ops_serialized)

        # We need to filter out indices in trainable_params which do not
        # correspond to operators.
        trainable_params = sorted(tape.trainable_params)
        if len(trainable_params) == 0:
            return None

        tp_shift = []
        record_tp_rows = []
        all_params = 0

        for op_idx, trainable_param in enumerate(trainable_params):
            # get op_idx-th operator among differentiable operators
            operation, _, _ = tape.get_operation(op_idx)
            if isinstance(operation, Operation) and not isinstance(
                operation, (BasisState, StatePrep)
            ):
                # We now just ignore non-op or state preps
                tp_shift.append(trainable_param)
                record_tp_rows.append(all_params)
            all_params += 1

        if use_sp:
            # When the first element of the tape is state preparation. Still, I am not sure
            # whether there must be only one state preparation...
            tp_shift = [i - 1 for i in tp_shift]

        return {
            "state_vector": self.state,
            "obs_serialized": obs_serialized,
            "ops_serialized": ops_serialized,
            "tp_shift": tp_shift,
            "record_tp_rows": record_tp_rows,
            "all_params": all_params,
            "obs_idx_offsets": obs_idx_offsets,
        }

    @staticmethod
    def _adjoint_jacobian_processing(jac):
        """
        Post-process the Jacobian matrix returned by ``adjoint_jacobian`` for
        the new return type system.
        """
        jac = np.squeeze(jac)

        if jac.ndim == 0:
            return np.array(jac)

        if jac.ndim == 1:
            return tuple(np.array(j) for j in jac)

        # must be 2-dimensional
        return tuple(tuple(np.array(j_) for j_ in j) for j in jac)

    def _handle_raises(self, tape: QuantumTape, is_jacobian: bool, grad_vec=None):
        """Handle the raises related with the tape for computing the Jacobian with the adjoint method or the vector-Jacobian products."""

        if tape.shots:
            raise QuantumFunctionError(
                "Requested adjoint differentiation to be computed with finite shots. "
                "The derivative is always exact when using the adjoint "
                "differentiation method."
            )

        tape_return_type = self._get_return_type(tape.measurements)

        if is_jacobian:
            if not tape_return_type:
                # the tape does not have measurements
                return True

            if tape_return_type is State:
                raise QuantumFunctionError(
                    "Adjoint differentiation method does not support measurement StateMP."
                )

        if not is_jacobian:
            if qml.math.allclose(grad_vec, 0.0) or not tape_return_type:
                # the tape does not have measurements or the gradient is 0.0
                return True

            if tape_return_type is State:
                raise QuantumFunctionError(
                    "Adjoint differentiation does not support State measurements."
                )

        if any(m.return_type is not Expectation for m in tape.measurements):
            raise QuantumFunctionError(
                "Adjoint differentiation method does not support expectation return type "
                "mixed with other return types"
            )

        return False

    def calculate_jacobian(self, tape: QuantumTape):
        """Computes the Jacobian with the adjoint method.

        .. code-block:: python

            statevector = LightningKokkosStateVector(num_wires=num_wires)
            statevector = statevector.get_final_state(tape)
            jacobian = LightningKokkosAdjointJacobian(statevector).calculate_jacobian(tape)

        Args:
            tape (QuantumTape): Operations and measurements that represent instructions for execution on Lightning.

        Returns:
            The Jacobian of a tape.
        """

        empty_array = self._handle_raises(tape, is_jacobian=True)

        if empty_array:
            return np.array([], dtype=self._dtype)

        processed_data = self._process_jacobian_tape(tape)

        if not processed_data:  # training_params is empty
            return np.array([], dtype=self._dtype)

        trainable_params = processed_data["tp_shift"]
        jac = self._jacobian_lightning(
            processed_data["state_vector"],
            processed_data["obs_serialized"],
            processed_data["ops_serialized"],
            trainable_params,
        )
        jac = np.array(jac)
        jac = jac.reshape(-1, len(trainable_params)) if len(jac) else jac
        jac_r = np.zeros((jac.shape[0], processed_data["all_params"]))
        jac_r[:, processed_data["record_tp_rows"]] = jac

        return self._adjoint_jacobian_processing(jac_r)

    # pylint: disable=inconsistent-return-statements
    def calculate_vjp(self, tape: QuantumTape, grad_vec):
        """Compute the vector-Jacobian products of a tape.

        .. code-block:: python

            statevector = LightningKokkosStateVector(num_wires=num_wires)
            statevector = statevector.get_final_state(tape)
            vjp = LightningKokkosAdjointJacobian(statevector).calculate_vjp(tape, grad_vec)

        computes :math:`\\pmb{w} = (w_1,\\cdots,w_m)` where

        .. math::

            w_k = dy_k \\cdot J_{k,j}

        Here, :math:`dy` is the workflow cotangent (grad_vec), and :math:`J` the Jacobian.

        Args:
            tape (QuantumTape): Operations and measurements that represent instructions for execution on Lightning.
            grad_vec (tensor_like): Gradient-output vector, also called `dy` or cotangent. Must have shape matching the output
                shape of the corresponding tape, i.e. number of measurements if the return type is expectation.

        Returns:
            The vector-Jacobian products of a tape.
        """

        empty_array = self._handle_raises(tape, is_jacobian=False, grad_vec=grad_vec)

        if empty_array:
            return qml.math.convert_like(np.zeros(len(tape.trainable_params)), grad_vec)

        # Proceed, because tape_return_type is Expectation.
        if qml.math.ndim(grad_vec) == 0:
            grad_vec = (grad_vec,)

        if len(grad_vec) != len(tape.measurements):
            raise ValueError(
                "Number of observables in the tape must be the same as the "
                "length of grad_vec in the vjp method"
            )

        if np.iscomplexobj(grad_vec):
            raise ValueError(
                "The vjp method only works with a real-valued grad_vec when the "
                "tape is returning an expectation value"
            )

        ham = qml.simplify(qml.dot(grad_vec, [m.obs for m in tape.measurements]))

        num_params = len(tape.trainable_params)

        if num_params == 0:
            return np.array([], dtype=self.qubit_state.dtype)

        new_tape = qml.tape.QuantumScript(
            tape.operations,
            [qml.expval(ham)],
            shots=tape.shots,
            trainable_params=tape.trainable_params,
        )

        return self.calculate_jacobian(new_tape)