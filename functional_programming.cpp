#include <concepts>
#include <type_traits>
#include <ranges>
#include <array>
#include <vector>
#include <algorithm>
#include "template_arithmetics.h"

namespace ranges = std::ranges;
namespace views = std::views;

template<typename... Ts>
struct [[deprecated]] PrintType;
template<auto V>
struct [[deprecated]] PrintValue;

template<int n>
using Int = std::integral_constant<int, n>::type;
template<double n>
using Double = std::integral_constant<double, n>::type;
template<size_t n>
using SizeT = std::integral_constant<size_t, n>::type;

template<typename Ty>
struct Unwrap { using type = Ty; };

template<typename Ty>
using unwrap = typename Unwrap<Ty>::type;

template<template<typename...>typename Fn, typename... Args>
struct Bind
{
	template<typename... Placeholders>
	using type = Fn<Args..., Placeholders...>;
};

template<typename Fn>
using result = typename Fn::type;

constexpr static size_t NPOS = static_cast<size_t>(-1);
using NPOST = std::integral_constant<size_t, NPOS>::type;

template<template<auto, auto>typename Operator, auto InitialValue, auto UnpackedValue, auto... Values>
struct Reduce
{
	constexpr static auto value = Reduce<Operator, Operator<InitialValue, UnpackedValue>::value, Values...>::value;
};
template<template<auto, auto>typename Operator, auto InitialValue, auto UnpackedValue>
struct Reduce<Operator, InitialValue, UnpackedValue>
{
	constexpr static auto value = Operator<InitialValue, UnpackedValue>::value;
};
template<template<typename, typename>typename,typename,  typename...>
struct TypeReduce;

template<template<typename, typename>typename Operator, typename Acc, typename Unpacked, typename... Ts>
struct TypeReduce<Operator, Acc, Unpacked, Ts...>
{
	using type = TypeReduce<Operator, typename Operator<Acc, Unpacked>::type, Ts...>::type;
};
template<template<typename, typename>typename Operator, typename Acc>
struct TypeReduce<Operator, Acc>
{
	using type = Acc;
};

template<template<typename, typename>typename Operator, typename Initial, typename... Ts>
using type_reduce_t = typename TypeReduce<Operator, Initial, Ts...>::type;

template<template<auto>typename Operator, template<auto...>typename Into, auto... Vs>
struct Map { using type = Into<Operator<Vs>::value...>; };

template<auto Value>
struct Expression { constexpr static auto value = Value; };

template<typename... Types>
struct TypeArray
{
	template<typename... Us>
	using Template = TypeArray<Us...>;
	using type = TypeArray;
	constexpr static bool is_type_array{ true };
	using tuple_type = std::tuple<Types...>;
	template<size_t I>
	using at = std::tuple_element_t<I, tuple_type>;
	template<template<typename...> typename U>
	using into = U<Types...>;
	template<typename... Us>
	using append = TypeArray<Types..., Us...>;
	template<typename... Us>
	using prepend = TypeArray<Us..., Types...>;
	
	template<typename>
	struct AppendArray;
	template<typename... Ts>
	struct AppendArray<TypeArray<Ts...>>
	{
		using type = TypeArray<Types..., Ts...>;
	};
	template<typename TA>
	using append_array = typename AppendArray<TA>::type;

	template<typename>
	struct PrependArray;
	template<typename... Ts>
	struct PrependArray<TypeArray<Ts...>>
	{
		using type = TypeArray<Ts..., Types...>;
	};
	template<typename TA>
	using prepend_array = typename PrependArray<TA>::type;


	template<template<typename>typename Operator>
	using map = TypeArray<typename Operator<Types>::type...>;
	
	template<template<typename, typename>typename Operator, typename Initial>
	using reduce = TypeReduce_t<Operator, Initial, Types...>;

	template<template<typename>typename, typename...>
	struct Filter;

	template<template<typename>typename Pred,typename Unpacked, typename... Ts>
	struct Filter<Pred, Unpacked, Ts...>
	{
		using type = std::conditional_t<
			Pred<Unpacked>::value,
			typename TypeArray<Unpacked>::template append_array<typename Filter<Pred, Ts...>::type>::type,
			typename Filter<Pred, Ts...>::type>;
	};
	template<template<typename>typename Pred>
	struct Filter<Pred>
	{
		using type = TypeArray<>;
	};

