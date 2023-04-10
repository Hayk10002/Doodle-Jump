#include "Utils.hpp"

#include <ranges>
#include <Thor/Math.hpp>

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

	float getYFrom5Nums(float x1, float y1, float x2, float y2, float x)
	{
		return ((y1 - y2) * x + x1 * y2 - y1 * x2) / (x1 - x2);
	}

	float getYFrom5NumsClamped(float x1, float y1, float x2, float y2, float x)
	{
		x = std::clamp(x, x1, x2);
		return getYFrom5Nums(x1, y1, x2, y2, x);
	}

	float getYFrom5NumsLeftClamped(float x1, float y1, float x2, float y2, float x)
	{
		x = std::max(x, x1);
		return getYFrom5Nums(x1, y1, x2, y2, x);
	}

	float getYFrom5NumsRightClamped(float x1, float y1, float x2, float y2, float x)
	{
		x = std::min(x, x2);
		return getYFrom5Nums(x1, y1, x2, y2, x);
	}

	bool getTrueWithChance(float chance)
	{
		return thor::Distributions::uniform(0.f, 1.f)() <= chance;
	}

	float randomSignWithChance(float positive_chance)
	{
		return getTrueWithChance(positive_chance) ? 1.f : -1.f;
	}
	
	size_t pickOneWithRelativeProbabilities(const std::deque<float>& relative_probabilities)
	{
		float sum = std::ranges::fold_left(relative_probabilities, 0, std::plus());
		float random_val = thor::Distributions::uniform(0.f, sum)();
		for (size_t i = 0; i < relative_probabilities.size(); i++)
		{
			if (random_val <= relative_probabilities[i]) return i;
			random_val -= relative_probabilities[i];
		}
		return 0;
	}
}
