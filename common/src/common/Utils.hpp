#pragma once
#include <functional>
#include <deque>

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <nlohmann/json.hpp>
namespace utils
{
	sf::FloatRect getViewArea(const sf::RenderTarget& target);
	sf::Vector2f element_wiseProduct(sf::Vector2f f, sf::Vector2f s);
	template<class T>
	sf::Vector2<T> toSFMLVector2(const ImVec2& vec)
	{
		return sf::Vector2<T>(vec.x, vec.y);
	}
	template<class T>
	ImVec2 toImVec2(const sf::Vector2<T> vec)
	{
		return ImVec2(vec.x, vec.y);
	}
	template<class T>
	sf::Vector2<T> yFlipped(sf::Vector2<T> vec)
	{
		return { vec.x, -vec.y };
	} 

	float getYFrom5Nums(float x1, float y1, float x2, float y2, float x);
	float getYFrom5NumsClamped(float x1, float y1, float x2, float y2, float x);
	float getYFrom5NumsLeftClamped(float x1, float y1, float x2, float y2, float x);
	float getYFrom5NumsRightClamped(float x1, float y1, float x2, float y2, float x);
	bool getTrueWithChance(float chance);
	float randomSignWithChance(float positive_chance);
	size_t pickOneWithRelativeProbabilities(const std::deque<float>& relative_probabilities);

	bool isMouseHoveringRect(sf::Vector2f coords, sf::Vector2f half_size, const sf::RenderWindow& window);
}