	template<template<typename>typename Pred>
	using filter = typename Filter<Pred, Types...>::type;

	template<template<typename>typename Pred, typename...Ts>
	struct Partition
	{
		template<typename U>
		struct NotPred
		{
			constexpr static bool value = !Pred<U>::value;
		};
		struct type
		{
			using satisfied = typename Filter<Pred, Ts...>::type;
			using unsatisfied = typename Filter<NotPred, Ts...>::type;
		};
	};

	template<template<typename, typename>typename, typename>
	struct Sort;

	template<template<typename, typename>typename Comparator>
	struct Sort<Comparator, TypeArray<>>
	{
		using type = TypeArray<>;
	};

	template<template<typename, typename>typename Comparator,typename Pivot, typename... Ts>
	struct Sort<Comparator, TypeArray<Pivot, Ts...>>
	{
		template<typename U>
		struct Pred
		{
			constexpr static bool value = Comparator<U, Pivot>::value;
		};
		using partition_result = typename Partition<Pred, Ts...>::type;
		using type = Sort<Comparator,typename partition_result::satisfied>::type::template append<Pivot>::template append_array<typename Sort<Comparator,typename partition_result::unsatisfied>::type>;
	};
	
	template<template<typename, typename>typename Comparator>
	using sort = typename Sort<Comparator, type>::type;
};
template<auto... Values>
struct Array
{
	using type = Array;

	template<auto... Vs>
	using Template = Array<Vs...>;

	constexpr static bool is_array{ true };
	constexpr static std::tuple value{ Values... };
	constexpr static size_t size{ sizeof...(Values) };
	template<size_t I>
	static constexpr auto at = std::get<I>(value);
	template<auto... Vs>
	using append = Array<Values..., Vs...>;
	template<auto... Vs>
	using prepend = Array<Vs..., Values...>;
	template<typename>
	struct AppendArray {};
	template<auto... Vs>
	struct AppendArray<Array<Vs...>> { using type = Array<Values..., Vs...>; };
	template<typename>
	struct PrependArray {};
	template<auto... Vs>
	struct PrependArray<Array<Vs...>> { using type = Array<Vs..., Values...>; };
	template<typename U>
	using append_array = typename AppendArray<U>::type;
	template<typename U>
	using prepend_array = typename PrependArray<U>::type;

	template<template<auto>typename Operator>
	using map = Array<Operator<Values>::value...>;

	template<template<auto, auto>typename Operator, auto InitialValue>
	constexpr static auto reduce = ::Reduce<Operator, InitialValue, Values...>::value;
	
	
	template<template<auto>typename, auto...>
	struct Filter {};
	
	template<template<auto>typename Pred, auto UnpackedValue, auto...Values>
	struct Filter<Pred, UnpackedValue, Values...>
	{
		using type = std::conditional_t<
			Pred<UnpackedValue>::value,
			typename Array<UnpackedValue>::template append_array<typename Filter<Pred, Values...>::type>::type,
			typename Filter<Pred, Values...>::type>;
	};
	
	template<template<auto>typename Pred>
	struct Filter<Pred> { using type = Array<>; };

	template<template<auto>typename Pred>
	using filter = Filter<Pred, Values...>::type;


	template<auto, auto...>
	struct Contains;

	template<auto Value, auto Unpacked, auto... Vs>
	struct Contains<Value, Unpacked, Vs...> { constexpr static bool value = Value == Unpacked || Contains<Value, Vs...>::value; };
	
	template<auto Value>
	struct Contains<Value> { constexpr static bool value = false; };

	template<auto Value>
	constexpr static bool contains = Contains<Value, Values...>::value;


	template<auto,size_t,auto...>
	struct Find;
	
	template<auto Value,size_t I ,auto Unpacked, auto... Vs>
	struct Find<Value, I, Unpacked, Vs...> { constexpr static size_t value = Value == Unpacked ? I : Find<Value, I + 1, Vs...>::value; };
	
	template<auto Value, size_t I>
	struct Find<Value, I> { constexpr static size_t value = NPOS; };

