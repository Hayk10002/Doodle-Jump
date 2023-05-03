#pragma once
#include <concepts>
#include <string>

#include <imgui.h>
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include <Thor/Math/Distributions.hpp>

#include <common/GameStuff.hpp>
#include <common/Utils.hpp>

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
	ReturnerTypesCount
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
	TEXT(ReturnerTypesCount)
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
	template<> struct ToValueType<ReturnerTypesCount> { using type = void; };
	template<ReturnType type> using ValueType = ToValueType<type>::type;

	template<class T, class... Ts>
	concept one_of_types = (std::same_as<T, Ts> || ...);
}

void toImGui(float& val);
void toImGui(int& val);
void toImGui(size_t& val);
void toImGui(sf::Vector2f& val);

template<class T>
const char* getClassName()
{
	return typeid(std::decay_t<T>).name();
}

template<ReturnType RT>
class Returner
{
	size_t ImGui_id{ 0 };
	inline static size_t ImGui_id_counter{ 0 };
public:
	using ValT = ValueType<RT>;
	constexpr static inline ReturnType RetType = RT;
	Returner() : ImGui_id{ ++ImGui_id_counter } {}

	ValT getValue() const
	{
		return get();
	}

	virtual void to_json(nl::json& j) const
	{
		j["name"] = getClassName<decltype(*this)>();
	}

	virtual void from_json(const nl::json& j) {}
	void toImGui()
	{
		static size_t id = 0;
		if (ImGui::TreeNodeEx(std::format("{} (type: {})##{}", getName(), ReturnType_to_text[RT], ImGui_id).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			toImGuiImpl();
			ImGui::TreePop();
		}
	}


	virtual std::string getName() const
	{
		return TEXT(Returner);
	}

protected:
	virtual ValT get() const
	{
		return ValT{};
	}
	virtual void toImGuiImpl() {}
};

template<class RetT>
	requires std::derived_from<RetT, Returner<RetT::RetType>>
void toImGui(std::unique_ptr<RetT>& returner, const std::string& format);

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
		j["name"] = getClassName<decltype(*this)>();
		j["value"] = val;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("value")) j["value"].get_to(val);
	}

	virtual std::string getName() const override { return TEXT(ConstantReturner); }

protected:
	virtual ValT get() const override 
	{ 
		return val;
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		ImGui::Text("Value:"); ImGui::SameLine(); ::toImGui(val);
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
		j["name"] = getClassName<decltype(*this)>();
		j["min_value"] = min_val;
		j["max_value"] = max_val;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("min_value")) j["min_value"].get_to(min_val);
		if (j.contains("max_value")) j["max_value"].get_to(max_val);
	}

	virtual std::string getName() const override
	{
		return TEXT(UniformDistributionReturner);
	}

protected:
	virtual ValT get() const override 
	{ 
		return thor::Distributions::uniform(min_val, max_val)();
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		ImGui::Text("Min:"); ImGui::SameLine(); ::toImGui(min_val);
		ImGui::Text("Max:"); ImGui::SameLine(); ::toImGui(max_val);
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
		j["name"] = getClassName<decltype(*this)>();
		j["center"] = center;
		j["half_size"] = half_size;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("center")) j["center"].get_to(center);
		if (j.contains("half_size")) j["half_size"].get_to(half_size);
	}

	virtual std::string getName() const override
	{
		return TEXT(RectDistributionReturner);
	}
	
protected:
	virtual ValT get() const override
	{
		return thor::Distributions::rect(center, half_size)();
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		ImGui::Text("Center:"); ImGui::SameLine(); ::toImGui(center);
		ImGui::Text("Half size:"); ImGui::SameLine(); ::toImGui(half_size);
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
		j["name"] = getClassName<decltype(*this)>();
		j["center"] = center;
		j["radius"] = radius;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("center")) j["center"].get_to(center);
		if (j.contains("radius")) j["radius"].get_to(radius);
	}

	virtual std::string getName() const override
	{
		return TEXT(CircleDistributionReturner);
	}

