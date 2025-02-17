#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <numbers>
#include <vector>

// Gauss-Legendre nodes and weights (runtime version)
inline auto gausslegendre(unsigned int n)
    -> std::pair<std::vector<double>, std::vector<double>> {
    assert(n >= 2);
    constexpr double pi = std::numbers::pi;

    std::vector<double> x(n);
    std::vector<double> w(n);

    // Initial guess for nodes
    for (unsigned int i = 0; i < n; ++i) {
        x[i] = std::cos(pi * (2.0 * (i + 1) - 1.0) / (2.0 * n))
               + (0.27 / n)
                     * std::sin(pi * (2.0 * i - (n - 1.0)) / (n - 1.0)
                                * (n - 1.0) / (n + 1.0));
    }

    std::vector<std::vector<double>> L;
    L.resize(n + 1);
    for (unsigned int i = 0; i < n + 1; ++i) { L[i].resize(n); }

    std::vector<double> dL(n);
    double eps = std::numeric_limits<double>::epsilon();
    while (true) {
        for (unsigned int i = 0; i < n; ++i) {
            L[0][i] = 1.0;
            L[1][i] = x[i];
        }

        for (unsigned int k = 2; k <= n; ++k) {
            for (unsigned int i = 0; i < n; ++i) {
                L[k][i] = (2.0 * k - 1.0) / k * x[i] * L[k - 1][i]
                          - (k - 1.0) / k * L[k - 2][i];
            }
        }

        for (unsigned int i = 0; i < n; ++i) {
            dL[i] = (n / (x[i] * x[i] - 1.0)) * (x[i] * L[n][i] - L[n - 1][i]);
        }

        std::vector<double> dx(n, 0.0);
        for (unsigned int i = 0; i < n; ++i) {
            dx[i] = L[n][i] / dL[i];
            x[i] -= dx[i];
            dx[i] = std::abs(dx[i]);
        }

        if (*std::ranges::max_element(dx) < eps) { break; }
    }

    // Compute weights
    for (unsigned int i = 0; i < n; ++i) {
        w[i] = 2.0 / ((1.0 - x[i] * x[i]) * dL[i] * dL[i]);
    }

    return {x, w};
}

// Gauss-Legendre nodes and weights (compile time version)
template <unsigned int N>
consteval auto
gausslegendre() -> std::pair<std::array<double, N>, std::array<double, N>> {
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

    constexpr auto constexpr_sin = [](double x) {
        const unsigned int terms = 20;

        double result = x;
        double term = x;
        double x2 = x * x;
        for (unsigned int k = 1; k < terms; ++k) {
            term *= -x2 / ((2 * k + 1) * (2 * k));
            result += term;
        }
        return result;
    };

    constexpr auto constexpr_abs = [](double x) { return x < 0 ? -x : x; };

    std::array<double, N> x{};
    std::array<double, N> w{};

    const auto n = N;

    // Initial guess for nodes
    for (unsigned int i = 0; i < n; ++i) {
        x[i] = constexpr_cos(pi * (2.0 * (i + 1) - 1.0) / (2.0 * n))
               + (0.27 / n)
                     * constexpr_sin(pi * (2.0 * i - (n - 1.0)) / (n - 1.0)
                                     * (n - 1.0) / (n + 1.0));
    }

    std::array<std::array<double, N>, N + 1> L{};

    std::array<double, N> dL{};
    double eps = std::numeric_limits<double>::epsilon();
    while (true) {
        for (unsigned int i = 0; i < n; ++i) {
            L[0][i] = 1.0;
            L[1][i] = x[i];
        }

        for (unsigned int k = 2; k <= n; ++k) {
            for (unsigned int i = 0; i < n; ++i) {
                L[k][i] = (2.0 * k - 1.0) / k * x[i] * L[k - 1][i]
                          - (k - 1.0) / k * L[k - 2][i];
            }
        }

        for (unsigned int i = 0; i < n; ++i) {
            dL[i] = (n / (x[i] * x[i] - 1.0)) * (x[i] * L[n][i] - L[n - 1][i]);
        }

        std::array<double, N> dx{};
        for (unsigned int i = 0; i < n; ++i) {
            dx[i] = L[n][i] / dL[i];
            x[i] -= dx[i];
            dx[i] = constexpr_abs(dx[i]);
        }

        if (*std::ranges::max_element(dx) < eps) { break; }
    }

    // Compute weights
    for (unsigned int i = 0; i < n; ++i) {
        w[i] = 2.0 / ((1.0 - x[i] * x[i]) * dL[i] * dL[i]);
    }

    return {x, w};
}
