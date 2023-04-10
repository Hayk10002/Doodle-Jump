#pragma once
#include <functional>
#include <SFML/Graphics.hpp>
#include <deque>
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