#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <numbers>
#include <vector>

// Gauss-Lobatto nodes and weights (runtime version)
inline auto gausslobatto(unsigned int n)
    -> std::pair<std::vector<double>, std::vector<double>> {
    assert(n >= 2);
    constexpr double pi = std::numbers::pi;

    std::vector<double> x(n);
    std::vector<double> w(n);

    // Initial guess for nodes
    for (unsigned int i = 0; i < n; ++i) { x[i] = std::cos(pi * i / (n - 1)); }

    std::vector<std::vector<double>> L;
    L.resize(n);
    for (unsigned int i = 0; i < n; ++i) { L[i].resize(n); }

    double eps = std::numeric_limits<double>::epsilon();
    while (true) {
        for (unsigned int i = 0; i < n; ++i) {
            L[0][i] = 1.0;
            L[1][i] = x[i];
        }

        for (unsigned int k = 2; k <= n - 1; ++k) {
            for (unsigned int i = 0; i < n; ++i) {
                L[k][i] = (2.0 * k - 1.0) / k * x[i] * L[k - 1][i]
                          - (k - 1.0) / k * L[k - 2][i];
            }
        }

        std::vector<double> dx(n, 0.0);
        for (unsigned int i = 0; i < n; ++i) {
            dx[i] = (x[i] * L[n - 1][i] - L[n - 2][i]) / (n * L[n - 1][i]);
            x[i] -= dx[i];
            dx[i] = std::abs(dx[i]);
        }

        if (*std::ranges::max_element(dx) < eps) { break; }
    }

    // Compute weights
    for (unsigned int i = 0; i < n; ++i) {
        w[i] = 2.0 / ((n - 1) * n * (L[n - 1][i] * L[n - 1][i]));
    }

    return {x, w};
}

// Gauss-Lobatto nodes and weights (compile time version)
template <unsigned int N>
consteval auto
gausslobatto() -> std::pair<std::array<double, N>, std::array<double, N>> {
    static_assert(N >= 2, "N must be >= 2");
    constexpr double pi = std::numbers::pi;

    constexpr auto constexpr_cos = [](double x) {
        const unsigned int terms = 20;

        double result = 1.0;
        double term = 1.0;
        double x2 = x * x;
        for (unsigned int k = 1; k < terms; ++k) {
            term *= -x2 / (2 * k * (2 * k - 1));
            result += term;
        }
        return result;
    };

    // constexpr auto constexpr_sin = [](double x) {
    //     const unsigned int  terms = 20;

    //     double result = x;
    //     double term = x;
    //     double x2 = x * x;
    //     for (unsigned int  k = 1; k < terms; ++k) {
    //         term *= -x2 / ((2 * k + 1) * (2 * k));
    //         result += term;
    //     }
    //     return result;
    // };

    constexpr auto constexpr_abs = [](double x) { return x < 0 ? -x : x; };

    std::array<double, N> x{};
    std::array<double, N> w{};

    const auto n = N;

    // Initial guess for nodes
    for (unsigned int i = 0; i < n; ++i) {
        x[i] = constexpr_cos(pi * i / (n - 1));
    }

    std::array<std::array<double, N>, N> L{};

    double eps = std::numeric_limits<double>::epsilon();
    while (true) {
        for (unsigned int i = 0; i < n; ++i) {
            L[0][i] = 1.0;
            L[1][i] = x[i];
        }

        for (unsigned int k = 2; k <= n - 1; ++k) {
            for (unsigned int i = 0; i < n; ++i) {
                L[k][i] = (2.0 * k - 1.0) / k * x[i] * L[k - 1][i]
                          - (k - 1.0) / k * L[k - 2][i];
            }
        }

        std::array<double, N> dx{};
        for (unsigned int i = 0; i < n; ++i) {
            dx[i] = (x[i] * L[n - 1][i] - L[n - 2][i]) / (n * L[n - 1][i]);
            x[i] -= dx[i];
            dx[i] = constexpr_abs(dx[i]);
        }

        if (*std::ranges::max_element(dx) < eps) { break; }
    }

    // Compute weights
    for (unsigned int i = 0; i < n; ++i) {
        w[i] = 2.0 / ((n - 1) * n * (L[n - 1][i] * L[n - 1][i]));
    }

    return {x, w};
}
