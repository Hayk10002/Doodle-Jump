#pragma once

#include <SFML/Graphics.hpp>

struct Previews
{
	inline static sf::RenderWindow* window = nullptr;

	static void point(sf::Vector2f val, sf::Vector2f zero);
	static void height(float height, sf::Vector2f zero);
	static void offset(sf::Vector2f offset, sf::Vector2f zero);
	static void speed(sf::Vector2f speed, sf::Vector2f zero);
	static void xBoundary(float boundary, sf::Vector2f zero);
	static void yBoundary(float boundary, sf::Vector2f zero);
	static void rect(sf::Vector2f half_size, sf::Vector2f zero);
	static void circle(float radius, sf::Vector2f zero);
	static void deflect(float max_rotation, sf::Vector2f center, sf::Vector2f zero);

private:
	static bool checkWindow();
};