protected:
	virtual ValT get() const override
	{
		return thor::Distributions::circle(center, radius)();
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		ImGui::Text("Center:"); ImGui::SameLine(); ::toImGui(center);
		ImGui::Text("Radius:"); ImGui::SameLine(); ::toImGui(radius);
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
		j["name"] = getClassName<decltype(*this)>();
		j["direction"] = direction;
		j["max_rotation"] = max_rotation;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("direction")) j["direction"].get_to(direction);
		if (j.contains("max_rotation")) j["max_rotation"].get_to(max_rotation);
	}

	virtual std::string getName() const override
	{
		return TEXT(DeflectDistributionReturner);
	}

protected:
	virtual ValT get() const override
	{
		return thor::Distributions::deflect(direction, max_rotation)();
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		ImGui::Text("Direction:"); ImGui::SameLine(); ::toImGui(direction);
		ImGui::Text("Max rotation:"); ImGui::SameLine(); ::toImGui(max_rotation);
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
		j["name"] = getClassName<decltype(*this)>();
	}

	virtual void from_json(const nl::json& j) override
	{}

	virtual std::string getName() const override
	{
		return TEXT(GeneratedHeightReturner);
	}

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
		j["name"] = getClassName<decltype(*this)>();
		j["returner"] = ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("returner")) j["returner"].get_to(ret);
	}

	virtual std::string getName() const override
	{
		return TEXT(NegativeReturner);
	}

protected:
	virtual ValT get() const override
	{
		return ret ? -ret->getValue() : ValT{};
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		::toImGui<Returner<RT>>(ret, "Value:");
	}
};

template <ReturnType RT>
	requires requires { { std::declval<ValueType<RT>>() < std::declval<ValueType<RT>>() } -> std::same_as<bool>; }
class MinReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	std::unique_ptr<Returner<RT>> f_ret;
	std::unique_ptr<Returner<RT>> s_ret;
	MinReturner(std::unique_ptr<Returner<RT>>&& f_ret = std::unique_ptr<Returner<RT>>{}, std::unique_ptr<Returner<RT>>&& s_ret = std::unique_ptr<Returner<RT>>{}) :
		f_ret(std::move(f_ret)),
		s_ret(std::move(s_ret))
	{}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["name"] = getClassName<decltype(*this)>();
		j["first_returner"] = f_ret;
		j["second_returner"] = s_ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("first_returner")) j["first_returner"].get_to(f_ret);
		if (j.contains("second_returner")) j["second_returner"].get_to(s_ret);
	}

	virtual std::string getName() const override
	{
		return TEXT(MinReturner);
	}

protected:
	virtual ValT get() const override
	{
		ValT f_val = f_ret ? f_ret->getValue() : ValT{};
		ValT s_val = s_ret ? s_ret->getValue() : ValT{};
		return (f_val < s_val) ? f_val : s_val;
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		::toImGui<Returner<RT>>(f_ret, "First:");
		::toImGui<Returner<RT>>(s_ret, "Second:");
	}
};

template <ReturnType RT>
	requires requires { { std::declval<ValueType<RT>>() < std::declval<ValueType<RT>>() } -> std::same_as<bool>; }
class MaxReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	std::unique_ptr<Returner<RT>> f_ret;
	std::unique_ptr<Returner<RT>> s_ret;
	MaxReturner(std::unique_ptr<Returner<RT>>&& f_ret = std::unique_ptr<Returner<RT>>{}, std::unique_ptr<Returner<RT>>&& s_ret = std::unique_ptr<Returner<RT>>{}) :
		f_ret(std::move(f_ret)),
		s_ret(std::move(s_ret))
	{}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["name"] = getClassName<decltype(*this)>();
		j["first_returner"] = f_ret;
		j["second_returner"] = s_ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("first_returner")) j["first_returner"].get_to(f_ret);
		if (j.contains("second_returner")) j["second_returner"].get_to(s_ret);
	}

	virtual std::string getName() const override
	{
		return TEXT(MaxReturner);
	}

