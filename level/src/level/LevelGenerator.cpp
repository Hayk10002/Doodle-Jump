#include "LevelGenerator.hpp"

#include <Thor/Math.hpp>
#include <common/Utils.hpp>

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

LevelGenerator::Generator::~Generator()
{
	if (coro) coro.destroy();
}

LevelGenerator::Generator::Generator(Generator&& oth) noexcept
{
	oth.coro = nullptr;
}

LevelGenerator::Generator& LevelGenerator::Generator::operator=(Generator&& oth) noexcept
{
	coro = std::exchange(oth.coro, nullptr);
	return *this;
}

bool LevelGenerator::Generator::resume()
{
	if (!coro.done()) coro.resume();
	return !coro.done();
}

LevelGenerator::Generator LevelGenerator::Generator::promise_type::get_return_object()
{
	return Generator{ handle_type::from_promise(*this) };
}

LevelGenerator::LevelGenerator(sf::RenderWindow* window, Tiles* tiles, Items* items, Monsters* monsters) :
	m_window(window),
	m_tiles(tiles),
	m_items(items),
	m_monsters(monsters),
	m_generator(getGenerator()),
	m_generated_height(window->mapPixelToCoords(sf::Vector2i(window->getSize())).y)
{}

void LevelGenerator::update()
{
	while(true)
	{
		m_generating_area = getGeneratingArea();
		if (m_generating_area.height >= 0) m_generator.resume();
		else break;
	}
}

sf::FloatRect LevelGenerator::getGeneratingArea()
{
	sf::FloatRect area{ utils::getViewArea(*m_window)};
	if (m_generated_height > area.top + area.height) m_generated_height = area.top + area.height;
	area.top -= area.height / 2;
	area.height = m_generated_height - area.top;
	return area;
}

LevelGenerator::Generator LevelGenerator::getGenerator()
{
	while (true)
	{
		generateNormalGeneration(m_generated_height, m_generating_area.left, m_generating_area.left + m_generating_area.width);
		
		co_await std::suspend_always{};
	}
}

bool LevelGenerator::NormalGenerationStats::getTrueWithChance(float chance)
{
	return from_0_to_1() <= chance;
}

float LevelGenerator::NormalGenerationStats::randomSignWithChance(float positive_chance)
{
	return getTrueWithChance(positive_chance) ? 1.f : -1.f;
}

float LevelGenerator::NormalGenerationStats::getTileDensityInverseFromGeneratedHeight(float generated_height)
{
	return getYFrom5NumsRightClamped(0, 1, minimum_density_height, minimum_density_inverse, -generated_height);
}

float LevelGenerator::NormalGenerationStats::getCurrentTileDensityInverse(float generated_height, float left, float right)
{
	float tile_density_inv = getTileDensityInverseFromGeneratedHeight(generated_height);
	tile_density_inv /= ((right - left) / tile_maximum_interval.x);
	tile_density_inv += thor::Distributions::uniform(-density_inverse_deviation, density_inverse_deviation)();
	tile_density_inv = std::max(0.f, tile_density_inv);
	return tile_density_inv;
}

float LevelGenerator::NormalGenerationStats::getHorizontalTileChanceFromGeneratedHeight(float generated_height)
{
	return getYFrom5NumsClamped(horizontal_tile_minimum_height, 0, all_horizontal_tile_height, 1, -generated_height);
}

float LevelGenerator::NormalGenerationStats::getHorizontalTileSpeedMultiplierFromGeneratedHeight(float generated_height)
{
	return getYFrom5NumsClamped(horizontal_tile_minimum_height, horizontal_tile_minimum_speed_multiplier, horizontal_tile_maximum_speed_height, 1, -generated_height);
}

float LevelGenerator::NormalGenerationStats::getDecayedTileChanceFromGeneratedHeight(float generated_height)
{
	return getYFrom5NumsClamped(decayed_tile_minimum_height, 0, decayed_tile_maximum_chance_height, decayed_tile_maximum_chance, -generated_height);
}

float LevelGenerator::NormalGenerationStats::getDecayedMovingTileChanceFromGeneratedHeight(float generated_height)
{
	return getYFrom5NumsClamped(decayed_moving_tile_minimum_height, 0, decayed_all_moving_tile_height, 1, -generated_height);
}

float LevelGenerator::NormalGenerationStats::getDecayedMovingTileSpeedMultiplierFromGeneratedHeight(float generated_height)
{
	return getYFrom5NumsClamped(decayed_moving_tile_minimum_height, decayed_moving_tile_minimum_speed_multiplier, decayed_moving_tile_maximum_speed_height, 1, -generated_height);
}

