#pragma once

#include <array>
#include <concepts>
#include <stdexcept>
#include <vector>

class Quadrature {
public:
    explicit Quadrature(
        const std::pair<std::vector<double>, std::vector<double>>
            &points_and_weights)
        : m_points(points_and_weights.first),
          m_weights(points_and_weights.second),
          m_len(points_and_weights.first.size()) {
        if (m_points.size() != m_weights.size()) {
            throw std::runtime_error("points.size() != weights.size()");
        }
    }

    template <size_t N>
    explicit Quadrature(
        const std::pair<std::array<double, N>, std::array<double, N>>
            &points_and_weights)
        : m_points(points_and_weights.first.begin(),
                   points_and_weights.first.end()),
          m_weights(points_and_weights.second.begin(),
                    points_and_weights.second.end()),
          m_len(N) {}

    enum class Builtin {
        Legendre3,
        Legendre5,
        Legendre7,
        Lobatto3,
        Lobatto5,
        Lobatto7,
    };

    explicit Quadrature(Builtin type) {
        switch (type) {
        case Builtin::Legendre3:
            m_points = {0.774596669241483, 0, -0.774596669241483};
            m_weights = {0.555555555555556, 0.888888888888889,
                         0.555555555555556};
            m_len = 3;
            break;
        case Builtin::Legendre5:
            m_points = {0.906179845938664, 0.538469310105683,
                        -1.34940133673351e-79, -0.538469310105683,
                        -0.906179845938664};
            m_weights = {0.236926885056189, 0.478628670499366,
                         0.568888888888889, 0.478628670499366,
                         0.236926885056189};
            m_len = 5;
            break;
        case Builtin::Legendre7:
            m_points = {
                0.949107912342758,  0.741531185599394,  0.405845151377397, 0,
                -0.405845151377397, -0.741531185599394, -0.949107912342758};
            m_weights = {0.129484966168869, 0.279705391489277,
                         0.381830050505119, 0.417959183673469,
                         0.381830050505119, 0.279705391489277,
                         0.129484966168869};
            m_len = 7;
            break;
        case Builtin::Lobatto3:
            m_points = {1, 0, -1};
            m_weights = {0.333333333333333, 1.33333333333333,
                         0.333333333333333};
            m_len = 3;
            break;
        case Builtin::Lobatto5:
            m_points = {1, 0.654653670707977, 0, -0.654653670707977, -1};
            m_weights = {0.1, 0.544444444444444, 0.711111111111111,
                         0.544444444444444, 0.1};
            m_len = 5;
            break;
        case Builtin::Lobatto7:
            m_points = {1, 0.830223896278567,  0.468848793470714,
                        0, -0.468848793470714, -0.830223896278567,
                        -1};
            m_weights = {0.0476190476190476, 0.276826047361566,
                         0.431745381209863,  0.487619047619048,
                         0.431745381209863,  0.276826047361566,
                         0.0476190476190476};
            m_len = 7;
            break;
        default: throw std::invalid_argument("invalid type");
        }
    }

    // default: Gauss-Legendre 5 points
    Quadrature() : Quadrature(Builtin::Legendre5) {}

    struct Interval {
        const double xl;
        const double xr;

        // [xl,xr] -> [-1,1]
        double trans_to_local(double x) const {
            return -1 + (2 * (x - xl) / (xr - xl));
        }

        // [-1,1] -> [xl,xr]
        double trans_to_global(double x) const {
            return xl + ((xr - xl) * (x + 1) / 2);
        }
    };

    template <typename FuncType>
        requires std::invocable<FuncType, double>
                 && std::same_as<std::invoke_result_t<FuncType, double>, double>
    double intg(const FuncType &f, const Interval &the_interval) const {
        double result = 0;
        for (std::size_t i = 0; i < m_len; ++i) {
            double x = the_interval.trans_to_global(m_points[i]);
            result += m_weights[i] * f(x);
        }
        return ((the_interval.xr - the_interval.xl) / 2.0) * result;
    }

private:
    std::vector<double> m_points;
    std::vector<double> m_weights;
    std::size_t m_len;
};
