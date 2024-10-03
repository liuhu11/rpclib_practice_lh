#ifndef FUNC_TRAITS_H_RPC
#define FUNC_TRAITS_H_RPC

#include <cstdint>
#include <tuple>
#include <type_traits>

#include "constant.h"
#include "invoke.h"

namespace rpc::detail {
// 在模板编程中，通常需要类型而不是值
template <int N>
using is_zero = std::conditional_t<N == 0, true_, false_>;

template <int N, typename... Types>
using nth_type = invoke_type<std::tuple_element<N, std::tuple<Types...>>>;

namespace tags {
// tags for the function traits, used for tag dispatching
struct zero_arg{};
struct nonzero_arg{};
struct void_result{};
struct nonvoid_result{};
template <int N>
struct arg_count_trait {
    using type = nonzero_arg;
};
// 注意怎么特化的-类型参数N被模板形参0取代
template <>
struct arg_count_trait<0> {
    using type = zero_arg;
};

// 利用别名模板定义_t版本
template <int N>
using arg_count_trait_t = arg_count_trait<N>::type;

template <typename T>
struct result_trait {
    using type = nonvoid_result;
};

template <>
struct result_trait<void> {
    using type = void_result;
};

template <typename T>
using result_trait_t = result_trait<T>;
}

//! \brief Provides a small function traits implementation that
//! works with a reasonably large set of functors.
template <typename T>
struct func_traits : func_traits<decltype(&T::operator())> {};

template <typename T, typename R, typename... Args>
struct func_traits<R (T::*)(Args...)> : func_traits<R(*)(Args...)>{};

template <typename T, typename R, typename... Args>
struct func_traits<R(T::*)(Args...) const> : func_traits<R(*)(Args...)>{};

template <typename R, typename... Args>
struct func_traits<R(*)(Args...)> {
    using result_type = R;
    using arg_count = constant<size_t, sizeof...(Args)>;
    using args_type = std::tuple<typename std::decay_t<Args>...>;
};

template <typename T>
struct func_kind_info : func_kind_info<decltype(&T::operator())>{};

template <typename T, typename R, typename... Args>
struct func_kind_info<R(T::*)(Args...)> : func_kind_info<R(*)(Args...)>{};

template <typename T, typename R, typename... Args>
struct func_kind_info<R(T::*)(Args...) const> : func_kind_info<R(*)(Args...)>{};

template <typename R, typename... Args>
struct func_kind_info<R(*)(Args...)> {
    using args_kind = typename tags::arg_count_trait_t<sizeof...(Args)>;
    using result_kind = typename tags::result_trait_t<R>;
};

// std::integral_constant 可以隐式转换成对应类型的变量
template <typename Func>
using is_zero_arg = is_zero<func_traits<Func>::arg_count::value>;

template <typename Func>
using is_single_arg = std::conditional_t<func_traits<Func>::arg_count == 1, true_, false_>;

template <typename Func>
using is_void_result = std::is_void<func_traits<Func>::result_type>;
}

#endif