float LevelGenerator::NormalGenerationStats::getSpringChanceFromGeneratedHeight(float generated_height)
{
	return getYFrom5NumsClamped(0, spring_maximum_chance, spring_minimum_chance_height, spring_minimum_chance, -generated_height);
}

float LevelGenerator::NormalGenerationStats::getTrampolineChanceFromGeneratedHeight(float generated_height)
{
	return getYFrom5NumsClamped(0, trampoline_maximum_chance, trampoline_minimum_chance_height, trampoline_minimum_chance, -generated_height);
}

float LevelGenerator::NormalGenerationStats::getPropellerHatChanceFromGeneratedHeight(float generated_height)
{
	return getYFrom5NumsClamped(0, propeller_hat_maximum_chance, propeller_hat_minimum_chance_height, propeller_hat_minimum_chance, -generated_height);
}

float LevelGenerator::NormalGenerationStats::getJetpackChanceFromGeneratedHeight(float generated_height)
{
	return getYFrom5NumsClamped(0, jetpack_maximum_chance, jetpack_minimum_chance_height, jetpack_minimum_chance, -generated_height);
}

Tile* LevelGenerator::generateNormalTiles(float generated_height, float left, float right, float& tile_density_inv)
{
	NormalTile* tile;
	Tile* returning_tile;
	if (tile_density_inv < 1)
	{
		size_t tile_count = std::max(1, int((right - left) / m_gen_stats.tile_maximum_interval.x));
		tile_count = std::max(1, int(float(tile_count) * (1 - tile_density_inv)));
		float offset = thor::Distributions::uniform(0.f, m_gen_stats.tile_maximum_interval.x)();
		size_t random_index = thor::Distributions::uniform(0, tile_count - 1)();
		for (size_t i = 0; i < tile_count; i++)
		{
			tile = new NormalTile{};
			float pos = left + std::fmodf((offset + i * (right - left) / tile_count), (right - left));
			float precise_pos = thor::Distributions::uniform(pos - 200, pos + 200)();
			precise_pos = std::clamp(precise_pos, left + tile->getCollisionBoxSize().x / 2, right - tile->getCollisionBoxSize().x / 2);
			tile->setPosition(precise_pos, generated_height);
			m_tiles->m_tiles.push_back(std::unique_ptr<NormalTile>(tile));
			if (i == random_index) returning_tile = tile;
		}

		do
		{
			tile_density_inv = m_gen_stats.getCurrentTileDensityInverse(generated_height, left, right);
		} while (tile_density_inv < 1);

	}
	else
	{
		tile = new NormalTile{};
		tile->setPosition(thor::Distributions::uniform(left + tile->getCollisionBoxSize().x / 2, right - tile->getCollisionBoxSize().x / 2)(), generated_height);
		m_tiles->m_tiles.push_back(std::unique_ptr<NormalTile>(tile));
		returning_tile = tile;
	}
	return returning_tile;
}

Tile* LevelGenerator::generateHorizontalTile(float generated_height, float left, float right, float& tile_density_inv)
{
	tile_density_inv = std::max(1.f, tile_density_inv);
	HorizontalSlidingTile* tile = new HorizontalSlidingTile(m_gen_stats.randomSignWithChance(0.5) * m_gen_stats.getHorizontalTileSpeedMultiplierFromGeneratedHeight(generated_height) * m_gen_stats.horizontal_tile_speed_distribution());
	tile->updateMovingLocation(left, right);
	tile->setPosition(thor::Distributions::uniform(left + tile->getCollisionBoxSize().x / 2, right - tile->getCollisionBoxSize().x / 2)(), generated_height);
	m_tiles->m_tiles.push_back(std::unique_ptr<HorizontalSlidingTile>(tile));
	return tile;
}

void LevelGenerator::generateDecayedTile(float generated_height, float left, float right, float tile_density_inv)
{
	if (tile_density_inv < 2) return;
	float speed = (m_gen_stats.getTrueWithChance(m_gen_stats.getDecayedMovingTileChanceFromGeneratedHeight(generated_height)) ?
		m_gen_stats.randomSignWithChance(0.5) * m_gen_stats.getDecayedMovingTileSpeedMultiplierFromGeneratedHeight(generated_height) * m_gen_stats.decayed_moving_tile_speed_distribution() :
		0);

	DecayedTile* tile = new DecayedTile(speed);
	tile->updateMovingLocation(left, right);
	tile->setPosition(thor::Distributions::uniform(left + tile->getCollisionBoxSize().x / 2, right - tile->getCollisionBoxSize().x / 2)(), generated_height - tile_density_inv / 2 * m_gen_stats.tile_minimum_interval.y);
	m_tiles->m_tiles.push_back(std::unique_ptr<DecayedTile>(tile));
}

