#include "Utils.hpp"

namespace utils
{
	sf::FloatRect getViewArea(const sf::RenderTarget& target)
	{
		return sf::FloatRect{ target.mapPixelToCoords({0, 0}), target.mapPixelToCoords(sf::Vector2i{target.getSize()}) - target.mapPixelToCoords({0, 0}) };
	}
}