protected:
	virtual ValT get() const override
	{
		ValT f_val = f_ret ? f_ret->getValue() : ValT{};
		ValT s_val = s_ret ? s_ret->getValue() : ValT{};
		return (f_val < s_val) ? s_val : f_val;
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		::toImGui<Returner<RT>>(f_ret, "First:");
		::toImGui<Returner<RT>>(s_ret, "Second:");
	}
};

template <ReturnType RT>
	requires requires { { std::declval<ValueType<RT>>() < std::declval<ValueType<RT>>() } -> std::same_as<bool>; }
class ClampReturner : public Returner<RT>
{
public:
	using typename Returner<RT>::ValT;
	std::unique_ptr<Returner<RT>> ret;
	std::unique_ptr<Returner<RT>> min_ret;
	std::unique_ptr<Returner<RT>> max_ret;
	ClampReturner(std::unique_ptr<Returner<RT>>&& ret = std::unique_ptr<Returner<RT>>{}, std::unique_ptr<Returner<RT>>&& min_ret = std::unique_ptr<Returner<RT>>{}, std::unique_ptr<Returner<RT>>&& max_ret = std::unique_ptr<Returner<RT>>{}):
		ret(std::move(ret)),
		min_ret(std::move(min_ret)),
		max_ret(std::move(max_ret))
	{}

	virtual void to_json(nl::json& j) const override
	{
		Returner<RT>::to_json(j);
		j["name"] = getClassName<decltype(*this)>();
		j["returner"] = ret;
		j["min_returner"] = min_ret;
		j["max_returner"] = max_ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("returner")) j["returner"].get_to(ret);
		if (j.contains("min_returner")) j["min_returner"].get_to(min_ret);
		if (j.contains("max_returner")) j["max_returner"].get_to(max_ret);
	}

	virtual std::string getName() const override
	{
		return TEXT(ClampReturner);
	}

protected:
	virtual ValT get() const override
	{
		ValT val = ret ? ret->getValue() : ValT{};
		ValT min_val = min_ret ? min_ret->getValue() : ValT{};
		ValT max_val = max_ret ? max_ret->getValue() : ValT{};
		if (max_val < min_val) return val;
		if (val < min_val) return min_val;
		if (max_val < val) return max_val;
		return val;
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		::toImGui<Returner<RT>>(ret, "Value:");
		::toImGui<Returner<RT>>(min_ret, "Min value:");
		::toImGui<Returner<RT>>(max_ret, "Max value:");
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
		j["name"] = getClassName<decltype(*this)>();
		j["first_returner"] = f_ret;
		j["second_returner"] = s_ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("first_returner")) j["first_returner"].get_to(f_ret);
		if (j.contains("second_returner")) j["second_returner"].get_to(s_ret);
	}

	virtual std::string getName() const override
	{
		return TEXT(SumReturner);
	}

protected:
	virtual ValT get() const override
	{
		return (f_ret ? f_ret->getValue() : ValueType<FRT>{}) + (s_ret ? s_ret->getValue() : ValueType<SRT>{});
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		::toImGui<Returner<FRT>>(f_ret, "First:");
		::toImGui<Returner<SRT>>(s_ret, "Second:");
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
		j["name"] = getClassName<decltype(*this)>();
		j["first_returner"] = f_ret;
		j["second_returner"] = s_ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("first_returner")) j["first_returner"].get_to(f_ret);
		if (j.contains("second_returner")) j["second_returner"].get_to(s_ret);
	}

	virtual std::string getName() const override
	{
		return TEXT(DifferenceReturner);
	}

