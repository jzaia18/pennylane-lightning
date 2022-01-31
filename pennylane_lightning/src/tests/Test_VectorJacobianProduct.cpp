#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>
#include <limits>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <catch2/catch.hpp>

#include "AdjointDiff.hpp"
#include "JacobianProd.hpp"
#include "StateVectorRaw.hpp"
#include "Util.hpp"

#include "TestHelpers.hpp"

using namespace Pennylane;
using namespace Pennylane::Algorithms;

/**
 * @brief Tests the constructability of the AdjointDiff.hpp classes.
 *
 */
TEMPLATE_TEST_CASE("VectorJacobianProduct::VectorJacobianProduct",
                   "[VectorJacobianProduct]", float, double) {
    SECTION("VectorJacobianProduct") {
        REQUIRE(std::is_constructible<VectorJacobianProduct<>>::value);
    }
    SECTION("VectorJacobianProduct<TestType> {}") {
        REQUIRE(std::is_constructible<VectorJacobianProduct<TestType>>::value);
    }
}

TEST_CASE("VectorJacobianProduct::vectorJacobianProduct Op=RX, Obs=Z dy={0}",
          "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;
    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};

    {
        const size_t num_qubits = 1;
        const size_t num_params = 3;
        const size_t num_obs = 1;
        auto obs = ObsDatum<double>({"PauliZ"}, {{}}, {{0}});
        std::vector<std::vector<double>> jacobian(
            num_obs, std::vector<double>(num_params, 0));
        std::vector<double> vjp_res(num_params);
        std::vector<double> dy(num_obs, 0);

        for (const auto &p : param) {
            auto ops = VJP.createOpsData({"RX"}, {{p}}, {{0}}, {false});

            std::vector<std::complex<double>> cdata(0b1 << num_qubits);
            cdata[0] = std::complex<double>{1, 0};

            StateVectorRaw<double> psi(cdata.data(), cdata.size());
            VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                      psi.getLength(), {obs}, ops, {0}, true);

            CAPTURE(jacobian);
            CHECK(0 == Approx(jacobian[0].front()));

            CAPTURE(vjp_res);
            CHECK(0 == Approx(vjp_res[0]));
            CHECK(0 == Approx(vjp_res[1]));
            CHECK(0 == Approx(vjp_res[2]));
        }
    }
}
TEST_CASE("VectorJacobianProduct::vectorJacobianProduct Op=RX, Obs=Z dy={1}",
          "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;
    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};

    {
        const size_t num_qubits = 1;
        const size_t num_params = 3;
        const size_t num_obs = 1;
        auto obs = ObsDatum<double>({"PauliZ"}, {{}}, {{0}});
        std::vector<std::vector<double>> jacobian(
            num_obs, std::vector<double>(num_params, 0));
        std::vector<double> vjp_res(num_params);
        std::vector<double> dy(num_obs, 1.0);

        for (const auto &p : param) {
            auto ops = VJP.createOpsData({"RX"}, {{p}}, {{0}}, {false});

            std::vector<std::complex<double>> cdata(0b1 << num_qubits);
            cdata[0] = std::complex<double>{1, 0};

            StateVectorRaw<double> psi(cdata.data(), cdata.size());
            VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                      psi.getLength(), {obs}, ops, {0}, true);

            CAPTURE(jacobian);
            CHECK(-sin(p) == Approx(jacobian[0].front()));

            CAPTURE(vjp_res);
            CHECK(-sin(p) == Approx(vjp_res[0]));
            CHECK(0 == Approx(vjp_res[1]));
            CHECK(0 == Approx(vjp_res[2]));
        }
    }
}
TEST_CASE("VectorJacobianProduct::vectorJacobianProduct Op=RX, Obs=Z dy={0.4}",
          "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;
    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};

    {
        const size_t num_qubits = 1;
        const size_t num_params = 3;
        const size_t num_obs = 1;
        auto obs = ObsDatum<double>({"PauliZ"}, {{}}, {{0}});
        std::vector<std::vector<double>> jacobian(
            num_obs, std::vector<double>(num_params, 0));
        std::vector<double> vjp_res(num_params);
        std::vector<double> dy(num_obs, 0.4);

        for (const auto &p : param) {
            auto ops = VJP.createOpsData({"RX"}, {{p}}, {{0}}, {false});

            std::vector<std::complex<double>> cdata(0b1 << num_qubits);
            cdata[0] = std::complex<double>{1, 0};

            StateVectorRaw<double> psi(cdata.data(), cdata.size());
            VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                      psi.getLength(), {obs}, ops, {0}, true);

            CAPTURE(jacobian);
            CHECK(-sin(p) == Approx(jacobian[0].front()));

            CAPTURE(vjp_res);
            CHECK(vjp_res.size() == num_params);
            CHECK(-sin(p) * 0.4 == Approx(vjp_res[0]));
            CHECK(0 == Approx(vjp_res[1]));
            CHECK(0 == Approx(vjp_res[2]));
        }
    }
}
TEST_CASE("VectorJacobianProduct::vectorJacobianProduct Op=RY, Obs=X dy={0.4}",
          "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;
    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};
    {
        const size_t num_qubits = 1;
        const size_t num_params = 3;
        const size_t num_obs = 1;

        auto obs = ObsDatum<double>({"PauliX"}, {{}}, {{0}});
        std::vector<std::vector<double>> jacobian(
            num_obs, std::vector<double>(num_params, 0));
        std::vector<double> vjp_res(num_params);
        std::vector<double> dy(num_obs, 0.4);

        for (const auto &p : param) {
            auto ops = VJP.createOpsData({"RY"}, {{p}}, {{0}}, {false});

            std::vector<std::complex<double>> cdata(0b1 << num_qubits);
            cdata[0] = std::complex<double>{1, 0};

            StateVectorRaw<double> psi(cdata.data(), cdata.size());
            VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                      psi.getLength(), {obs}, ops, {0}, true);

            CAPTURE(jacobian);
            CHECK(cos(p) == Approx(jacobian[0].front()).margin(1e-7));

            CAPTURE(vjp_res);
            CHECK(vjp_res.size() == num_params);
            CHECK(cos(p) * 0.4 == Approx(vjp_res[0]));
            CHECK(0 == Approx(vjp_res[1]));
            CHECK(0 == Approx(vjp_res[2]));
        }
    }
}
TEST_CASE(
    "VectorJacobianProduct::vectorJacobianProduct Op=RX, Obs=[Z,Z] dy={1}",
    "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;
    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};
    {
        const size_t num_qubits = 2;
        const size_t num_params = 1;
        const size_t num_obs = 2;
        std::vector<std::vector<double>> jacobian(
            num_obs, std::vector<double>(num_params, 0));
        std::vector<double> vjp_res(num_params);
        std::vector<double> dy(num_obs, 1);

        std::vector<std::complex<double>> cdata(0b1 << num_qubits);
        StateVectorRaw<double> psi(cdata.data(), cdata.size());
        cdata[0] = std::complex<double>{1, 0};

        auto obs1 = ObsDatum<double>({"PauliZ"}, {{}}, {{0}});
        auto obs2 = ObsDatum<double>({"PauliZ"}, {{}}, {{1}});

        auto ops = VJP.createOpsData({"RX"}, {{param[0]}}, {{0}}, {false});

        VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                  psi.getLength(), {obs1, obs2}, ops, {0},
                                  true);

        CAPTURE(jacobian);
        CHECK(-sin(param[0]) == Approx(jacobian[0][0]).margin(1e-7));
        CHECK(0.0 == Approx(jacobian[1][0]).margin(1e-7));

        CAPTURE(vjp_res);
        CHECK(vjp_res.size() == num_params);
        CHECK(-sin(param[0]) == Approx(vjp_res[0]));
    }
}
TEST_CASE("VectorJacobianProduct::vectorJacobianProduct Op=[RX,RX,RX], "
          "Obs=[Z,Z,Z], dy={0.4}",
          "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;
    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};
    {
        const size_t num_qubits = 3;
        const size_t num_params = 3;
        const size_t num_obs = 3;
        std::vector<std::vector<double>> jacobian(
            num_obs, std::vector<double>(num_params, 0));
        std::vector<double> vjp_res(num_params);
        std::vector<double> dy(num_obs, 0.4);

        std::vector<std::complex<double>> cdata(0b1 << num_qubits);
        StateVectorRaw<double> psi(cdata.data(), cdata.size());
        cdata[0] = std::complex<double>{1, 0};

        auto obs1 = ObsDatum<double>({"PauliZ"}, {{}}, {{0}});
        auto obs2 = ObsDatum<double>({"PauliZ"}, {{}}, {{1}});
        auto obs3 = ObsDatum<double>({"PauliZ"}, {{}}, {{2}});

        auto ops = VJP.createOpsData({"RX", "RX", "RX"},
                                     {{param[0]}, {param[1]}, {param[2]}},
                                     {{0}, {1}, {2}}, {false, false, false});

        VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                  psi.getLength(), {obs1, obs2, obs3}, ops,
                                  {0, 1, 2}, true);

        CAPTURE(jacobian);
        CHECK(-sin(param[0]) == Approx(jacobian[0][0]).margin(1e-7));
        CHECK(-sin(param[1]) == Approx(jacobian[1][1]).margin(1e-7));
        CHECK(-sin(param[2]) == Approx(jacobian[2][2]).margin(1e-7));

        CAPTURE(vjp_res);
        CHECK(vjp_res.size() == num_params);
        CHECK(-sin(param[0]) * 0.4 == Approx(vjp_res[0]));
        CHECK(-sin(param[1]) * 0.4 == Approx(vjp_res[1]));
        CHECK(-sin(param[2]) * 0.4 == Approx(vjp_res[2]));
    }
}
TEST_CASE(
    "VectorJacobianProduct::vectorJacobianProduct Op=[RX,RX,RX], Obs=[Z,Z,Z], "
    "TParams=[0,2], dy={1}",
    "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;
    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};
    {
        const size_t num_qubits = 3;
        const size_t num_params = 3;
        const size_t num_obs = 3;
        std::vector<std::vector<double>> jacobian(
            num_obs, std::vector<double>(num_params, 0));
        std::vector<size_t> t_params{0, 2};
        std::vector<double> vjp_res(num_params);
        std::vector<double> dy(num_obs, 1);

        std::vector<std::complex<double>> cdata(0b1 << num_qubits);
        StateVectorRaw<double> psi(cdata.data(), cdata.size());
        cdata[0] = std::complex<double>{1, 0};

        auto obs1 = ObsDatum<double>({"PauliZ"}, {{}}, {{0}});
        auto obs2 = ObsDatum<double>({"PauliZ"}, {{}}, {{1}});
        auto obs3 = ObsDatum<double>({"PauliZ"}, {{}}, {{2}});

        auto ops = VJP.createOpsData({"RX", "RX", "RX"},
                                     {{param[0]}, {param[1]}, {param[2]}},
                                     {{0}, {1}, {2}}, {false, false, false});

        VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                  psi.getLength(), {obs1, obs2, obs3}, ops,
                                  t_params, true);

        CAPTURE(jacobian);
        CHECK(-sin(param[0]) == Approx(jacobian[0][0]).margin(1e-7));
        CHECK(0 == Approx(jacobian[1][1]).margin(1e-7));
        CHECK(-sin(param[2]) == Approx(jacobian[2][1]).margin(1e-7));

        CAPTURE(vjp_res);
        CHECK(vjp_res.size() == num_params);
        CHECK(-sin(param[0]) == Approx(vjp_res[0]));
        CHECK(0 == Approx(vjp_res[2]));
        CHECK(-sin(param[2]) == Approx(vjp_res[1]));
    }
}
TEST_CASE("VectorJacobianProduct::vectorJacobianProduct Op=[RX,RX,RX], "
          "Obs=[ZZZ], dy={0.4}",
          "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;
    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};
    {
        const size_t num_qubits = 3;
        const size_t num_params = 3;
        const size_t num_obs = 1;
        std::vector<std::vector<double>> jacobian(
            num_obs, std::vector<double>(num_params, 0));
        std::vector<double> vjp_res(num_params);
        std::vector<double> dy(num_obs, 0.4);

        std::vector<std::complex<double>> cdata(0b1 << num_qubits);
        StateVectorRaw<double> psi(cdata.data(), cdata.size());
        cdata[0] = std::complex<double>{1, 0};

        auto obs = ObsDatum<double>({"PauliZ", "PauliZ", "PauliZ"},
                                    {{}, {}, {}}, {{0}, {1}, {2}});
        auto ops = VJP.createOpsData({"RX", "RX", "RX"},
                                     {{param[0]}, {param[1]}, {param[2]}},
                                     {{0}, {1}, {2}}, {false, false, false});

        VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                  psi.getLength(), {obs}, ops, {0, 1, 2}, true);

        CAPTURE(jacobian);
        CHECK(-0.1755096592645253 == Approx(jacobian[0][0]).margin(1e-7));
        CHECK(0.26478810666384334 == Approx(jacobian[0][1]).margin(1e-7));
        CHECK(-0.6312451595102775 == Approx(jacobian[0][2]).margin(1e-7));

        CAPTURE(vjp_res);
        CHECK(vjp_res.size() == num_params);
        CHECK(-0.1755096592645253 * 0.4 == Approx(vjp_res[0]));
        CHECK(0.26478810666384334 * 0.4 == Approx(vjp_res[1]));
        CHECK(-0.6312451595102775 * 0.4 == Approx(vjp_res[2]));
    }
}
TEST_CASE(
    "VectorJacobianProduct::vectorJacobianProduct Op=Mixed, Obs=[XXX], dy={1}",
    "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;
    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};
    {
        const size_t num_qubits = 3;
        const size_t num_params = 6;
        const size_t num_obs = 1;
        std::vector<std::vector<double>> jacobian(
            num_obs, std::vector<double>(num_params, 0));
        std::vector<double> vjp_res(num_params);
        std::vector<double> dy(num_obs, 1);

        std::vector<std::complex<double>> cdata(0b1 << num_qubits);
        StateVectorRaw<double> psi(cdata.data(), cdata.size());
        cdata[0] = std::complex<double>{1, 0};

        auto obs = ObsDatum<double>({"PauliX", "PauliX", "PauliX"},
                                    {{}, {}, {}}, {{0}, {1}, {2}});
        auto ops = VJP.createOpsData(
            {"RZ", "RY", "RZ", "CNOT", "CNOT", "RZ", "RY", "RZ"},
            {{param[0]},
             {param[1]},
             {param[2]},
             {},
             {},
             {param[0]},
             {param[1]},
             {param[2]}},
            {{0}, {0}, {0}, {0, 1}, {1, 2}, {1}, {1}, {1}},
            {false, false, false, false, false, false, false, false});

        VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                  psi.getLength(), {obs}, ops,
                                  {0, 1, 2, 3, 4, 5}, true);

        CAPTURE(jacobian);
        CHECK(0.0 == Approx(jacobian[0][0]).margin(1e-7));
        CHECK(-0.674214427 == Approx(jacobian[0][1]).margin(1e-7));
        CHECK(0.275139672 == Approx(jacobian[0][2]).margin(1e-7));
        CHECK(0.275139672 == Approx(jacobian[0][3]).margin(1e-7));
        CHECK(-0.0129093062 == Approx(jacobian[0][4]).margin(1e-7));
        CHECK(0.323846156 == Approx(jacobian[0][5]).margin(1e-7));

        CAPTURE(vjp_res);
        CHECK(vjp_res.size() == num_params);
        CHECK(0.0 == Approx(vjp_res[0]).margin(1e-7));
        CHECK(-0.674214427 == Approx(vjp_res[1]).margin(1e-7));
        CHECK(0.275139672 == Approx(vjp_res[2]).margin(1e-7));
        CHECK(0.275139672 == Approx(vjp_res[3]).margin(1e-7));
        CHECK(-0.0129093062 == Approx(vjp_res[4]).margin(1e-7));
        CHECK(0.323846156 == Approx(vjp_res[5]).margin(1e-7));
    }
}
TEST_CASE("VectorJacobianProduct::vectorJacobianProduct Op=Mixed, Obs=[XXX], "
          "dy={-0.2}",
          "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;
    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};
    {
        const size_t num_qubits = 3;
        const size_t num_params = 6;
        const size_t num_obs = 1;
        std::vector<std::vector<double>> jacobian(
            num_obs, std::vector<double>(num_params, 0));
        std::vector<double> vjp_res(num_params);
        std::vector<double> dy(num_obs, -0.2);

        std::vector<std::complex<double>> cdata(0b1 << num_qubits);
        StateVectorRaw<double> psi(cdata.data(), cdata.size());
        cdata[0] = std::complex<double>{1, 0};

        auto obs = ObsDatum<double>({"PauliX", "PauliX", "PauliX"},
                                    {{}, {}, {}}, {{0}, {1}, {2}});
        auto ops = VJP.createOpsData(
            {"RZ", "RY", "RZ", "CNOT", "CNOT", "RZ", "RY", "RZ"},
            {{param[0]},
             {param[1]},
             {param[2]},
             {},
             {},
             {param[0]},
             {param[1]},
             {param[2]}},
            {{0}, {0}, {0}, {0, 1}, {1, 2}, {1}, {1}, {1}},
            {false, false, false, false, false, false, false, false});

        VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                  psi.getLength(), {obs}, ops,
                                  {0, 1, 2, 3, 4, 5}, true);

        CAPTURE(jacobian);
        CHECK(0.0 == Approx(jacobian[0][0]).margin(1e-7));
        CHECK(-0.674214427 == Approx(jacobian[0][1]).margin(1e-7));
        CHECK(0.275139672 == Approx(jacobian[0][2]).margin(1e-7));
        CHECK(0.275139672 == Approx(jacobian[0][3]).margin(1e-7));
        CHECK(-0.0129093062 == Approx(jacobian[0][4]).margin(1e-7));
        CHECK(0.323846156 == Approx(jacobian[0][5]).margin(1e-7));

        CAPTURE(vjp_res);
        CHECK(vjp_res.size() == num_params);
        CHECK(-0.0 == Approx(vjp_res[0]).margin(1e-7));
        CHECK(-0.2 * -0.674214427 == Approx(vjp_res[1]).margin(1e-7));
        CHECK(-0.2 * 0.275139672 == Approx(vjp_res[2]).margin(1e-7));
        CHECK(-0.2 * 0.275139672 == Approx(vjp_res[3]).margin(1e-7));
        CHECK(-0.2 * -0.0129093062 == Approx(vjp_res[4]).margin(1e-7));
        CHECK(-0.2 * 0.323846156 == Approx(vjp_res[5]).margin(1e-7));
    }
}
TEST_CASE(
    "VectorJacobianProduct::vectorJacobianProduct Decomposed Rot gate, non "
    "computational basis state",
    "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;

    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};
    {
        const size_t num_qubits = 1;
        const size_t num_params = 3;
        const size_t num_obs = 1;

        const auto thetas = Util::linspace(-2 * M_PI, 2 * M_PI, 7);
        std::unordered_map<double, std::vector<double>> expec_results{
            {thetas[0], {0, -9.90819496e-01, 0}},
            {thetas[1], {-8.18996553e-01, 1.62526544e-01, 0}},
            {thetas[2], {-0.203949, 0.48593716, 0}},
            {thetas[3], {0, 1, 0}},
            {thetas[4], {-2.03948985e-01, 4.85937177e-01, 0}},
            {thetas[5], {-8.18996598e-01, 1.62526487e-01, 0}},
            {thetas[6], {0, -9.90819511e-01, 0}}};

        for (const auto &theta : thetas) {
            std::vector<double> local_params{theta, std::pow(theta, 3),
                                             SQRT2<double>() * theta};
            std::vector<std::vector<double>> jacobian(
                num_obs, std::vector<double>(num_params, 0));
            std::vector<double> vjp_res(num_params);
            std::vector<double> dy(num_obs, 1);

            std::vector<std::complex<double>> cdata{INVSQRT2<double>(),
                                                    -INVSQRT2<double>()};
            StateVectorRaw<double> psi(cdata.data(), cdata.size());

            auto obs = ObsDatum<double>({"PauliZ"}, {{}}, {{0}});
            auto ops = VJP.createOpsData(
                {"RZ", "RY", "RZ"},
                {{local_params[0]}, {local_params[1]}, {local_params[2]}},
                {{0}, {0}, {0}}, {false, false, false});

            VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                      psi.getLength(), {obs}, ops, {0, 1, 2},
                                      true);

            CAPTURE(theta);
            CAPTURE(jacobian);
            CHECK(expec_results[theta][0] ==
                  Approx(jacobian[0][0]).margin(1e-7));
            CHECK(expec_results[theta][1] ==
                  Approx(jacobian[0][1]).margin(1e-7));
            CHECK(expec_results[theta][2] ==
                  Approx(jacobian[0][2]).margin(1e-7));

            CAPTURE(vjp_res);
            CHECK(vjp_res.size() == num_params);
            CHECK(expec_results[theta][0] == Approx(vjp_res[0]).margin(1e-7));
            CHECK(expec_results[theta][1] == Approx(vjp_res[1]).margin(1e-7));
            CHECK(expec_results[theta][2] == Approx(vjp_res[2]).margin(1e-7));
        }
    }
}
TEST_CASE(
    "VectorJacobianProduct::vectorJacobianProduct Mixed Ops, Obs and TParams",
    "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;
    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};
    {
        const size_t num_qubits = 2;
        const std::vector<size_t> t_params{1, 2, 3};
        const size_t num_obs = 1;

        const auto thetas = Util::linspace(-2 * M_PI, 2 * M_PI, 8);

        std::vector<double> local_params{0.543, 0.54, 0.1,  0.5, 1.3,
                                         -2.3,  0.5,  -0.5, 0.5};
        std::vector<std::vector<double>> jacobian(
            num_obs, std::vector<double>(t_params.size(), 0));
        std::vector<double> vjp_res(t_params.size());
        std::vector<double> dy(num_obs, 1);

        std::vector<std::complex<double>> cdata{ONE<double>(), ZERO<double>(),
                                                ZERO<double>(), ZERO<double>()};
        StateVectorRaw<double> psi(cdata.data(), cdata.size());

        auto obs = ObsDatum<double>({"PauliX", "PauliZ"}, {{}, {}}, {{0}, {1}});
        auto ops = VJP.createOpsData(
            {"Hadamard", "RX", "CNOT", "RZ", "RY", "RZ", "RZ", "RY", "RZ", "RZ",
             "RY", "CNOT"},
            {{},
             {local_params[0]},
             {},
             {local_params[1]},
             {local_params[2]},
             {local_params[3]},
             {local_params[4]},
             {local_params[5]},
             {local_params[6]},
             {local_params[7]},
             {local_params[8]},
             {}},
            {{0}, {0}, {0, 1}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {1}, {0, 1}},
            {false, false, false, false, false, false, false, false, false,
             false, false, false});

        VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                  psi.getLength(), {obs}, ops, t_params, true);

        std::vector<double> expected{-0.71429188, 0.04998561, -0.71904837};
        CHECK(expected[0] == Approx(jacobian[0][0]));
        CHECK(expected[1] == Approx(jacobian[0][1]));
        CHECK(expected[2] == Approx(jacobian[0][2]));

        CAPTURE(vjp_res);
        CHECK(vjp_res.size() == t_params.size());
        CHECK(expected[0] == Approx(vjp_res[0]).margin(1e-7));
        CHECK(expected[1] == Approx(vjp_res[1]).margin(1e-7));
        CHECK(expected[2] == Approx(vjp_res[2]).margin(1e-7));
    }
}
TEST_CASE("VectorJacobianProduct::vectorJacobianProduct Mixed Ops, Obs and "
          "TParams, dy={-0.5}",
          "[VectorJacobianProduct]") {
    VectorJacobianProduct<double> VJP;
    std::vector<double> param{-M_PI / 7, M_PI / 5, 2 * M_PI / 3};
    {
        const size_t num_qubits = 2;
        const std::vector<size_t> t_params{1, 2, 3};
        const size_t num_obs = 1;

        const auto thetas = Util::linspace(-2 * M_PI, 2 * M_PI, 8);

        std::vector<double> local_params{0.543, 0.54, 0.1,  0.5, 1.3,
                                         -2.3,  0.5,  -0.5, 0.5};
        std::vector<std::vector<double>> jacobian(
            num_obs, std::vector<double>(t_params.size(), 0));
        std::vector<double> vjp_res(t_params.size());
        std::vector<double> dy(num_obs, -0.5);

        std::vector<std::complex<double>> cdata{ONE<double>(), ZERO<double>(),
                                                ZERO<double>(), ZERO<double>()};
        StateVectorRaw<double> psi(cdata.data(), cdata.size());

        auto obs = ObsDatum<double>({"PauliX", "PauliZ"}, {{}, {}}, {{0}, {1}});
        auto ops = VJP.createOpsData(
            {"Hadamard", "RX", "CNOT", "RZ", "RY", "RZ", "RZ", "RY", "RZ", "RZ",
             "RY", "CNOT"},
            {{},
             {local_params[0]},
             {},
             {local_params[1]},
             {local_params[2]},
             {local_params[3]},
             {local_params[4]},
             {local_params[5]},
             {local_params[6]},
             {local_params[7]},
             {local_params[8]},
             {}},
            {{0}, {0}, {0, 1}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {1}, {0, 1}},
            {false, false, false, false, false, false, false, false, false,
             false, false, false});

        VJP.vectorJacobianProduct(vjp_res, jacobian, dy, psi.getData(),
                                  psi.getLength(), {obs}, ops, t_params, true);

        std::vector<double> expected{-0.71429188, 0.04998561, -0.71904837};
        CHECK(expected[0] == Approx(jacobian[0][0]));
        CHECK(expected[1] == Approx(jacobian[0][1]));
        CHECK(expected[2] == Approx(jacobian[0][2]));

        CAPTURE(vjp_res);
        CHECK(vjp_res.size() == t_params.size());
        CHECK(-0.5 * expected[0] == Approx(vjp_res[0]).margin(1e-7));
        CHECK(-0.5 * expected[1] == Approx(vjp_res[1]).margin(1e-7));
        CHECK(-0.5 * expected[2] == Approx(vjp_res[2]).margin(1e-7));
    }
}