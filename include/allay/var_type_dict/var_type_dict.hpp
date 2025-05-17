#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>

namespace detail {

//----------------------------------------------------------------------------//
// 根据输入的N，创建使用N个NullType占位的“元组”类型
// 例如N=3，TypeListCreate<3,TCont>，得到的就是TCont<NullType,NullType,NullType>

struct NullType {};  // 用于占位的空类型

// 递降构造
template <size_t N, template <typename...> class TCont, typename... T>
struct TypeListCreate {
    using Type = typename TypeListCreate<N - 1, TCont, NullType, T...>::Type;
};

// 递降到0结束
template <template <typename...> class TCont, typename... T>
struct TypeListCreate<0, TCont, T...> {
    using Type = TCont<T...>;
};

//----------------------------------------------------------------------------//
// 根据索引类型获取对应值的索引整数，需要提供后面的类型列表的开始索引值（通常取为0）
// 例如
// Tag2Id<A,0,A,B,C>::Value = 0, Tag2Id<B,0,A,B,C>::Value = 1

// 递增查找，N变为N+1，同时移除一个类型
template <typename TFindTag, size_t N, typename TCurTag, typename... TTags>
struct Tag2Pos {
    constexpr static size_t Value = Tag2Pos<TFindTag, N + 1, TTags...>::Value;
};

// 当第一个类型和第三个类型相同时返回，返回此时的整数N
template <typename TFindTag, size_t N, typename... TTags>
struct Tag2Pos<TFindTag, N, TFindTag, TTags...> {
    constexpr static size_t Value = N;
};

//----------------------------------------------------------------------------//
// 根据位置整数查找参数类型
// 例如
// Pos2Type<0,A,B,C>::Type == A
// Pos2Type<2,A,B,C>::Type == C

template <size_t Pos, typename... TTags>
struct Pos2Type {
    static_assert((Pos != 0), "Invalid position.");  // 查找失败
};

// 递降查找
template <size_t Pos, typename TCur, typename... TTags>
struct Pos2Type<Pos, TCur, TTags...> {
    using Type = Pos2Type<Pos - 1, TTags...>::Type;
};

// 查找到0停止（优先匹配这个）
template <typename TCur, typename... TTags>
struct Pos2Type<0, TCur, TTags...> {
    using Type = TCur;
};

//----------------------------------------------------------------------------//
// 构造新的元组类型
// TCont<NullType,NullType,NullType>
// 在提供的索引位置上替换为TVal类型（还需要提供后续列表的索引起点，通常取0）
// 例如逐步生成
// TCont<TVal1,NullType,NullType>
// TCont<TVal1,TVal2,NullType>
// TCont<TVal1,TVal2,TVal3>

template <typename TVal, size_t N, size_t M, typename TProcessedTypes,
          typename... TRemainTypes>
struct NewTupleType;

// 对参数M递增调用
// TModifiedTypes是TCurType已经吸收的类型
// TRemainTypes是TCurType尚未吸收的类型
// M不等于N时，将当前M位置对应的TCurType类型吸收进TCont
// 从TCont<TModifiedTypes...>变成TCont<TModifiedTypes..., TCurType>
template <typename TVal, size_t N, size_t M, template <typename...> class TCont,
          typename... TModifiedTypes, typename TCurType,
          typename... TRemainTypes>
struct NewTupleType<TVal, N, M, TCont<TModifiedTypes...>, TCurType,
                    TRemainTypes...> {
    using Type = typename NewTupleType<TVal, N, M + 1,
                                       TCont<TModifiedTypes..., TCurType>,
                                       TRemainTypes...>::Type;
};

// 参数M=N时递归停止，
// TModifiedTypes是TCurType已经吸收的类型
// TRemainTypes是TCurType尚未吸收的类型
// 在当前M=N时，将TVal类型吸收进TCont
// 然后将剩下的类型全部吸收进TCont
// 得到的类型为TCont<TModifiedTypes..., TVal,TRemainTypes...>
template <typename TVal, size_t N, template <typename...> class TCont,
          typename... TModifiedTypes, typename TCurType,
          typename... TRemainTypes>
struct NewTupleType<TVal, N, N, TCont<TModifiedTypes...>, TCurType,
                    TRemainTypes...> {
    using Type = TCont<TModifiedTypes..., TVal, TRemainTypes...>;
};

//----------------------------------------------------------------------------//

}  // end namespace detail

// 异类参数词典，在这个层面存储的是索引类型序列
template <typename... TParameters>
struct VarTypeDict {
    template <typename... TTypes>
    struct Values {
        Values() = default;

        // 输入一个std::shared_ptr<void>数组，移动存储到Values的私有成员中
        // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
        explicit Values(std::shared_ptr<void> (&&input)[sizeof...(TTypes)]) {
            for (size_t i = 0; i < sizeof...(TTypes); ++i) {
                m_tuple[i] = std::move(input[i]);
            }
        }

        // 设置参数，需要提供索引类型和对应的值
        // 返回的是新的Value类型对象
        template <typename TTag, typename TVal>
        auto set(TVal &&val) && {
            // 根据TTag获取索引位置
            constexpr static size_t TagPos =
                detail::Tag2Pos<TTag, 0, TParameters...>::Value;

            // 存储参数值，万能转发，并且通过共享指针调用
            using RawVal = std::decay_t<TVal>;
            auto *tmp = new RawVal(std::forward<TVal>(val));
            m_tuple[TagPos] = std::shared_ptr<void>(tmp, [](void *ptr) {
                auto *nptr = static_cast<RawVal *>(ptr);
                delete nptr;
            });

            // 返回的是新的Value对象（候选列表索引从0开始）
            using NewValueType =
                detail::NewTupleType<RawVal, TagPos, 0, Values<>,
                                     TTypes...>::Type;
            return NewValueType(std::move(m_tuple));
        }

        // 获取参数，需要提供索引类型
        template <typename TTag>
        auto &get() const {
            // 查找索引类型TTag在TParameters中的位置
            constexpr static size_t TagPos =
                detail::Tag2Pos<TTag, 0, TParameters...>::Value;
            // 根据索引位置获取参数指针(被共享指针包裹的内容)
            void *tmp = m_tuple[TagPos].get();

            // 根据索引位置获取参数类型
            using AimType = detail::Pos2Type<TagPos, TTypes...>::Type;
            // 转换指针为参数类型并返回
            auto *res = static_cast<AimType *>(tmp);
            return *res;
        }

        // 获取参数类型，需要提供索引类型（候选列表索引从0开始）
        template <typename TTag>
        using ValueType =
            detail::Pos2Type<detail::Tag2Pos<TTag, 0, TParameters...>::Value,
                             TTypes...>::Type;

    private:
        // values通过std::shared_ptr<void>数组来存储参数
        std::shared_ptr<void> m_tuple[sizeof...(TTypes)];
    };

    // 初始化一个指定长度的values对象
    // values的所有类型参数都是空类型，暂时用于占位
    static auto init() {
        using EmptyValueType =
            typename detail::TypeListCreate<sizeof...(TParameters),
                                            Values>::Type;
        return EmptyValueType{};
    }
};