protected:
	virtual ValT get() const override
	{
		return (f_ret ? f_ret->getValue() : ValueType<FRT>{}) - (s_ret ? s_ret->getValue() : ValueType<SRT>{});
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		::toImGui<Returner<FRT>>(f_ret, "First:");
		::toImGui<Returner<SRT>>(s_ret, "Second:");
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
		j["name"] = getClassName<decltype(*this)>();
		j["first_returner"] = f_ret;
		j["second_returner"] = s_ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("first_returner")) j["first_returner"].get_to(f_ret);
		if (j.contains("second_returner")) j["second_returner"].get_to(s_ret);
	}

	virtual std::string getName() const override
	{
		return TEXT(ProductReturner);
	}

protected:
	virtual ValT get() const override
	{
		return (f_ret ? f_ret->getValue() : ValueType<FRT>{}) * (s_ret ? s_ret->getValue() : ValueType<SRT>{});
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		::toImGui<Returner<FRT>>(f_ret, "First:");
		::toImGui<Returner<SRT>>(s_ret, "Second:");
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
		j["name"] = getClassName<decltype(*this)>();
		j["first_returner"] = f_ret;
		j["second_returner"] = s_ret;
	}

	virtual void from_json(const nl::json& j) override
	{
		Returner<RT>::from_json(j);
		if (j.contains("first_returner")) j["first_returner"].get_to(f_ret);
		if (j.contains("second_returner")) j["second_returner"].get_to(s_ret);
	}

	virtual std::string getName() const override
	{
		return TEXT(QuotientReturner);
	}

protected:
	virtual ValT get() const override
	{
		return (f_ret ? f_ret->getValue() : ValueType<FRT>{}) / (s_ret ? s_ret->getValue() : ValueType<SRT>{});
	}
	virtual void toImGuiImpl() override
	{
		Returner<RT>::toImGuiImpl();
		::toImGui<Returner<FRT>>(f_ret, "First:");
		::toImGui<Returner<SRT>>(s_ret, "Second:");
	}
};


template <class RetT, template<ReturnType> class T>
RetT* returnForPrimType(const std::string& name) 
{
	constexpr bool requirement = requires { {std::derived_from<T<RetT::RetType>, RetT>}; };
	if constexpr (!requirement) return nullptr;
	else
	{
		if (getClassName<T<RetT::RetType>>() != name) return nullptr;
		return new T<RetT::RetType>;
	}
};

template <class RetT, template<ReturnType> class ...Ts>
RetT* returnForPrimTypes(const std::string& name)
{
	return nonZeroPtr<RetT>({ returnForPrimType<RetT, Ts>(name)... });
};

template<class RetT, template<ReturnType, ReturnType, ReturnType> class T, ReturnType t1, ReturnType t2>
RetT* returnForNonPrimTypeW2T(const std::string& name)
{
	constexpr bool requirement = requires { {std::derived_from<T<RetT::RetType, t1, t2>, RetT>}; };
	if constexpr (!requirement) return nullptr;
	else
	{
		if (getClassName<T<RetT::RetType, t1, t2>>() != name) return nullptr;
		return new T<RetT::RetType, t1, t2>;
	}
};

template<class RetT, template<ReturnType, ReturnType, ReturnType> class T, ReturnType t1, int ...types>
RetT* returnForNonPrimTypeW1T(const std::string& name, std::integer_sequence<int, types...>)
{
	return nonZeroPtr<RetT>({returnForNonPrimTypeW2T<RetT, T, t1, ReturnType(types)>(name)...});
};

template<class RetT, template<ReturnType, ReturnType, ReturnType> class T, int ...types1, int ...types2>
RetT* returnForNonPrimType(const std::string& name, std::integer_sequence<int, types1...>, std::integer_sequence<int, types2...> i2)
{
	return nonZeroPtr<RetT>({returnForNonPrimTypeW1T<RetT, T, ReturnType(types1)>(name, i2)...});
};