	template<auto Value>
	constexpr static size_t find = Find<Value, 0, Values...>::value;

	
	template<template<auto>typename Pred,auto... Vs>
	struct Partition
	{
		template<auto V>
		struct NotPred { constexpr static bool value = !Pred<V>::value; };
		using base_type = type;
		struct type
		{
			using satisfied = typename base_type::Filter<Pred, Vs...>::type;
			using unsatisfied = typename base_type::Filter<NotPred, Vs...>::type;
		};
	};
	
	
	template<template<auto, auto>typename, typename>
	struct Sort;

	template<template<auto, auto>typename C>
	struct Sort<C, Array<>> { using type = Array<>; };

	template<template<auto,auto>typename Comparator,auto Pivot,  auto... Vs>
	struct Sort<Comparator, Array<Pivot, Vs...>>
	{
		template<auto V>
		using Pred = Comparator<V, Pivot>;
		using partition_result = typename Partition<Pred, Vs...>::type;
		using prefix = typename Sort<Comparator, typename partition_result::satisfied>::type;
		using suffix = typename Sort<Comparator, typename partition_result::unsatisfied>::type;
		using type = typename prefix::template append<Pivot>::template append_array<suffix>;
	};
	
	using sort = Sort<Less, type>::type;
};

template<typename... AdjLists>
struct Graph
{
	using type = Graph;
	using tuple_type = std::tuple<AdjLists...>;
	template<size_t I>
	using edges_of = std::tuple_element_t<I, tuple_type>;
	template<typename... AL>
	using add_edge = Graph<AdjLists..., AL...>::type;
	template<size_t From, size_t To>
	constexpr static bool has_edge = Expression<edges_of<From>::contains<To>>::value;
};


template<auto...>
struct Queue;

template<auto Front, auto... Values>
struct Queue<Front, Values...>
{
	using type = Queue;
	constexpr static bool empty = false;
	constexpr static auto top = Front;
	constexpr static size_t size = sizeof...(Values) + 1;
	
	struct Pop
	{
		using type = Queue<Values...>;
	};
	template<auto Value>
	struct Push
	{
		using type = Queue<Front, Values..., Value>;
	};
	template<auto Value>
	using push = typename Push<Value>::type;
	
	using pop = typename Pop::type;
};

template<>
struct Queue<>
{
	using type = Queue;
	constexpr static bool empty = true;
	constexpr static size_t size = 0;
	template<auto Value>
	struct Push
	{
		using type = Queue<Value>;
	};
	template<auto Value>
	using push = typename Push<Value>::type;
};

template<auto...>
struct Stack;

template<auto Top, auto... Values>
struct Stack<Top, Values...> {
	using type = Stack;
	constexpr static auto top = Top;
	constexpr static bool empty = false;
	constexpr static size_t size = sizeof...(Values) + 1;
	template<auto V>
	using push = Stack<V, Top, Values...>;
	
	using pop = Stack<Values...>;
};

template<>
struct Stack<>
{
	using type = Stack;
	constexpr static bool empty = true;
	constexpr static size_t size = 0;
	template<auto V>
	using push = Stack<V>;
};


//data section
// 
//text section
// 
//ValueDump<BinaryValueExpression<a, b, Plus>::value>;
//TypeDump<TypeArray<Array<1, 2, 3, 4, 5>, Array<6, 7, 8, 9, 10>>>;
//using ArrA = Array<1, 2, 3, 4, 5>;
//ValueDump<ArrayReduce<Multiply, ArrA, 1>::value>;

//template<int v>
//using Int = std::integral_constant<int, v>;

//using ArrB = TypeArray<Double<1.5>, Double<2.3>, Double<3.1>>;
//ValueDump<TypeArrayReduce<TypePlus, ArrB, Double<0.0>>::type::value>;

template<int n>
struct Even { constexpr static bool value = n % 2 == 0; };
template<int n>
using Plus10 = Plus<n, 10>;

using Stk1 = Stack<>;
PrintValue<Int<Stk1::push<4>::push<5>::push<6>::pop::top>::value>;

template<typename T, typename U>
struct TypeLess { constexpr static bool value = T::value < U::value; };
using TA = TypeArray<Int<1>, Int<3>, Int<2>, Int<5>, Int<4>>;
using res = TA::sort<TypeLess>;
PrintType<unwrap<res>>;
