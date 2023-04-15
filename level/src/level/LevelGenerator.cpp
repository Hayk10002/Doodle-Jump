// Generation
#include "LevelGenerator.hpp"
#include <ranges>
#include <algorithm>

#include <Thor/Math.hpp>
#include <common/Utils.hpp>

#include <level/Level.hpp>


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

void to_json(nl::json& j, const LevelGenerator::GenerationSettings& generation_settings)
{
	j["repeate_count"] = generation_settings.repeate_count;
}

void from_json(const nl::json& j, LevelGenerator::GenerationSettings& generation_settings)
{
	if (j.contains("repeate_count")) j["repeate_count"].get_to(generation_settings.repeate_count);
}

LevelGenerator::LevelGenerator():
	m_generator(getGenerator())
{}

void LevelGenerator::update()
{
	while (true)
	{
		m_generating_area = getGeneratingArea();
		if (m_generating_area.height < 0 || !m_generator.resume()) break;
	}
}

void LevelGenerator::setGenerationSettings(GenerationSettings settings)
{
	m_settings = settings;
}

float LevelGenerator::getGeneratedHeight()
{
	return m_generated_height;
}

void LevelGenerator::setLevelForGeneration(Level * level_ptr)
{
	Generation::setCurrentLevelForGenerating(level_ptr);
}

Level* LevelGenerator::getLevelForGeneration()
{
	return Generation::getCurrentLevelForGenerating();
}

sf::FloatRect LevelGenerator::getGeneratingArea()
{
	sf::FloatRect area{ utils::getViewArea(getLevelForGeneration()->window) };
	if (m_generated_height > area.top + area.height) m_generated_height = area.top + area.height;
	area.top -= area.height / 2;
	area.height = m_generated_height - area.top;
	return area;
}

LevelGenerator::Generator LevelGenerator::getGenerator()
{
	for (int i = 0; m_settings.repeate_count == -1 || i < m_settings.repeate_count; i++)
	{
		m_generated_height -= m_generation->generate(m_generated_height, m_generating_area.left, m_generating_area.left + m_generating_area.width);
		co_await std::suspend_always{};
	}
}

void to_json(nl::json& j, const LevelGenerator& level_generator)
{
	j["generation_settings"] = level_generator.m_settings;
	j["generation"] = level_generator.m_generation;
}

void from_json(const nl::json& j, LevelGenerator& level_generator)
{
	if(j.contains("generation_settings")) j["generation_settings"].get_to(level_generator.m_settings);
	if (j.contains("generation")) j["generation"].get_to(level_generator.m_generation);
}



float TileGeneration::generateImpl(float generated_height, float left, float right)
{
	Tile* tile = getTile();
	if (!tile) return 0.0f;
	sf::Vector2f position = position_returner->getValue();
	tile->setPosition(position.x, generated_height - position.y);
	getCurrentLevelForGenerating()->addTile(tile);
	if (item_generation)
	{
		item_generation->tile = tile;
		item_generation->generate(generated_height, left, right);
	}
	return height_returner->getValue();
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
	sf::Vector2f position = position_returner->getValue();
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
	auto* tile = new HorizontalSlidingTile(speed_returner->getValue());
	tile->updateMovingLocation(left_returner->getValue(), right_returner->getValue());
	return tile;
}

Tile* VerticalSlidingTileGeneration::getTile()
{
	auto* tile = new VerticalSlidingTile(speed_returner->getValue());
	tile->updateMovingLocation(top_returner->getValue(), bottom_returner->getValue());
	return tile;
}

Tile* DecayedTileGeneration::getTile()
{
	auto* tile = new DecayedTile(speed_returner->getValue());
	tile->updateMovingLocation(left_returner->getValue(), right_returner->getValue());
	return tile;
}

Tile* BombTileGeneration::getTile()
{
	auto* tile = new BombTile(exploding_height_returner->getValue());
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
	for (const auto& returner : offset_returners) tile->addNewPosition(returner->getValue());
	return tile;
}

Tile* ClusterTileGeneration::getTile()
{
	auto* tile = new ClusterTile(id);
	for (const auto& returner : offset_returners) tile->addNewPosition(returner->getValue());
	return tile;
}



Item* SpringGeneration::getItem()
{
	auto* item = new Spring(tile);
	item->setOffsetFromTile(tile_offset_returner->getValue());
	return item;
}