template<class RetT, template<ReturnType, ReturnType, ReturnType> class ...Ts, int ...types1, int ...types2>
RetT* returnForNonPrimTypes(const std::string& name, std::integer_sequence<int, types1...> i1, std::integer_sequence<int, types2...> i2)
{
	return nonZeroPtr<RetT>({returnForNonPrimType<RetT, Ts>(name, i1, i2)...});
};


template <class RetT>
RetT* nonZeroPtr(std::initializer_list<RetT*> l)
{
	for (auto ptr : l) if (ptr) return ptr;
	return nullptr;
}

template <class RetT>
	requires std::derived_from<RetT, Returner<RetT::RetType>>
RetT* getReturnerPointerFromName(const std::string& name)
{
	return nonZeroPtr<RetT>(
		{
			returnForPrimTypes<RetT, Returner, ConstantReturner, UniformDistributionReturner, RectDistributionReturner, CircleDistributionReturner, DeflectDistributionReturner, GeneratedHeightReturner, NegativeReturner, MinReturner, MaxReturner, ClampReturner>(name),
			returnForNonPrimTypes<RetT, SumReturner, DifferenceReturner, ProductReturner, QuotientReturner>(name, std::make_integer_sequence<int, ReturnerTypesCount>(), std::make_integer_sequence<int, ReturnerTypesCount>())
		});
}

template <class RetT>
	requires std::derived_from<RetT, Returner<RetT::RetType>>
