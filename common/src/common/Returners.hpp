#pragma once
#include <concepts>
#include <string>

#include <nlohmann/json.hpp>
#include <SFML/System/Vector2.hpp>
#include <Thor/Math/Distributions.hpp>

#define TEXT(x) #x

enum ReturnType
{
	Unknown,
	FloatValue,
	IntValue,
	SizeTValue,
	Vector2fValue,
	Position,
	Height,	
	XOffset,
	YOffset,
	Offset,
	Speed,
	XSpeed,
	YSpeed,
	XBoundary,
	YBoundary,
	Chance,
	RelativeProbability,
	Count
};

constexpr const char* ReturnType_to_text[] = 
{
	TEXT(Unknown),
	TEXT(FloatValue),
	TEXT(IntValue),
	TEXT(SizeTValue),
	TEXT(Vector2fValue),
	TEXT(Position),
	TEXT(Height),
	TEXT(XOffset),
	TEXT(YOffset),
	TEXT(Offset),
	TEXT(Speed),
	TEXT(XSpeed),
	TEXT(YSpeed),
	TEXT(XBoundary),
	TEXT(YBoundary),
	TEXT(Chance),
	TEXT(RelativeProbability),
	TEXT(Count)
};

namespace
{
	template<ReturnType> struct ToValueType { using type = void; }; 
	template<> struct ToValueType<FloatValue> { using type = float; };
	template<> struct ToValueType<IntValue> { using type = int; };
	template<> struct ToValueType<SizeTValue> { using type = size_t; };
	template<> struct ToValueType<Vector2fValue> { using type = sf::Vector2f; };
	template<> struct ToValueType<Position> { using type = sf::Vector2f; };
	template<> struct ToValueType<Height> { using type = float; };
	template<> struct ToValueType<XOffset> { using type = float; };
	template<> struct ToValueType<YOffset> { using type = float; };
	template<> struct ToValueType<Offset> { using type = sf::Vector2f; };
	template<> struct ToValueType<Speed> { using type = sf::Vector2f; };
	template<> struct ToValueType<XSpeed> { using type = float; };
	template<> struct ToValueType<YSpeed> { using type = float; };
	template<> struct ToValueType<XBoundary> { using type = float; };
	template<> struct ToValueType<YBoundary> { using type = float; };
	template<> struct ToValueType<Chance> { using type = float; };
	template<> struct ToValueType<RelativeProbability> { using type = float; };
	template<> struct ToValueType<Count> { using type = unsigned int; };
	template<ReturnType type> using ValueType = ToValueType<type>::type;

	template<class T, class... Ts>
	concept one_of_types = (std::same_as<T, Ts> || ...);
}

template<ReturnType RT>
class Returner
{
public:
	using ValT = ValueType<RT>;
	constexpr static inline ReturnType RetType = RT;
	ValT getValue() const
	{
		return get();
	}

	virtual void to_json(nl::json& j) const
	{
		j["name"] = TEXT(Returner);
	}

	virtual void from_json(const nl::json& j)
	{
	}

protected:
	virtual ValT get() const
	{
		return ValT{};
	}
};

template <ReturnType RT>
class ConstantReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	ValT val;
	ConstantReturner(const ValT& val = ValT{}) : val(val) {}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["name"] = TEXT(ConstantReturner);
		j["value"] = val;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("value")) j["value"].get_to(val);
	}

protected:
	virtual ValT get() const override 
	{ 
		return val;
	}
};

template<ReturnType RT>
	requires one_of_types<ValueType<RT>, sf::Time, float, int, unsigned int>
class UniformDistributionReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	ValT min_val, max_val;
	UniformDistributionReturner(const ValT& min_val = ValT{}, const ValT& max_val = ValT{}) :
		min_val(min_val),
		max_val(max_val)
	{}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["name"] = TEXT(UniformDistributionReturner);
		j["min_value"] = min_val;
		j["max_value"] = max_val;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("min_value")) j["min_value"].get_to(min_val);
		if (j.contains("max_value")) j["max_value"].get_to(max_val);
	}

protected:
	virtual ValT get() const override 
	{ 
		return thor::Distributions::uniform(min_val, max_val)();
	}
};

template<ReturnType RT>
	requires std::same_as<ValueType<RT>, sf::Vector2f>
class RectDistributionReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	ValT center, half_size;
	RectDistributionReturner(const ValT& center = ValT{}, const ValT& half_size = ValT{}) :
		center(center),
		half_size(half_size)
	{}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["name"] = TEXT(RectDistributionReturner);
		j["center"] = center;
		j["half_size"] = half_size;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("center")) j["center"].get_to(center);
		if (j.contains("half_size")) j["half_size"].get_to(half_size);
	}

protected:
	virtual ValT get() const override
	{
		return thor::Distributions::rect(center, half_size)();
	}
};