void LevelGenerator::generateSpringOnTile(Tile* tile)
{
	Spring* item = new Spring(tile);
	item->setOffsetFromTile(thor::Distributions::uniform(-tile->getCollisionBoxSize().x / 2 + item->getCollisionBoxSize().x / 2, tile->getCollisionBoxSize().x / 2 - item->getCollisionBoxSize().x / 2)());
	m_items->m_items.push_back(std::unique_ptr<Spring>(item));
}

void LevelGenerator::generateSpringShoesOnTile(Tile* tile)
{
	SpringShoes* item = new SpringShoes(tile, m_gen_stats.spring_shoes_max_use_count_distribution(), m_tiles, m_monsters);
	item->setOffsetFromTile(thor::Distributions::uniform(-tile->getCollisionBoxSize().x / 2 + item->getCollisionBoxSize().x / 2, tile->getCollisionBoxSize().x / 2 - item->getCollisionBoxSize().x / 2)());
	m_items->m_items.push_back(std::unique_ptr<SpringShoes>(item));
}

void LevelGenerator::generateTrampolineOnTile(Tile* tile)
{
	Trampoline* item = new Trampoline(tile);
	item->setOffsetFromTile(thor::Distributions::uniform(-tile->getCollisionBoxSize().x / 2 + item->getCollisionBoxSize().x / 2, tile->getCollisionBoxSize().x / 2 - item->getCollisionBoxSize().x / 2)());
	m_items->m_items.push_back(std::unique_ptr<Trampoline>(item));
}

void LevelGenerator::generatePropellerHatOnTile(Tile* tile)
{
	PropellerHat* item = new PropellerHat(tile);
	item->setOffsetFromTile(thor::Distributions::uniform(-tile->getCollisionBoxSize().x / 2 + item->getCollisionBoxSize().x / 2, tile->getCollisionBoxSize().x / 2 - item->getCollisionBoxSize().x / 2)());
	m_items->m_items.push_back(std::unique_ptr<PropellerHat>(item));
}

void LevelGenerator::generateJetpackOnTile(Tile* tile)
{
	Jetpack* item = new Jetpack(tile);
	item->setOffsetFromTile(thor::Distributions::uniform(-tile->getCollisionBoxSize().x / 2 + item->getCollisionBoxSize().x / 2, tile->getCollisionBoxSize().x / 2 - item->getCollisionBoxSize().x / 2)());
	m_items->m_items.push_back(std::unique_ptr<Jetpack>(item));
}

void LevelGenerator::generateNormalGeneration(float& generated_height, float left, float right)
{
	Tile* current_tile;
	float tile_density_inv = m_gen_stats.getCurrentTileDensityInverse(generated_height, left, right);
	if (m_gen_stats.getTrueWithChance(m_gen_stats.getHorizontalTileChanceFromGeneratedHeight(generated_height)))
		current_tile = generateHorizontalTile(generated_height, left, right, tile_density_inv);
	else current_tile = generateNormalTiles(generated_height, left, right, tile_density_inv);

	if (m_gen_stats.getTrueWithChance(m_gen_stats.getSpringChanceFromGeneratedHeight(generated_height)))
	{
		generateSpringOnTile(current_tile);
		tile_density_inv = 6;
	}
	else if (m_gen_stats.getTrueWithChance(m_gen_stats.getTrampolineChanceFromGeneratedHeight(generated_height)))
	{
		generateTrampolineOnTile(current_tile);
		tile_density_inv = 6;
	}
	else if (m_gen_stats.getTrueWithChance(m_gen_stats.getPropellerHatChanceFromGeneratedHeight(generated_height)))
	{
		generatePropellerHatOnTile(current_tile);
		tile_density_inv = 6;
	}
	else if (m_gen_stats.getTrueWithChance(m_gen_stats.getJetpackChanceFromGeneratedHeight(generated_height)))
	{
		generateJetpackOnTile(current_tile);
		tile_density_inv = 6;
	}
	else if (m_gen_stats.getTrueWithChance(m_gen_stats.spring_shoes_chance))
	{
		generateSpringShoesOnTile(current_tile);
		tile_density_inv = 6;
	}

	if (m_gen_stats.getTrueWithChance(m_gen_stats.getDecayedTileChanceFromGeneratedHeight(generated_height)))
		generateDecayedTile(generated_height, left, right, tile_density_inv);

	generated_height -= std::min(m_gen_stats.tile_maximum_interval.y, tile_density_inv * m_gen_stats.tile_minimum_interval.y);
}