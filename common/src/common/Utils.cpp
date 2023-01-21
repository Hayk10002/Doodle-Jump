#include "Utils.hpp"

namespace utils
{
	sf::FloatRect getViewArea(const sf::RenderTarget& target)
	{
		return sf::FloatRect{ target.mapPixelToCoords({0, 0}), target.mapPixelToCoords(sf::Vector2i{target.getSize()}) - target.mapPixelToCoords({0, 0}) };
	}
	sf::Vector2f element_wiseProduct(sf::Vector2f f, sf::Vector2f s)
	{
		return sf::Vector2f{ f.x * s.x, f.y * s.y };
	}
}