template<ReturnType RT>
	requires std::same_as<ValueType<RT>, sf::Vector2f>
class CircleDistributionReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	ValT center;
	float radius;
	CircleDistributionReturner(const ValT& center = ValT{}, float radius = 0) :
		center(center),
		radius(radius)
	{}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["name"] = TEXT(CircleDistributionReturner);
		j["center"] = center;
		j["radius"] = radius;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("center")) j["center"].get_to(center);
		if (j.contains("radius")) j["radius"].get_to(radius);
	}

protected:
	virtual ValT get() const override
	{
		return thor::Distributions::circle(center, radius)();
	}
};

template<ReturnType RT>
	requires std::same_as<ValueType<RT>, sf::Vector2f>
class DeflectDistributionReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	ValT direction;
	float max_rotation;
	DeflectDistributionReturner(const ValT& center = ValT{}, float radius = 0) :
		direction(center),
		max_rotation(radius)
	{}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["name"] = TEXT(DeflectDistributionReturner);
		j["direction"] = direction;
		j["max_rotation"] = max_rotation;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("direction")) j["direction"].get_to(direction);
		if (j.contains("max_rotation")) j["max_rotation"].get_to(max_rotation);
	}

protected:
	virtual ValT get() const override
	{
		return thor::Distributions::deflect(direction, max_rotation)();
	}
};

template<ReturnType RT>
	requires (RT == Height)
class GeneratedHeightReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["name"] = TEXT(GeneratedHeightReturner);
	}

	virtual void from_json(const nl::json& j) override
	{}

protected:
	virtual ValT get() const override;
};

template<ReturnType RT>
	requires requires { {-std::declval<ValueType<RT>>} -> std::same_as<ValueType<RT>>; }
class NegativeReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	std::unique_ptr<Returner<RT>> ret;
	NegativeReturner(std::unique_ptr<Returner<RT>>&& ret = std::unique_ptr<Returner<RT>>{}) :
		ret(std::move(ret))
	{}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["name"] = TEXT(NegativeReturner);
		j["returner"] = ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("returner")) j["returner"].get_to(ret);
	}

protected:
	virtual ValT get() const override
	{
		return -ret->getValue();
	}
};

template <ReturnType RT, ReturnType FRT, ReturnType SRT>
	requires requires { { std::declval<ValueType<FRT>>() + std::declval<ValueType<SRT>>() } -> std::same_as<ValueType<RT>>; }
class SumReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	std::unique_ptr<Returner<FRT>> f_ret;
	std::unique_ptr<Returner<SRT>> s_ret;
	SumReturner(std::unique_ptr<Returner<FRT>>&& f_ret = std::unique_ptr<Returner<FRT>>{}, std::unique_ptr<Returner<SRT>>&& s_ret = std::unique_ptr<Returner<SRT>>{}) :
		f_ret(std::move(f_ret)),
		s_ret(std::move(s_ret))
	{}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["first_type"] = ReturnType_to_text[FRT];
		j["second_type"] = ReturnType_to_text[SRT];
		j["name"] = TEXT(SumReturner);
		j["first_returner"] = f_ret;
		j["second_returner"] = s_ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("first_returner")) j["first_returner"].get_to(f_ret);
		if (j.contains("second_returner")) j["second_returner"].get_to(s_ret);
	}

protected:
	virtual ValT get() const override
	{
		return f_ret->getValue() + s_ret->getValue();
	}
};

template <ReturnType RT, ReturnType FRT, ReturnType SRT>
	requires requires { { std::declval<ValueType<FRT>>() - std::declval<ValueType<SRT>>() } -> std::same_as<ValueType<RT>>; }
class DifferenceReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	std::unique_ptr<Returner<FRT>> f_ret;
	std::unique_ptr<Returner<SRT>> s_ret;
	DifferenceReturner(std::unique_ptr<Returner<FRT>>&& f_ret = std::unique_ptr<Returner<FRT>>{}, std::unique_ptr<Returner<SRT>>&& s_ret = std::unique_ptr<Returner<SRT>>{}) :
		f_ret(std::move(f_ret)),
		s_ret(std::move(s_ret))
	{}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["first_type"] = ReturnType_to_text[FRT];
		j["second_type"] = ReturnType_to_text[SRT];
		j["name"] = TEXT(DifferenceReturner);
		j["first_returner"] = f_ret;
		j["second_returner"] = s_ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("first_returner")) j["first_returner"].get_to(f_ret);
		if (j.contains("second_returner")) j["second_returner"].get_to(s_ret);
	}

protected:
	virtual ValT get() const override
	{
		return f_ret->getValue() - s_ret->getValue();
	}
};

