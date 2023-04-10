#include "LevelGenerator.hpp"
#include <ranges>
#include <algorithm>

#include <Thor/Math.hpp>
#include <common/Utils.hpp>

#include <level/Level.hpp>


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

LevelGenerator::LevelGenerator(Level *level) :
	m_window(&level->window),
	m_tiles(&level->tiles),
	m_items(&level->items),
	m_monsters(&level->monsters),
	m_generator(getGenerator()),
	m_generated_height(level->window.mapPixelToCoords(sf::Vector2i(level->window.getSize())).y)
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
	Tile* returning_tile{};
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
	HorizontalSlidingTile* tile = new HorizontalSlidingTile(randomSignWithChance(0.5) * m_gen_stats.getHorizontalTileSpeedMultiplierFromGeneratedHeight(generated_height) * m_gen_stats.horizontal_tile_speed_distribution());
	tile->updateMovingLocation(left, right);
	tile->setPosition(thor::Distributions::uniform(left + tile->getCollisionBoxSize().x / 2, right - tile->getCollisionBoxSize().x / 2)(), generated_height);
	m_tiles->m_tiles.push_back(std::unique_ptr<HorizontalSlidingTile>(tile));
	return tile;
}

void LevelGenerator::generateDecayedTile(float generated_height, float left, float right, float tile_density_inv)
{
	if (tile_density_inv < 2) return;
	float speed = (getTrueWithChance(m_gen_stats.getDecayedMovingTileChanceFromGeneratedHeight(generated_height)) ?
		randomSignWithChance(0.5) * m_gen_stats.getDecayedMovingTileSpeedMultiplierFromGeneratedHeight(generated_height) * m_gen_stats.decayed_moving_tile_speed_distribution() :
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
	if (getTrueWithChance(m_gen_stats.getHorizontalTileChanceFromGeneratedHeight(generated_height)))
		current_tile = generateHorizontalTile(generated_height, left, right, tile_density_inv);
	else current_tile = generateNormalTiles(generated_height, left, right, tile_density_inv);

	if (getTrueWithChance(m_gen_stats.getSpringChanceFromGeneratedHeight(generated_height)))
	{
		generateSpringOnTile(current_tile);
		tile_density_inv = 6;
	}
	else if (getTrueWithChance(m_gen_stats.getTrampolineChanceFromGeneratedHeight(generated_height)))
	{
		generateTrampolineOnTile(current_tile);
		tile_density_inv = 6;
	}
	else if (getTrueWithChance(m_gen_stats.getPropellerHatChanceFromGeneratedHeight(generated_height)))
	{
		generatePropellerHatOnTile(current_tile);
		tile_density_inv = 6;
	}
	else if (getTrueWithChance(m_gen_stats.getJetpackChanceFromGeneratedHeight(generated_height)))
	{
		generateJetpackOnTile(current_tile);
		tile_density_inv = 6;
	}
	else if (getTrueWithChance(m_gen_stats.spring_shoes_chance))
	{
		generateSpringShoesOnTile(current_tile);
		tile_density_inv = 6;
	}

	if (getTrueWithChance(m_gen_stats.getDecayedTileChanceFromGeneratedHeight(generated_height)))
		generateDecayedTile(generated_height, left, right, tile_density_inv);

	generated_height -= std::min(m_gen_stats.tile_maximum_interval.y, tile_density_inv * m_gen_stats.tile_minimum_interval.y);
}



float Generation::generateImpl(float generated_height, float left, float right)
{
	return 0.0f;
}

float Generation::generate(float generated_height, float left, float right)
{
	if (!level) return 0.0f;
	else return generateImpl(generated_height, left, right);
}

void Generation::setCurrentLevelForGenerating(Level* level)
{
	Generation::level = level;
}

Level* Generation::getCurrentLevelForGenerating()
{
	return level;
}



LevelGeneratorNew::Generator::~Generator()
{
	if (coro) coro.destroy();
}

LevelGeneratorNew::Generator::Generator(Generator&& oth) noexcept
{
	oth.coro = nullptr;
}

LevelGeneratorNew::Generator& LevelGeneratorNew::Generator::operator=(Generator&& oth) noexcept
{
	coro = std::exchange(oth.coro, nullptr);
	return *this;
}

bool LevelGeneratorNew::Generator::resume()
{
	if (!coro.done()) coro.resume();
	return !coro.done();
}

LevelGeneratorNew::Generator LevelGeneratorNew::Generator::promise_type::get_return_object()
{
	return Generator{ handle_type::from_promise(*this) };
}

LevelGeneratorNew::LevelGeneratorNew():
	m_generator(getGenerator())
{}

void LevelGeneratorNew::update()
{
	while (true)
	{
		m_generating_area = getGeneratingArea();
		if (m_generating_area.height < 0 || !m_generator.resume()) break;
	}
}

void LevelGeneratorNew::setGenerationSettings(GenerationSettings settings)
{
	m_settings = settings;
}

void LevelGeneratorNew::setLevelForGeneration(Level * level_ptr)
{
	Generation::setCurrentLevelForGenerating(level_ptr);
}

Level* LevelGeneratorNew::getLevelForGeneration()
{
	return Generation::getCurrentLevelForGenerating();
}

sf::FloatRect LevelGeneratorNew::getGeneratingArea()
{
	sf::FloatRect area{ utils::getViewArea(getLevelForGeneration()->window) };
	if (m_generated_height > area.top + area.height) m_generated_height = area.top + area.height;
	area.top -= area.height / 2;
	area.height = m_generated_height - area.top;
	return area;
}

LevelGeneratorNew::Generator LevelGeneratorNew::getGenerator()
{
	for (int i = 0; m_settings.repeate_count == -1 || i < m_settings.repeate_count; i++)
	{
		m_generated_height -= m_generation->generate(m_generated_height, m_generating_area.left, m_generating_area.left + m_generating_area.width);
		co_await std::suspend_always{};
	}
}



float TileGeneration::generateImpl(float generated_height, float left, float right)
{
	Tile* tile = getTile();
	if (!tile) return 0.0f;
	sf::Vector2f position = position_returner();
	tile->setPosition(position.x, generated_height - position.y);
	getCurrentLevelForGenerating()->addTile(tile);
	if (item_generation)
	{
		item_generation->tile = tile;
		item_generation->generate(generated_height, left, right);
	}
	return height_returner();
}

Tile* TileGeneration::getTile()
{
	return nullptr;
}



float ItemGeneration::generateImpl(float, float, float)
{
	Item* item = getItem();
	if (!item) return 0.0f;
	getCurrentLevelForGenerating()->addItem(item);
	return 0.0f;
}

Item* ItemGeneration::getItem()
{
	return nullptr;
}



float MonsterGeneration::generateImpl(float generated_height, float, float)
{
	Monster* monster = getMonster();
	if (!monster) return 0.0f;
	sf::Vector2f position = position_returner();
	monster->setPosition(position.x, generated_height - position.y);
	getCurrentLevelForGenerating()->addMonster(monster);
	return 0.0f;
}

Monster* MonsterGeneration::getMonster()
{
	return nullptr;
}



Tile* NormalTileGeneration::getTile()
{
	auto* tile = new NormalTile;
	return tile;
}

Tile* HorizontalSlidingTileGeneration::getTile()
{
	auto* tile = new HorizontalSlidingTile(speed_returner());
	tile->updateMovingLocation(left_returner(), right_returner());
	return tile;
}

Tile* VerticalSlidingTileGeneration::getTile()
{
	auto* tile = new VerticalSlidingTile(speed_returner());
	tile->updateMovingLocation(top_returner(), bottom_returner());
	return tile;
}

Tile* DecayedTileGeneration::getTile()
{
	auto* tile = new DecayedTile(speed_returner());
	tile->updateMovingLocation(left_returner(), right_returner());
	return tile;
}

Tile* BombTileGeneration::getTile()
{
	auto* tile = new BombTile(exploding_height_returner());
	tile->setSpecUpdate([tile, this](sf::Time) { tile->updateHeight(getCurrentLevelForGenerating()->window); });
	return tile;
}

Tile* OneTimeTileGeneration::getTile()
{
	auto* tile = new OneTimeTile;
	return tile;
}

Tile* TeleportTileGeneration::getTile()
{
	auto* tile = new TeleportTile;
	for (const auto& returner : offset_returners) tile->addNewPosition(returner());
	return tile;
}

Tile* ClusterTileGeneration::getTile()
{
	auto* tile = new ClusterTile(id);
	for (const auto& returner : offset_returners) tile->addNewPosition(returner());
	return tile;
}



Item* SpringGeneration::getItem()
{
	auto* item = new Spring(tile);
	item->setOffsetFromTile(tile_offset_returner());
	return item;
}

Item* TrampolineGeneration::getItem()
{
	auto* item = new Trampoline(tile);
	item->setOffsetFromTile(tile_offset_returner());
	return item;
}

Item* PropellerHatGeneration::getItem()
{
	auto* item = new PropellerHat(tile);
	item->setOffsetFromTile(tile_offset_returner());
	return item;
}

Item* JetpackGeneration::getItem()
{
	auto* item = new Jetpack(tile);
	item->setOffsetFromTile(tile_offset_returner());
	return item;
}

Item* SpringShoesGeneration::getItem()
{
	auto* item = new SpringShoes(tile, max_use_count_returner(), &getCurrentLevelForGenerating()->tiles, &getCurrentLevelForGenerating()->monsters);
	item->setOffsetFromTile(tile_offset_returner());
	return item;
}



Monster* BlueOneEyedMonsterGeneration::getMonster()
{
	auto* monster = new BlueOneEyedMonster(speed_returner());
	monster->updateMovingLocation(left_returner(), right_returner());
	return monster;
}

Monster* CamronMonsterGeneration::getMonster()
{
	auto* monster = new CamronMonster;
	return monster;
}

Monster* PurpleSpiderMonsterGeneration::getMonster()
{
	auto* monster = new PurpleSpiderMonster;
	return monster;
}

Monster* LargeBlueMonsterGeneration::getMonster()
{
	auto* monster = new LargeBlueMonster;
	return monster;
}

Monster* UFOGeneration::getMonster()
{
	auto* monster = new UFO;
	return monster;
}

Monster* BlackHoleGeneration::getMonster()
{
	auto* monster = new BlackHole;
	return monster;
}

Monster* OvalGreenMonsterGeneration::getMonster()
{
	auto* monster = new OvalGreenMonster;
	return monster;
}

Monster* FlatGreenMonsterGeneration::getMonster()
{
	auto* monster = new FlatGreenMonster;
	return monster;
}

Monster* LargeGreenMonsterGeneration::getMonster()
{
	auto* monster = new LargeGreenMonster;
	return monster;
}

Monster* BlueWingedMonsterGeneration::getMonster()
{
	auto* monster = new BlueWingedMonster;
	return monster;
}

Monster* TheTerrifyingMonsterGeneration::getMonster()
{
	auto* monster = new TheTerrifyingMonster(speed_returner());
	monster->updateMovingLocation(left_returner(), right_returner());
	return monster;
}



float GenerationWithChance::generateImpl(float generated_height, float left, float right)
{
	if (!generation) return 0.0f;
	if (getTrueWithChance(chance_returner())) return generation->generate(generated_height, left, right);
	return 0.0f;

}

float GroupGeneration::generateImpl(float generated_height, float left, float right)
{
	float max_height = 0.0f;
	for (auto& generation : generations) if (generation)
	{
		float height = generation->generate(generated_height, left, right);
		max_height = std::max(max_height, height);
	}
	return max_height;
}

float ConsecutiveGeneration::generateImpl(float generated_height, float left, float right)
{
	float sum_height = 0.0f;
	for (auto& generation : generations) if (generation) sum_height += generation->generate(generated_height - sum_height, left, right);
	return sum_height;
}

float PickOneGeneration::generateImpl(float generated_height, float left, float right)
{
	std::deque<float> chances = generations | std::views::transform([](const ProbabilityGenerationPair& val) { return val.relative_probability_returner(); }) | std::ranges::to<std::deque>();
	size_t pair_ind = utils::pickOneWithRelativeProbabilities(chances);
	if (generations[pair_ind].generation) return generations[pair_ind].generation->generate(generated_height, left, right);
	return 0.0f;
}