#include "allay/var_type_dict/var_type_dict.hpp"
#include <cmath>
#include <iostream>

namespace {

using Params1 = VarTypeDict<struct A, struct B, struct C, struct Weight>;

template <typename TIn>
auto fun(const TIn &in) {
    auto a = in.template get<A>();
    auto b = in.template get<B>();
    auto weight = in.template get<Weight>();

    using TypeOfB = TIn::template ValueType<B>;
    static_assert(std::is_same_v<TypeOfB, double>);

    return a * weight + b * (1 - weight);
}

int test_var_type_dict_1() {
    std::cout << "Test named params 1\t";
    auto res = fun(Params1::init().set<B>(2.4).set<A>(1.3).set<Weight>(0.1));
    if (std::fabs(res - 0.1 * 1.3 - 0.9 * 2.4) >= 1e-8) {
        std::cout << "Test failed!\n";
        return 1;
    }
    std::cout << "Test passed!\n";

    auto p = Params1::init().set<B>(2.4).set<A>(1.3).set<Weight>(0.1);
    std::cout << "Test named params 2\t";
    auto res3 = fun(p);
    if (std::fabs(res3 - 0.1 * 1.3 - 0.9 * 2.4) >= 1e-8) {
        std::cout << "Test failed!\n";
        return 1;
    }
    std::cout << "Test passed!\n";

    return 0;
}

using Params2 = VarTypeDict<struct D, struct E, struct F, struct G>;

int test_var_type_dict_2() {
    std::cout << "Test initialization and setting values...\t";

    auto params = Params2::init().set<G>(2.4).set<D>(10).set<F>(0.1).set<E>(
        std::string{"abc"});
    if (params.template get<G>() == 2.4 && params.template get<D>() == 10
        && params.template get<F>() == 0.1
        && params.template get<E>() == "abc") {
        std::cout << "Passed.\n";
    }
    else {
        std::cout << "Failed.\n";
        return 1;
    }

    return 0;
}
}  // namespace

int main() { return test_var_type_dict_1() + test_var_type_dict_2(); }