Item* TrampolineGeneration::getItem()
{
	auto* item = new Trampoline(tile);
	item->setOffsetFromTile(tile_offset_returner->getValue());
	return item;
}

Item* PropellerHatGeneration::getItem()
{
	auto* item = new PropellerHat(tile);
	item->setOffsetFromTile(tile_offset_returner->getValue());
	return item;
}

Item* JetpackGeneration::getItem()
{
	auto* item = new Jetpack(tile);
	item->setOffsetFromTile(tile_offset_returner->getValue());
	return item;
}

Item* SpringShoesGeneration::getItem()
{
	auto* item = new SpringShoes(tile, max_use_count_returner->getValue(), &getCurrentLevelForGenerating()->tiles, &getCurrentLevelForGenerating()->monsters);
	item->setOffsetFromTile(tile_offset_returner->getValue());
	return item;
}



Monster* BlueOneEyedMonsterGeneration::getMonster()
{
	auto* monster = new BlueOneEyedMonster(speed_returner->getValue());
	monster->updateMovingLocation(left_returner->getValue(), right_returner->getValue());
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
	auto* monster = new TheTerrifyingMonster(speed_returner->getValue());
	monster->updateMovingLocation(left_returner->getValue(), right_returner->getValue());
	return monster;
}



float GenerationWithChance::generateImpl(float generated_height, float left, float right)
{
	if (!generation) return 0.0f;
	if (utils::getTrueWithChance(chance_returner->getValue())) return generation->generate(generated_height, left, right);
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
	std::deque<float> chances = generations | std::views::transform([](const ProbabilityGenerationPair& val) { return val.relative_probability_returner->getValue(); }) | std::ranges::to<std::deque>();
	size_t pair_ind = utils::pickOneWithRelativeProbabilities(chances);
	if (generations[pair_ind].generation) return generations[pair_ind].generation->generate(generated_height, left, right);
	return 0.0f;
}



void Generation::to_json(nl::json& j) const
{
#define TEXT(x) #x
	j["name"] = TEXT(Generation);
#undef TEXT
}

void Generation::from_json(const nl::json& j)
{
}

void TileGeneration::to_json(nl::json& j) const
{
	Generation::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(TileGeneration);
#undef TEXT
	j["position_returner"] = position_returner;
	j["height_returner"] = height_returner;
	j["item_generation"] = item_generation;
}

void TileGeneration::from_json(const nl::json& j)
{
	Generation::from_json(j);
	if(j.contains("position_returner")) j["position_returner"].get_to(position_returner);
	if(j.contains("height_returner")) j["height_returner"].get_to(height_returner);
	if(j.contains("item_generation")) j["item_generation"].get_to(item_generation);
}

void ItemGeneration::to_json(nl::json& j) const
{
	Generation::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(ItemGeneration);
#undef TEXT
	j["tile_offset_returner"] = tile_offset_returner;
}

void ItemGeneration::from_json(const nl::json& j)
{
	Generation::from_json(j);
	if (j.contains("tile_offset_returner")) j["tile_offset_returner"].get_to(tile_offset_returner);
}

void MonsterGeneration::to_json(nl::json& j) const
{
	Generation::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(MonsterGeneration);
#undef TEXT
	j["position_returner"] = position_returner;
}

void MonsterGeneration::from_json(const nl::json& j)
{
	Generation::from_json(j);
	if (j.contains("position_returner")) j["position_returner"].get_to(position_returner);
}

void NormalTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(NormalTileGeneration);
#undef TEXT
}

void NormalTileGeneration::from_json(const nl::json& j)
{
	TileGeneration::from_json(j);
}

void HorizontalSlidingTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(HorizontalSlidingTileGeneration);
#undef TEXT
	j["speed_returner"] = speed_returner;
	j["left_returner"] = left_returner;
	j["right_returner"] = right_returner;
}

void HorizontalSlidingTileGeneration::from_json(const nl::json& j)
{
	TileGeneration::from_json(j);
	if (j.contains("speed_returner")) j["speed_returner"].get_to(speed_returner);
	if (j.contains("left_returner")) j["left_returner"].get_to(left_returner);
	if (j.contains("right_returner")) j["right_returner"].get_to(right_returner);
}

void VerticalSlidingTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(VerticalSlidingTileGeneration);
#undef TEXT
	j["speed_returner"] = speed_returner;
	j["top_returner"] = top_returner;
	j["bottom_returner"] = bottom_returner;
}

void VerticalSlidingTileGeneration::from_json(const nl::json& j)
{
	TileGeneration::from_json(j);
	if (j.contains("speed_returner")) j["speed_returner"].get_to(speed_returner);
	if (j.contains("top_returner")) j["top_returner"].get_to(top_returner);
	if (j.contains("bottom_returner")) j["bottom_returner"].get_to(bottom_returner);
}

void DecayedTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(DecayedTileGeneration);
#undef TEXT
	j["speed_returner"] = speed_returner;
	j["left_returner"] = left_returner;
	j["right_returner"] = right_returner;
}

void DecayedTileGeneration::from_json(const nl::json& j)
{
	TileGeneration::from_json(j);
	if (j.contains("speed_returner")) j["speed_returner"].get_to(speed_returner);
	if (j.contains("left_returner")) j["left_returner"].get_to(left_returner);
	if (j.contains("right_returner")) j["right_returner"].get_to(right_returner);
}

void BombTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(BombTileGeneration);
#undef TEXT
	j["exploding_height_returner"] = exploding_height_returner;
}

void BombTileGeneration::from_json(const nl::json& j)
{
	TileGeneration::from_json(j);
	if (j.contains("exploding_height_returner")) j["exploding_height_returner"].get_to(exploding_height_returner);
}

void OneTimeTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(OneTimeTileGeneration);
#undef TEXT
}

void OneTimeTileGeneration::from_json(const nl::json& j)
{
	TileGeneration::from_json(j);
}

void TeleportTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(TeleportTileGeneration);
#undef TEXT
	j["offset_returners"] = nl::json::array();
	for (auto& returner : offset_returners) j["offset_returners"].push_back(returner);
}

void TeleportTileGeneration::from_json(const nl::json& j)
{
	TileGeneration::from_json(j);
	if (j.contains("offset_returners"))
	{
		offset_returners.clear();
		for (size_t i = 0; i < j["offset_returners"].size(); i++) offset_returners.push_back(j["offset_returners"][i]);
	}
}

void ClusterTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(ClusterTileGeneration);
#undef TEXT
	j["offset_returners"] = nl::json::array();
	for (auto& returner : offset_returners) j["offset_returners"].push_back(returner);
	j["id"] = id;
}

void ClusterTileGeneration::from_json(const nl::json& j)
{
	TileGeneration::from_json(j);
	if (j.contains("offset_returners"))
	{
		offset_returners.clear();
		for (size_t i = 0; i < j["offset_returners"].size(); i++) offset_returners.push_back(j["offset_returners"][i]);
	}
	if (j.contains("id")) j["id"].get_to(id);
}

void SpringGeneration::to_json(nl::json& j) const
{
	ItemGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(SpringGeneration);
#undef TEXT
}

void SpringGeneration::from_json(const nl::json& j)
{
	ItemGeneration::from_json(j);
}

void TrampolineGeneration::to_json(nl::json& j) const
{
	ItemGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(TrampolineGeneration);
#undef TEXT
}

void TrampolineGeneration::from_json(const nl::json& j)
{
	ItemGeneration::from_json(j);
}

void PropellerHatGeneration::to_json(nl::json& j) const
{
	ItemGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(PropellerHatGeneration);
#undef TEXT
}

void PropellerHatGeneration::from_json(const nl::json& j)
{
	ItemGeneration::from_json(j);
}

void JetpackGeneration::to_json(nl::json& j) const
{
	ItemGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(JetpackGeneration);
#undef TEXT
}

void JetpackGeneration::from_json(const nl::json& j)
{
	ItemGeneration::from_json(j);
}

void SpringShoesGeneration::to_json(nl::json& j) const
{
	ItemGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(SpringShoesGeneration);
#undef TEXT
	j["max_use_count_returner"] = max_use_count_returner;
}

void SpringShoesGeneration::from_json(const nl::json& j)
{
	ItemGeneration::from_json(j);
	if (j.contains("max_use_count_returner")) j["max_use_count_returner"].get_to(max_use_count_returner);
}

void BlueOneEyedMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(BlueOneEyedMonsterGeneration);
#undef TEXT
	j["speed_returner"] = speed_returner;
	j["left_returner"] = left_returner;
	j["right_returner"] = right_returner;
}

void BlueOneEyedMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
	if (j.contains("speed_returner")) j["speed_returner"].get_to(speed_returner);
	if (j.contains("left_returner")) j["left_returner"].get_to(left_returner);
	if (j.contains("right_returner")) j["right_returner"].get_to(right_returner);
}

void CamronMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(CamronMonsterGeneration);
#undef TEXT
}

void CamronMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

void PurpleSpiderMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(PurpleSpiderMonsterGeneration);
#undef TEXT
}

void PurpleSpiderMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

void LargeBlueMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(LargeBlueMonsterGeneration);
#undef TEXT
}

void LargeBlueMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

void UFOGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(UFOGeneration);
#undef TEXT
}

void UFOGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

void BlackHoleGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(BlackHoleGeneration);
#undef TEXT
}

void BlackHoleGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

void OvalGreenMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(OvalGreenMonsterGeneration);
#undef TEXT
}

void OvalGreenMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

void FlatGreenMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(FlatGreenMonsterGeneration);
#undef TEXT
}

void FlatGreenMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

void LargeGreenMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(LargeGreenMonsterGeneration);
#undef TEXT
}

void LargeGreenMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

void BlueWingedMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(BlueWingedMonsterGeneration);
#undef TEXT
}

void BlueWingedMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

void TheTerrifyingMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(TheTerrifyingMonsterGeneration);
#undef TEXT
	j["speed_returner"] = speed_returner;
	j["left_returner"] = left_returner;
	j["right_returner"] = right_returner;
}

void TheTerrifyingMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
	if (j.contains("speed_returner")) j["speed_returner"].get_to(speed_returner);
	if (j.contains("left_returner")) j["left_returner"].get_to(left_returner);
	if (j.contains("right_returner")) j["right_returner"].get_to(right_returner);
}

void GenerationWithChance::to_json(nl::json& j) const
{
	Generation::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(GenerationWithChance);
#undef TEXT
	j["generation"] = generation;
	j["chance_returner"] = chance_returner;
}

void GenerationWithChance::from_json(const nl::json& j)
{
	Generation::from_json(j);
	if (j.contains("generation")) j["generation"].get_to(generation);
	if (j.contains("chance_returner")) j["chance_returner"].get_to(chance_returner);
}

void GroupGeneration::to_json(nl::json& j) const
{
	Generation::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(GroupGeneration);
#undef TEXT
	for (auto& generation : generations) j["generations"].push_back(generation);
}

void GroupGeneration::from_json(const nl::json& j)
{
	Generation::from_json(j);
	if (j.contains("generations"))
	{
		generations.clear();
		for (size_t i = 0; i < j["generations"].size(); i++) generations.push_back(j["generations"][i]);
	}
}

void ConsecutiveGeneration::to_json(nl::json& j) const
{
	Generation::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(ConsecutiveGeneration);
#undef TEXT
	for (auto& generation : generations) j["generations"].push_back(generation);
}

void ConsecutiveGeneration::from_json(const nl::json& j)
{
	Generation::from_json(j);
	if (j.contains("generations"))
	{
		generations.clear();
		for (size_t i = 0; i < j["generations"].size(); i++) generations.push_back(j["generations"][i]);
	}
}

void to_json(nl::json& j, const PickOneGeneration::ProbabilityGenerationPair& pair)
{
	j["relative_probility_returner"] = pair.relative_probability_returner;
	j["generation"] = pair.generation;
}

void from_json(const nl::json& j, PickOneGeneration::ProbabilityGenerationPair& pair)
{
	if (j.contains("relative_probability_returner")) j["relative_probability_returner"].get_to(pair.relative_probability_returner);
	if (j.contains("generation")) j["generation"].get_to(pair.generation);
}

void PickOneGeneration::to_json(nl::json& j) const
{
	Generation::to_json(j);
#define TEXT(x) #x
	j["name"] = TEXT(PickOneGeneration);
#undef TEXT
	for (auto& generation : generations) j["generations"].push_back(generation);
}

void PickOneGeneration::from_json(const nl::json& j)
{
	Generation::from_json(j);
	if (j.contains("generations"))
	{
		generations.clear();
		for (size_t i = 0; i < j["generations"].size(); i++) generations.push_back(j["generations"][i]);
	}
}
