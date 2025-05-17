#pragma once

#include <array>
#include <concepts>
#include <stdexcept>
#include <vector>

class Quadrature3 {
public:
    explicit Quadrature3(
        const std::pair<std::vector<double>, std::vector<double>>
            &points_and_weights)
        : m_points(points_and_weights.first),
          m_weights(points_and_weights.second),
          m_len(points_and_weights.first.size()) {
        if (m_points.size() != 3 * m_weights.size()) {
            throw std::runtime_error("points.size() != 3*weights.size()");
        }
    }

    template <size_t N>
    explicit Quadrature3(
        const std::pair<std::array<double, N>, std::array<double, 3 * N>>
            &points_and_weights)
        : m_points(points_and_weights.first.begin(),
                   points_and_weights.first.end()),
          m_weights(points_and_weights.second.begin(),
                    points_and_weights.second.end()),
          m_len(N) {}

    enum class Builtin {
        P1,
        P3,
        P7,
        P12,
    };

    explicit Quadrature3(Builtin type) {
        switch (type) {
        case Builtin::P1:
            m_weights = {1.0};
            m_points = {1.0 / 3, 1.0 / 3, 1.0 / 3};
            m_len = 1;
            break;
        case Builtin::P3:
            m_weights = {1.0 / 3, 1.0 / 3, 1.0 / 3};
            m_points = {
                2.0 / 3, 1.0 / 6, 1.0 / 6,  // p1
                1.0 / 6, 2.0 / 3, 1.0 / 6,  // p2
                1.0 / 6, 1.0 / 6, 2.0 / 3,  // p3
            };
            m_len = 3;
            break;
        case Builtin::P7:
            m_weights = {
                0.225,  // 1
                0.125939180544827,
                0.125939180544827,
                0.125939180544827,  // 3
                0.132394152788506,
                0.132394152788506,
                0.132394152788506,  // 3
            };
            m_points = {
                1.0 / 3, 1.0 / 3, 1.0 / 3,                                // p
                                                                          //
                0.797426985353087, 0.101286507323456, 0.101286507323456,  // q1
                0.101286507323456, 0.797426985353087, 0.101286507323456,  // q2
                0.101286507323456, 0.101286507323456, 0.797426985353087,  // q3
                                                                          //
                0.059715871789770, 0.470142064105115, 0.470142064105115,  // r1
                0.470142064105115, 0.059715871789770, 0.470142064105115,  // r2
                0.470142064105115, 0.470142064105115, 0.059715871789770,  // r3
            };
            m_len = 7;
            break;
        case Builtin::P12:
            m_weights = {
                0.050844906370207, 0.050844906370207, 0.050844906370207,  // 3
                0.116786275726379, 0.116786275726379, 0.116786275726379,  // 3
                0.082851075618374, 0.082851075618374, 0.082851075618374,
                0.082851075618374, 0.082851075618374, 0.082851075618374,  // 6
            };
            m_points = {
                0.873821971016996, 0.063089014491502, 0.063089014491502,  // p1
                0.063089014491502, 0.873821971016996, 0.063089014491502,  // p2
                0.063089014491502, 0.063089014491502, 0.873821971016996,  // p3
                                                                          //
                0.501426509658179, 0.249286745170910, 0.249286745170911,  // q1
                0.249286745170910, 0.501426509658179, 0.249286745170911,  // q2
                0.249286745170910, 0.249286745170911, 0.501426509658179,  // q3
                                                                          //
                0.636502499121399, 0.310352451033785, 0.053145049844816,  // r1
                0.636502499121399, 0.053145049844816, 0.310352451033785,  // r2
                0.310352451033785, 0.636502499121399, 0.053145049844816,  // r3
                0.310352451033785, 0.053145049844816, 0.636502499121399,  // r4
                0.053145049844816, 0.636502499121399, 0.310352451033785,  // r5
                0.053145049844816, 0.310352451033785, 0.636502499121399,  // r6
            };
            m_len = 12;
            break;
        default: throw std::invalid_argument("invalid type");
        }
    }

    // default: 7 points
    Quadrature3() : Quadrature3(Builtin::P7) {}

    struct PointXY {
        double x;
        double y;
    };

    // Barycentric Coordinates
    struct PointCoordinate {
        double p1;
        double p2;
        double p3;
    };

    struct Triangle {
        const double ax;
        const double ay;
        const double bx;
        const double by;
        const double cx;
        const double cy;

        PointXY trans_to_xy(double c1, double c2, double c3) const {
            double x = c1 * ax + c2 * bx + c3 * cx;
            double y = c1 * ay + c2 * by + c3 * cy;
            return PointXY{.x = x, .y = y};
        }

        PointCoordinate trans_to_coordinate(double x, double y) const {
            double p3 = ((ay - by) * x + (bx - ax) * y + ax * by - bx * ay)
                        / ((ay - by) * cx + (bx - ax) * cy + ax * by - bx * ay);

            double p2 = ((ay - cy) * x + (cx - ax) * y + ax * cy - cx * ay)
                        / ((ay - cy) * bx + (cx - ax) * by + ax * cy - cx * ay);

            double p1 = 1 - p2 - p3;

            return PointCoordinate{.p1 = p1, .p2 = p2, .p3 = p3};
        }

        double area() const {
            return triangle_signed_area(ax, ay, bx, by, cx, cy);
        }

        static double triangle_signed_area(double x1, double y1, double x2,
                                           double y2, double x3, double y3) {
            double px = x2 - x1;
            double py = y2 - y1;
            double qx = x3 - x1;
            double qy = y3 - y1;

            return (px * qy - py * qx) / 2;
        }
    };

    template <typename FuncType>
        requires std::invocable<FuncType, double, double>
                 && std::same_as<std::invoke_result_t<FuncType, double, double>,
                                 double>
    double intg(const FuncType &f, const Triangle &the_triangle) const {
        double result = 0;
        for (std::size_t i = 0; i < m_len; ++i) {
            auto [x, y] = the_triangle.trans_to_xy(
                m_points[3 * i], m_points[3 * i + 1], m_points[3 * i + 2]);
            result += m_weights[i] * f(x, y);
        }
        return the_triangle.area() * result;
    }

private:
    std::vector<double> m_points;
    std::vector<double> m_weights;
    std::size_t m_len;
};