template <ReturnType RT, ReturnType FRT, ReturnType SRT>
	requires requires { { std::declval<ValueType<FRT>>() * std::declval<ValueType<SRT>>() } -> std::same_as<ValueType<RT>>; }
class ProductReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	std::unique_ptr<Returner<FRT>> f_ret;
	std::unique_ptr<Returner<SRT>> s_ret;
	ProductReturner(std::unique_ptr<Returner<FRT>>&& f_ret = std::unique_ptr<Returner<FRT>>{}, std::unique_ptr<Returner<SRT>>&& s_ret = std::unique_ptr<Returner<SRT>>{}) :
		f_ret(std::move(f_ret)),
		s_ret(std::move(s_ret))
	{}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["first_type"] = ReturnType_to_text[FRT];
		j["second_type"] = ReturnType_to_text[SRT];
		j["name"] = TEXT(ProductReturner);
		j["first_returner"] = f_ret;
		j["second_returner"] = s_ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("first_returner")) j["first_returner"].get_to(f_ret);
		if (j.contains("second_returner")) j["second_returner"].get_to(s_ret);
	}

protected:
	virtual ValT get() const override
	{
		return f_ret->getValue() * s_ret->getValue();
	}
};

template <ReturnType RT, ReturnType FRT, ReturnType SRT>
	requires requires { { std::declval<ValueType<FRT>>() / std::declval<ValueType<SRT>>() } -> std::same_as<ValueType<RT>>; }
class QuotientReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	std::unique_ptr<Returner<FRT>> f_ret;
	std::unique_ptr<Returner<SRT>> s_ret;
	QuotientReturner(std::unique_ptr<Returner<FRT>>&& f_ret = std::unique_ptr<Returner<FRT>>{}, std::unique_ptr<Returner<SRT>>&& s_ret = std::unique_ptr<Returner<SRT>>{}) :
		f_ret(std::move(f_ret)),
		s_ret(std::move(s_ret))
	{}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["first_type"] = ReturnType_to_text[FRT];
		j["second_type"] = ReturnType_to_text[SRT];
		j["name"] = TEXT(QuotientReturner);
		j["first_returner"] = f_ret;
		j["second_returner"] = s_ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("first_returner")) j["first_returner"].get_to(f_ret);
		if (j.contains("second_returner")) j["second_returner"].get_to(s_ret);
	}

protected:
	virtual ValT get() const override
	{
		return f_ret->getValue() / s_ret->getValue();
	}
};

template <class RetT>
	requires std::derived_from<RetT, Returner<RetT::RetType>>
RetT* getReturnerPointerFromJson(const nl::json& j)
{
	std::string name = (j.contains("name") ? j["name"] : TEXT(Returner));
	std::string first_type = (j.contains("first_type") ? j["first_type"] : ReturnType_to_text[RetT::RetType]);
	std::string second_type = (j.contains("second_type") ? j["second_type"] : ReturnType_to_text[RetT::RetType]);
	
#define RETURN_FOR_PRIM_TYPE(x) if constexpr(requires { { std::derived_from<x<RetT::RetType>, RetT> }; }) if(name == #x) return new x<RetT::RetType>
	RETURN_FOR_PRIM_TYPE(Returner);
	RETURN_FOR_PRIM_TYPE(ConstantReturner);
	RETURN_FOR_PRIM_TYPE(UniformDistributionReturner);
	RETURN_FOR_PRIM_TYPE(RectDistributionReturner);
	RETURN_FOR_PRIM_TYPE(CircleDistributionReturner);
	RETURN_FOR_PRIM_TYPE(DeflectDistributionReturner);
	RETURN_FOR_PRIM_TYPE(GeneratedHeightReturner);
	RETURN_FOR_PRIM_TYPE(NegativeReturner);
#undef RETURN_FOR_PRUM_TYPE

#define RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, t2) if constexpr(requires { { std::derived_from<x<RetT::RetType, t1, t2>, RetT> }; }) return new x<RetT::RetType, t1, t2>;

#define CHECK(text, type) if(text == #type) 

#define RETURN_FOR_NON_PRIM_TYPE_W1T(x, t1) \
{\
	CHECK(second_type, Unknown) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, Unknown)\
	CHECK(second_type, FloatValue) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, FloatValue)\
	CHECK(second_type, IntValue) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, IntValue)\
	CHECK(second_type, SizeTValue) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, SizeTValue)\
	CHECK(second_type, Vector2fValue) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, Vector2fValue)\
	CHECK(second_type, Position) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, Position)\
	CHECK(second_type, Height) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, Height)\
	CHECK(second_type, XOffset) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, XOffset)\
	CHECK(second_type, YOffset) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, YOffset)\
	CHECK(second_type, Offset) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, Offset)\
	CHECK(second_type, Speed) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, Speed)\
	CHECK(second_type, XSpeed) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, XSpeed)\
	CHECK(second_type, YSpeed) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, YSpeed)\
	CHECK(second_type, XBoundary) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, XBoundary)\
	CHECK(second_type, YBoundary) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, YBoundary)\
	CHECK(second_type, Chance) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, Chance)\
	CHECK(second_type, RelativeProbability) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, RelativeProbability)\
	CHECK(second_type, Count) RETURN_FOR_NON_PRIM_TYPE_W2T(x, t1, Count)\
}

