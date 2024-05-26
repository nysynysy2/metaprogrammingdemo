#pragma once
template<auto a, auto b>
struct Plus { constexpr static auto value = a + b; };

template<auto a, auto b>
struct Minus { constexpr static auto value = a - b; };

template<auto a, auto b>
struct Multiplies { constexpr static auto value = a * b; };

template<auto a, auto b>
struct Divide { constexpr static auto value = a / b; };

template<typename ValA, typename ValB>
struct TypePlus
{
	using type = std::integral_constant<typename ValA::value_type, ValA::value + ValB::value>::type;
	constexpr static auto value = type::value;
};
template<typename ValA, typename ValB>
struct TypeMinus
{
	using type = std::integral_constant<typename ValA::value_type, ValA::value - ValB::value>::type;
	constexpr static auto value = type::value;
};
template<typename ValA, typename ValB>
struct TypeMultiply
{
	using type = std::integral_constant<typename ValA::value_type, ValA::value* ValB::value>::type;
	constexpr static auto value = type::value;
};
template<typename ValA, typename ValB>
struct TypeDivide
{
	using type = std::integral_constant<typename ValA::value_type, ValA::value / ValB::value>::type;
	constexpr static auto value = type::value;
};

template<typename Operand1, typename Operand2, template<typename, typename>typename Operator>
struct BinaryTypeExpression
{
	using type = Operator<Operand1, Operand2>::type;
};
template<auto Operand1, auto Operand2, template<auto, auto>typename Operator>
struct BinaryValueExpression
{
	constexpr static auto value = Operator<Operand1, Operand2>::value;
};
template<auto ValA, auto ValB>
struct Greater
{
	constexpr static bool value = ValA > ValB;
};
template<auto ValA, auto ValB>
struct GreaterEqual
{
	constexpr static bool value = ValA >= ValB;
};
template<auto ValA, auto ValB>
struct Less
{
	constexpr static bool value = ValA < ValB;
};
template<auto ValA, auto ValB>
struct LessEqual
{
	constexpr static bool value = ValA <= ValB;
};
