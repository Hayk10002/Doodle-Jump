#pragma once
#include <SFML/Graphics.hpp>
namespace utils
{
	sf::FloatRect getViewArea(const sf::RenderTarget& target);
	sf::Vector2f element_wiseProduct(sf::Vector2f f, sf::Vector2f s);
}