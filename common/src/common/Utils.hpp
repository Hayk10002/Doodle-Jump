#pragma once
#include <functional>
#include <SFML/Graphics.hpp>
#include <deque>

#include <nlohmann/json.hpp>
namespace utils
{
	sf::FloatRect getViewArea(const sf::RenderTarget& target);
	sf::Vector2f element_wiseProduct(sf::Vector2f f, sf::Vector2f s);

	template<class T>
	using Returner = std::function<T()>;
	
	template<std::default_initializable T>
	const Returner<T> def_returner = []() { return T{}; };

	template<class T>
	Returner<T> constant_returner(const T& val)
	{
		return[val]() { return val; };
	}


	float getYFrom5Nums(float x1, float y1, float x2, float y2, float x);
	float getYFrom5NumsClamped(float x1, float y1, float x2, float y2, float x);
	float getYFrom5NumsLeftClamped(float x1, float y1, float x2, float y2, float x);
	float getYFrom5NumsRightClamped(float x1, float y1, float x2, float y2, float x);
	bool getTrueWithChance(float chance);
	float randomSignWithChance(float positive_chance);
	size_t pickOneWithRelativeProbabilities(const std::deque<float>& relative_probabilities);
}

namespace nlohmann
{
	template <class F>
	struct adl_serializer<std::function<F>>
	{
		template<class F>
		static void to_json(nl::json& j, std::function<F> ret) {};

		template<class F>
		static void from_json(const nl::json& j, std::function<F>& ret) {};
	};
}