RetT* getReturnerPointerFromJson(const nl::json& j)
{
	if (!j.contains("name")) return nullptr;
	return getReturnerPointerFromName<RetT>(j["name"].get<std::string>());
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
			ptr = std::unique_ptr<RetT>(getReturnerPointerFromJson<RetT>(j));
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

template<class RetT>
struct ImGuiButtonForNonPrimTypeGenerator
{

	template <template<ReturnType, ReturnType, ReturnType> class T, ReturnType t1, ReturnType t2>
	void ImGuiButtonForNonPrimTypeW2T()
	{
		constexpr bool requirement = requires { {std::derived_from<T<RetT::RetType, t1, t2>, RetT>}; };
		if constexpr (!requirement) return;
		else tree_map[ReturnType_to_text[t1]][ReturnType_to_text[t2]] = std::unique_ptr<RetT>(new T<RetT::RetType, t1, t2>);
	}

	template <template<ReturnType, ReturnType, ReturnType> class T, ReturnType t1, int ...types>
	void ImGuiButtonForNonPrimTypeW1T(std::integer_sequence<int, types...>)
	{
		(ImGuiButtonForNonPrimTypeW2T<T, t1, ReturnType(types)>(), ...);
	}

	template <template<ReturnType, ReturnType, ReturnType> class T, int ...types1, int... types2>
	void ImGuiButtonForNonPrimType(std::unique_ptr<RetT>& returner, std::integer_sequence<int, types1...>, std::integer_sequence<int, types2...> i2)
	{
		(ImGuiButtonForNonPrimTypeW1T<T, ReturnType(types1)>(i2), ...);
		for (auto& pair : tree_map) if (ImGui::TreeNodeEx(std::format("First type: {}", pair.first).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			for (auto& second_pair : pair.second) if (ImGui::SmallButton(std::format("Second type: {}", second_pair.first).c_str())) returner = std::move(second_pair.second);
			ImGui::TreePop();
		}
	}

	std::map<std::string, std::map<std::string, std::unique_ptr<RetT>>> tree_map{};

};

template<class RetT>
	requires std::derived_from<RetT, Returner<RetT::RetType>>
void toImGui(std::unique_ptr<RetT>& returner, const std::string& format)
{
	ImGui::Text(format.c_str());
	if (returner)
	{
		if (ImGui::BeginPopupContextItem(std::format("For reseting##{}", (uintptr_t)&returner).c_str()))
		{
			if (ImGui::SmallButton("Reset to None")) returner.reset();
			if (ImGui::SmallButton("Copy"))
			{
				nl::json j;
				returner->to_json(j);
				doodle_jump_clipboard = j.dump();
			}
			if (!doodle_jump_clipboard.empty() && ImGui::SmallButton("Paste"))
			{
				nl::json j = nl::json::parse(doodle_jump_clipboard);
				returner = std::unique_ptr<RetT>(getReturnerPointerFromJson<RetT>(j));
				if (returner) returner->from_json(j);
				else ImGui::OpenPopup("Paste failed");
				if (ImGui::BeginPopup("Paste failed"))
				{
					ImGui::Text("Cannot paste here");
					ImGui::EndPopup();
				}
			}
			ImGui::EndPopup();
		}
	}
	else
	{
		if (ImGui::BeginPopupContextItem(std::format("For new returner##{}", (uintptr_t)&returner).c_str()))
		{
			if (ImGui::SmallButton("Create new")) ImGui::OpenPopup("Choose a type");
			if (ImGui::BeginPopup("Choose a type"))
			{
#define IMGUI_BUTTON_FOR_PRIM_TYPE(x) if constexpr( requires { {std::derived_from<x<RetT::RetType>, RetT>}; }) if(ImGui::SmallButton(#x)) returner = std::unique_ptr<RetT>(new x<RetT::RetType>);
				IMGUI_BUTTON_FOR_PRIM_TYPE(Returner)
				IMGUI_BUTTON_FOR_PRIM_TYPE(ConstantReturner)
				IMGUI_BUTTON_FOR_PRIM_TYPE(UniformDistributionReturner)
				IMGUI_BUTTON_FOR_PRIM_TYPE(RectDistributionReturner)
				IMGUI_BUTTON_FOR_PRIM_TYPE(CircleDistributionReturner)
				IMGUI_BUTTON_FOR_PRIM_TYPE(DeflectDistributionReturner)
				IMGUI_BUTTON_FOR_PRIM_TYPE(GeneratedHeightReturner)
				IMGUI_BUTTON_FOR_PRIM_TYPE(NegativeReturner)
				IMGUI_BUTTON_FOR_PRIM_TYPE(MinReturner)
				IMGUI_BUTTON_FOR_PRIM_TYPE(MaxReturner)
				IMGUI_BUTTON_FOR_PRIM_TYPE(ClampReturner)
#undef IMGUI_BUTTON_FOR_PRIM_TYPE
#define IMGUI_TREE_FOR_NON_PRIM_TYPE(x)\
if(ImGui::TreeNode(#x))\
{\
	ImGuiButtonForNonPrimTypeGenerator<RetT>{}.ImGuiButtonForNonPrimType<x>(returner, std::make_integer_sequence<int, ReturnerTypesCount>(), std::make_integer_sequence<int, ReturnerTypesCount>());\
	ImGui::TreePop();\
}

				IMGUI_TREE_FOR_NON_PRIM_TYPE(SumReturner)
				IMGUI_TREE_FOR_NON_PRIM_TYPE(DifferenceReturner)
				IMGUI_TREE_FOR_NON_PRIM_TYPE(ProductReturner)
				IMGUI_TREE_FOR_NON_PRIM_TYPE(QuotientReturner)
#undef IMGUI_TREE_FOR_NON_PRIM_TYPE
				ImGui::EndPopup();
			}

			if (!doodle_jump_clipboard.empty() && ImGui::SmallButton("Paste"))
			{
				nl::json j = nl::json::parse(doodle_jump_clipboard);
				returner = std::unique_ptr<RetT>(getReturnerPointerFromJson<RetT>(j));
				if (returner) returner->from_json(j);
				else ImGui::OpenPopup("Paste failed");
				if (ImGui::BeginPopup("Paste failed"))
				{
					ImGui::Text("Cannot paste here");
					ImGui::EndPopup();
				}
			}
			ImGui::EndPopup();

		}
	}
	ImGui::SameLine();
	if (returner) returner->toImGui();
	else ImGui::Text(std::format("None (type: {})", ReturnType_to_text[RetT::RetType]).c_str());
}