#define RETURN_FOR_NON_PRIM_TYPE(x) \
{\
	CHECK(first_type, Unknown) RETURN_FOR_NON_PRIM_TYPE_W1T(x, Unknown)\
	CHECK(first_type, FloatValue) RETURN_FOR_NON_PRIM_TYPE_W1T(x, FloatValue)\
	CHECK(first_type, IntValue) RETURN_FOR_NON_PRIM_TYPE_W1T(x, IntValue)\
	CHECK(first_type, SizeTValue) RETURN_FOR_NON_PRIM_TYPE_W1T(x, SizeTValue)\
	CHECK(first_type, Vector2fValue) RETURN_FOR_NON_PRIM_TYPE_W1T(x, Vector2fValue)\
	CHECK(first_type, Position) RETURN_FOR_NON_PRIM_TYPE_W1T(x, Position)\
	CHECK(first_type, Height) RETURN_FOR_NON_PRIM_TYPE_W1T(x, Height)\
	CHECK(first_type, XOffset) RETURN_FOR_NON_PRIM_TYPE_W1T(x, XOffset)\
	CHECK(first_type, YOffset) RETURN_FOR_NON_PRIM_TYPE_W1T(x, YOffset)\
	CHECK(first_type, Offset) RETURN_FOR_NON_PRIM_TYPE_W1T(x, Offset)\
	CHECK(first_type, Speed) RETURN_FOR_NON_PRIM_TYPE_W1T(x, Speed)\
	CHECK(first_type, XSpeed) RETURN_FOR_NON_PRIM_TYPE_W1T(x, XSpeed)\
	CHECK(first_type, YSpeed) RETURN_FOR_NON_PRIM_TYPE_W1T(x, YSpeed)\
	CHECK(first_type, XBoundary) RETURN_FOR_NON_PRIM_TYPE_W1T(x, XBoundary)\
	CHECK(first_type, YBoundary) RETURN_FOR_NON_PRIM_TYPE_W1T(x, YBoundary)\
	CHECK(first_type, Chance) RETURN_FOR_NON_PRIM_TYPE_W1T(x, Chance)\
	CHECK(first_type, RelativeProbability) RETURN_FOR_NON_PRIM_TYPE_W1T(x, RelativeProbability)\
	CHECK(first_type, Count) RETURN_FOR_NON_PRIM_TYPE_W1T(x, Count)\
}

	CHECK(name, SumReturner) RETURN_FOR_NON_PRIM_TYPE(SumReturner)
	CHECK(name, DifferenceReturner) RETURN_FOR_NON_PRIM_TYPE(DifferenceReturner)
	CHECK(name, ProductReturner) RETURN_FOR_NON_PRIM_TYPE(ProductReturner)
	CHECK(name, QuotientReturner) RETURN_FOR_NON_PRIM_TYPE(QuotientReturner)

#undef RETURN_FOR_NON_PRIM_TYPE_W2T
#undef RETURN_FOR_NON_PRIM_TYPE_W1T
#undef RETURN_FOR_NON_PRIM_TYPE
#undef CHECK
	return nullptr;
}

namespace nlohmann
{
	template<class RetT>
		requires std::derived_from<RetT, Returner<RetT::RetType>>
	struct adl_serializer<std::unique_ptr<RetT>>
	{
		static void to_json(nl::json& j, const std::unique_ptr<RetT>& ptr) 
		{
			if (ptr) ptr->to_json(j);
			else j = nullptr;
		};

		static void from_json(const nl::json& j, std::unique_ptr<RetT>& ptr) 
		{
			if (j.is_null())
			{
				ptr = std::unique_ptr<RetT>{};
				return;
			}
#define TEXT(x) #x
			ptr = std::unique_ptr<RetT>(getReturnerPointerFromJson<RetT>(j));
#undef TEXT
			ptr->from_json(j);
		};
	};

	template<>
	struct adl_serializer<sf::Vector2f>
	{
		static void to_json(nl::json& j, const sf::Vector2f& vec)
		{
			j["x"] = vec.x;
			j["y"] = vec.y;
		}

		static void from_json(const nl::json& j, sf::Vector2f& vec)
		{
			if (j.contains("x")) j["x"].get_to(vec.x);
			if (j.contains("y")) j["y"].get_to(vec.y);
		}
	};
}