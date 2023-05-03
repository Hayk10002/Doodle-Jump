// Generation
#include "LevelGenerator.hpp"
#include <ranges>
#include <algorithm>

#include <Thor/Math.hpp>
#include <common/Utils.hpp>
#include <imgui.h>

#include <level/Level.hpp>
#include <common/Returners.hpp>

#define TEXT(x) #x

float Generation::generateImpl(float generated_height, float left, float right)
{
	return 0.0f;
}

void Generation::toImGuiImpl()
{
}

Generation::Generation():
	ImGui_id(++ImGui_id_counter)
{
}

float Generation::generate(float generated_height, float left, float right)
{
	if (!level) return 0.0f;
	else return generateImpl(generated_height, left, right);
}

void Generation::toImGui()
{
	if (ImGui::TreeNodeEx(std::format("{}##{}", getName(), ImGui_id).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		toImGuiImpl();
		ImGui::TreePop();
	}
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

void LevelGenerator::GenerationSettings::toImGui()
{	
	if (ImGui::TreeNodeEx("##settings", ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		ImGui::Text("Repeate count (-1 for infinite):"); ImGui::SameLine(); ::toImGui(repeate_count);
		ImGui::TreePop();
	}
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

const std::unique_ptr<Generation>& LevelGenerator::getGeneration() const
{
	return m_generation;
}

void LevelGenerator::setGenerationSettings(GenerationSettings settings)
{
	m_settings = settings;
}

float LevelGenerator::getGeneratedHeight()
{
	return m_generated_height;
}

void LevelGenerator::toImGui()
{
	if (ImGui::TreeNodeEx("Level Generator", ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		ImGui::Text("Settings:"); ImGui::SameLine(); m_settings.toImGui();
		::toImGui<Generation>(m_generation, "Generation:");
		ImGui::TreePop();
	}
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
	float height = height_returner ? height_returner->getValue() : 0.0f;
	sf::Vector2f position = position_returner ? position_returner->getValue() : sf::Vector2f{};
	Tile* tile = getTile();
	if (!tile) return height;
	tile->setPosition(position.x, generated_height - position.y);
	getCurrentLevelForGenerating()->addTile(tile);
	if (item_generation)
	{
		item_generation->tile = tile;
		item_generation->generate(generated_height, left, right);
	}
	return height;
}

void TileGeneration::toImGuiImpl()
{
	Generation::toImGuiImpl();
	::toImGui<Returner<Position>>(position_returner, "Position:");
	::toImGui<Returner<Height>>(height_returner, "Height:");
	::toImGui<ItemGeneration>(item_generation, "Item:");
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

void ItemGeneration::toImGuiImpl()
{
	Generation::toImGuiImpl();
	::toImGui<Returner<XOffset>>(tile_offset_returner, "Tile offset:");
}

Item* ItemGeneration::getItem()
{
	return nullptr;
}



float MonsterGeneration::generateImpl(float generated_height, float, float)
{
	Monster* monster = getMonster();
	if (!monster) return 0.0f;
	sf::Vector2f position = position_returner ? position_returner->getValue() : sf::Vector2f{};
	monster->setPosition(position.x, generated_height - position.y);
	getCurrentLevelForGenerating()->addMonster(monster);
	return 0.0f;
}

void MonsterGeneration::toImGuiImpl()
{
	Generation::toImGuiImpl();
	::toImGui<Returner<Position>>(position_returner, "Position:");
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
	auto* tile = new HorizontalSlidingTile(speed_returner ? speed_returner->getValue() : 0.0f);
	tile->updateMovingLocation(left_returner ? left_returner->getValue() : 0.0f, right_returner ? right_returner->getValue() : 0.0f);
	return tile;
}

void HorizontalSlidingTileGeneration::toImGuiImpl()
{
	TileGeneration::toImGuiImpl();
	::toImGui<Returner<XSpeed>>(speed_returner, "Speed:");
	::toImGui<Returner<XBoundary>>(left_returner, "Left Boundary:");
	::toImGui<Returner<XBoundary>>(right_returner, "Right Boundary:");
}

Tile* VerticalSlidingTileGeneration::getTile()
{
	auto* tile = new VerticalSlidingTile(speed_returner ? speed_returner->getValue() : 0.0f);
	tile->updateMovingLocation(top_returner ? top_returner->getValue() : 0.0f, bottom_returner ? bottom_returner->getValue() : 0.0f);
	return tile;
}

void VerticalSlidingTileGeneration::toImGuiImpl()
{
	TileGeneration::toImGuiImpl();
	::toImGui<Returner<YSpeed>>(speed_returner, "Speed:");
	::toImGui<Returner<YBoundary>>(top_returner, "Top Boundary:");
	::toImGui<Returner<YBoundary>>(bottom_returner, "Bottom Boundary:");
}

Tile* DecayedTileGeneration::getTile()
{
	auto* tile = new DecayedTile(speed_returner ? speed_returner->getValue() : 0.0f);
	tile->updateMovingLocation(left_returner ? left_returner->getValue() : 0.0f, right_returner ? right_returner->getValue() : 0.0f);
	return tile;
}

void DecayedTileGeneration::toImGuiImpl()
{
	TileGeneration::toImGuiImpl();
	::toImGui<Returner<XSpeed>>(speed_returner, "Speed:");
	::toImGui<Returner<XBoundary>>(left_returner, "Left Boundary:");
	::toImGui<Returner<XBoundary>>(right_returner, "Right Boundary:");
}

Tile* BombTileGeneration::getTile()
{
	auto* tile = new BombTile(exploding_height_returner ? exploding_height_returner->getValue() : 0.0f);
	tile->setSpecUpdate([tile, this](sf::Time) { tile->updateHeight(getCurrentLevelForGenerating()->window); });
	return tile;
}

void BombTileGeneration::toImGuiImpl()
{
	TileGeneration::toImGuiImpl();
	::toImGui<Returner<Height>>(exploding_height_returner, "Explotion height:");
}

Tile* OneTimeTileGeneration::getTile()
{
	auto* tile = new OneTimeTile;
	return tile;
}

Tile* TeleportTileGeneration::getTile()
{
	auto* tile = new TeleportTile;
	for (const auto& returner : offset_returners) tile->addNewPosition(returner ? returner->getValue() : sf::Vector2f{});
	return tile;
}

void TeleportTileGeneration::toImGuiImpl()
{
	TileGeneration::toImGuiImpl();
	ImGui::Text("Offsets from the first position:"); ImGui::SameLine();
	if (ImGui::TreeNodeEx(std::format("(size: {})###rets", offset_returners.size()).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		ImGui::TextDisabled("[0]: (0, 0)");
		for (size_t i = 0; i < offset_returners.size(); i++)
		{
			::toImGui<Returner<Offset>>(offset_returners[i], std::format("[{}]:", i + 1).c_str());
		}
		if (ImGui::SmallButton("New")) offset_returners.emplace_back();
		ImGui::TreePop();
	}
}

Tile* ClusterTileGeneration::getTile()
{
	auto* tile = new ClusterTile(id);
	for (const auto& returner : offset_returners) tile->addNewPosition(returner ? returner->getValue() : sf::Vector2f{});
	return tile;
}

void ClusterTileGeneration::toImGuiImpl()
{
	TileGeneration::toImGuiImpl();
	ImGui::Text("Offsets from the first position:"); ImGui::SameLine();
	if (ImGui::TreeNodeEx(std::format("(size: {})###rets", offset_returners.size()).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		ImGui::TextDisabled("[0]: (0, 0)");
		for (size_t i = 0; i < offset_returners.size(); i++)
		{
			::toImGui<Returner<Offset>>(offset_returners[i], std::format("[{}]:", i + 1).c_str());
		}
		if (ImGui::SmallButton("New")) offset_returners.emplace_back();
		ImGui::TreePop();
	}
	ImGui::Text("Id:"); ImGui::SameLine(); id.toImGui();
}



Item* SpringGeneration::getItem()
{
	auto* item = new Spring(tile);
	item->setOffsetFromTile(tile_offset_returner ? tile_offset_returner->getValue() : 0.0f);
	return item;
}

Item* TrampolineGeneration::getItem()
{
	auto* item = new Trampoline(tile);
	item->setOffsetFromTile(tile_offset_returner ? tile_offset_returner->getValue() : 0.0f);
	return item;
}

Item* PropellerHatGeneration::getItem()
{
	auto* item = new PropellerHat(tile);
	item->setOffsetFromTile(tile_offset_returner ? tile_offset_returner->getValue() : 0.0f);
	return item;
}

Item* JetpackGeneration::getItem()
{
	auto* item = new Jetpack(tile);
	item->setOffsetFromTile(tile_offset_returner ? tile_offset_returner->getValue() : 0.0f);
	return item;
}

Item* SpringShoesGeneration::getItem()
{
	auto* item = new SpringShoes(tile, max_use_count_returner ? max_use_count_returner->getValue() : 0u, &getCurrentLevelForGenerating()->tiles, &getCurrentLevelForGenerating()->monsters);
	item->setOffsetFromTile(tile_offset_returner ? tile_offset_returner->getValue() : 0.0f);
	return item;
}

void SpringShoesGeneration::toImGuiImpl()
{
	ItemGeneration::toImGuiImpl();
	::toImGui<Returner<SizeTValue>>(max_use_count_returner, "Max use count:");
}



Monster* BlueOneEyedMonsterGeneration::getMonster()
{
	auto* monster = new BlueOneEyedMonster(speed_returner ? speed_returner->getValue() : 0.0f);
	monster->updateMovingLocation(left_returner ? left_returner->getValue() : 0.0f, right_returner ? right_returner->getValue() : 0.0f);
	return monster;
}

void BlueOneEyedMonsterGeneration::toImGuiImpl()
{
	MonsterGeneration::toImGuiImpl();
	::toImGui<Returner<XSpeed>>(speed_returner, "Speed:");
	::toImGui<Returner<XBoundary>>(left_returner, "Left Boundary:");
	::toImGui<Returner<XBoundary>>(right_returner, "Right Boundary:");
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
	auto* monster = new TheTerrifyingMonster(speed_returner ? speed_returner->getValue() : sf::Vector2f{});
	monster->updateMovingLocation(left_returner ? left_returner->getValue() : 0.0f, right_returner ? right_returner->getValue() : 0.0f);
	return monster;
}

void TheTerrifyingMonsterGeneration::toImGuiImpl()
{
	MonsterGeneration::toImGuiImpl();
	::toImGui<Returner<Speed>>(speed_returner, "Speed:");
	::toImGui<Returner<XBoundary>>(left_returner, "Left Boundary:");
	::toImGui<Returner<XBoundary>>(right_returner, "Right Boundary:");
}



float GenerationWithChance::generateImpl(float generated_height, float left, float right)
{
	if (!generation) return 0.0f;
	if (utils::getTrueWithChance(chance_returner ? chance_returner->getValue() : 0.0f)) return generation->generate(generated_height, left, right);
	return 0.0f;

}

void GenerationWithChance::toImGuiImpl()
{
	Generation::toImGuiImpl();
	::toImGui<Generation>(generation, "Generation:");
	::toImGui<Returner<Chance>>(chance_returner, "Chance:");
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

void GroupGeneration::toImGuiImpl()
{
	Generation::toImGuiImpl();
	ImGui::Text("Generations:"); ImGui::SameLine();
	if (ImGui::TreeNodeEx(std::format("(size: {})###gens", generations.size()).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		for (size_t i = 0; i < generations.size(); i++) ::toImGui<Generation>(generations[i], std::format("[{}]:", i));
		if (ImGui::SmallButton("New")) generations.emplace_back();
		ImGui::TreePop();
	}
}

float ConsecutiveGeneration::generateImpl(float generated_height, float left, float right)
{
	float sum_height = 0.0f;
	for (auto& generation : generations) if (generation) sum_height += generation->generate(generated_height - sum_height, left, right);
	return sum_height;
}

void ConsecutiveGeneration::toImGuiImpl()
{	
	Generation::toImGuiImpl();
	ImGui::Text("Generations:"); ImGui::SameLine();
	if (ImGui::TreeNodeEx(std::format("(size: {})###gens", generations.size()).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		for (size_t i = 0; i < generations.size(); i++) ::toImGui<Generation>(generations[i], std::format("[{}]:", i));
		if (ImGui::SmallButton("New")) generations.emplace_back();
		ImGui::TreePop();
	}
}

void PickOneGeneration::ProbabilityGenerationPair::toImGui()
{
	if (ImGui::TreeNodeEx(std::format("Gen. & prob.##{}", (uintptr_t)this).c_str()), ImGuiTreeNodeFlags_SpanAvailWidth)
	{
		::toImGui<Generation>(generation, "Generation:");
		::toImGui<Returner<RelativeProbability>>(relative_probability_returner, "Relative probability:");
		ImGui::TreePop();
	}
}

float PickOneGeneration::generateImpl(float generated_height, float left, float right)
{
	std::deque<float> chances = generations | std::views::transform([](const ProbabilityGenerationPair& val) { return val.relative_probability_returner ? val.relative_probability_returner->getValue() : 0.0f; }) | std::ranges::to<std::deque>();
	size_t pair_ind = utils::pickOneWithRelativeProbabilities(chances);
	if (generations[pair_ind].generation) return generations[pair_ind].generation->generate(generated_height, left, right);
	return 0.0f;
}

void PickOneGeneration::toImGuiImpl()
{
	Generation::toImGuiImpl();
	ImGui::Text("Generations:"); ImGui::SameLine();
	if (ImGui::TreeNodeEx(std::format("(size: {})###gens", generations.size()).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		for (size_t i = 0; i < generations.size(); i++)
		{
			ImGui::Text(std::format("[{}]:", i).c_str()); ImGui::SameLine(); generations[i].toImGui();
		}
		if (ImGui::SmallButton("New")) generations.emplace_back(std::unique_ptr<Returner<RelativeProbability>>(), std::unique_ptr<Generation>());
		ImGui::TreePop();
	}
}


std::string Generation::getName() const
{
	return TEXT(Generation);
}

void Generation::to_json(nl::json& j) const
{
	j["name"] = getName();
}

void Generation::from_json(const nl::json& j)
{
}

std::string TileGeneration::getName() const
{
	return TEXT(TileGeneration);
}

void TileGeneration::to_json(nl::json& j) const
{
	Generation::to_json(j);
	j["name"] = getName();
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

std::string ItemGeneration::getName() const
{
	return TEXT(ItemGeneration);
}

void ItemGeneration::to_json(nl::json& j) const
{
	Generation::to_json(j);
	j["name"] = getName();
	j["tile_offset_returner"] = tile_offset_returner;
}

void ItemGeneration::from_json(const nl::json& j)
{
	Generation::from_json(j);
	if (j.contains("tile_offset_returner")) j["tile_offset_returner"].get_to(tile_offset_returner);
}

std::string MonsterGeneration::getName() const
{
	return TEXT(MonsterGeneration);
}

void MonsterGeneration::to_json(nl::json& j) const
{
	Generation::to_json(j);
	j["name"] = getName();
	j["position_returner"] = position_returner;
}

void MonsterGeneration::from_json(const nl::json& j)
{
	Generation::from_json(j);
	if (j.contains("position_returner")) j["position_returner"].get_to(position_returner);
}

std::string NormalTileGeneration::getName() const
{
	return TEXT(NormalTileGeneration);
}

void NormalTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
	j["name"] = getName();
}

void NormalTileGeneration::from_json(const nl::json& j)
{
	TileGeneration::from_json(j);
}

std::string HorizontalSlidingTileGeneration::getName() const
{
	return TEXT(HorizontalSlidingTileGeneration);
}

void HorizontalSlidingTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
	j["name"] = getName();
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

std::string VerticalSlidingTileGeneration::getName() const
{
	return TEXT(VerticalSlidingTileGeneration);
}

void VerticalSlidingTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
	j["name"] = getName();
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

std::string DecayedTileGeneration::getName() const
{
	return TEXT(DecayedTileGeneration);
}

void DecayedTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
	j["name"] = getName();
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

std::string BombTileGeneration::getName() const
{
	return TEXT(BombTileGeneration);
}

void BombTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
	j["name"] = getName();
	j["exploding_height_returner"] = exploding_height_returner;
}

void BombTileGeneration::from_json(const nl::json& j)
{
	TileGeneration::from_json(j);
	if (j.contains("exploding_height_returner")) j["exploding_height_returner"].get_to(exploding_height_returner);
}

std::string OneTimeTileGeneration::getName() const
{
	return TEXT(OneTimeTileGeneration);
}

void OneTimeTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
	j["name"] = getName();
}

void OneTimeTileGeneration::from_json(const nl::json& j)
{
	TileGeneration::from_json(j);
}

std::string TeleportTileGeneration::getName() const
{
	return TEXT(TeleportTileGeneration);
}

void TeleportTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
	j["name"] = getName();
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

std::string ClusterTileGeneration::getName() const
{
	return TEXT(ClusterTileGeneration);
}

void ClusterTileGeneration::to_json(nl::json& j) const
{
	TileGeneration::to_json(j);
	j["name"] = getName();
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

std::string SpringGeneration::getName() const
{
	return TEXT(SpringGeneration);
}

void SpringGeneration::to_json(nl::json& j) const
{
	ItemGeneration::to_json(j);
	j["name"] = getName();
}

void SpringGeneration::from_json(const nl::json& j)
{
	ItemGeneration::from_json(j);
}

std::string TrampolineGeneration::getName() const
{
	return TEXT(TrampolineGeneration);
}

void TrampolineGeneration::to_json(nl::json& j) const
{
	ItemGeneration::to_json(j);
	j["name"] = getName();
}

void TrampolineGeneration::from_json(const nl::json& j)
{
	ItemGeneration::from_json(j);
}

std::string PropellerHatGeneration::getName() const
{
	return TEXT(PropellerHatGeneration);
}

void PropellerHatGeneration::to_json(nl::json& j) const
{
	ItemGeneration::to_json(j);
	j["name"] = getName();
}

void PropellerHatGeneration::from_json(const nl::json& j)
{
	ItemGeneration::from_json(j);
}

std::string JetpackGeneration::getName() const
{
	return TEXT(JetpackGeneration);
}

void JetpackGeneration::to_json(nl::json& j) const
{
	ItemGeneration::to_json(j);
	j["name"] = getName();
}

void JetpackGeneration::from_json(const nl::json& j)
{
	ItemGeneration::from_json(j);
}

std::string SpringShoesGeneration::getName() const
{
	return TEXT(SpringShoesGeneration);
}

void SpringShoesGeneration::to_json(nl::json& j) const
{
	ItemGeneration::to_json(j);
	j["name"] = getName();
	j["max_use_count_returner"] = max_use_count_returner;
}

void SpringShoesGeneration::from_json(const nl::json& j)
{
	ItemGeneration::from_json(j);
	if (j.contains("max_use_count_returner")) j["max_use_count_returner"].get_to(max_use_count_returner);
}

std::string BlueOneEyedMonsterGeneration::getName() const
{
	return TEXT(BlueOneEyedMonsterGeneration);
}

void BlueOneEyedMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
	j["name"] = getName();
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

std::string CamronMonsterGeneration::getName() const
{
	return TEXT(CamronMonsterGeneration);
}

void CamronMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
	j["name"] = getName();
}

void CamronMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

std::string PurpleSpiderMonsterGeneration::getName() const
{
	return TEXT(PurpleSpiderMonsterGeneration);
}

void PurpleSpiderMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
	j["name"] = getName();
}

void PurpleSpiderMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

std::string LargeBlueMonsterGeneration::getName() const
{
	return TEXT(LargeBlueMonsterGeneration);
}

void LargeBlueMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
	j["name"] = getName();
}

void LargeBlueMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

std::string UFOGeneration::getName() const
{
	return TEXT(UFOGeneration);
}

void UFOGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
	j["name"] = getName();
}

void UFOGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

std::string BlackHoleGeneration::getName() const
{
	return TEXT(BlackHoleGeneration);
}

void BlackHoleGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
	j["name"] = getName();
}

void BlackHoleGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

std::string OvalGreenMonsterGeneration::getName() const
{
	return TEXT(OvalGreenMonsterGeneration);
}

void OvalGreenMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
	j["name"] = getName();
}

void OvalGreenMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

std::string FlatGreenMonsterGeneration::getName() const
{
	return TEXT(FlatGreenMonsterGeneration);
}

void FlatGreenMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
	j["name"] = getName();
}

void FlatGreenMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

std::string LargeGreenMonsterGeneration::getName() const
{
	return TEXT(LargeGreenMonsterGeneration);
}

void LargeGreenMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
	j["name"] = getName();
}

void LargeGreenMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

std::string BlueWingedMonsterGeneration::getName() const
{
	return TEXT(BlueWingedMonsterGeneration);
}

void BlueWingedMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
	j["name"] = getName();
}

void BlueWingedMonsterGeneration::from_json(const nl::json& j)
{
	MonsterGeneration::from_json(j);
}

std::string TheTerrifyingMonsterGeneration::getName() const
{
	return TEXT(TheTerrifyingMonsterGeneration);
}

void TheTerrifyingMonsterGeneration::to_json(nl::json& j) const
{
	MonsterGeneration::to_json(j);
	j["name"] = getName();
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

std::string GenerationWithChance::getName() const
{
	return TEXT(GenerationWithChance);
}

void GenerationWithChance::to_json(nl::json& j) const
{
	Generation::to_json(j);
	j["name"] = getName();
	j["generation"] = generation;
	j["chance_returner"] = chance_returner;
}

void GenerationWithChance::from_json(const nl::json& j)
{
	Generation::from_json(j);
	if (j.contains("generation")) j["generation"].get_to(generation);
	if (j.contains("chance_returner")) j["chance_returner"].get_to(chance_returner);
}

std::string GroupGeneration::getName() const
{
	return TEXT(GroupGeneration);
}

void GroupGeneration::to_json(nl::json& j) const
{
	Generation::to_json(j);
	j["name"] = getName();
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

std::string ConsecutiveGeneration::getName() const
{
	return TEXT(ConsecutiveGeneration);
}

void ConsecutiveGeneration::to_json(nl::json& j) const
{
	Generation::to_json(j);
	j["name"] = getName();
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

std::string PickOneGeneration::getName() const
{
	return TEXT(PickOneGeneration);
}

void PickOneGeneration::to_json(nl::json& j) const
{
	Generation::to_json(j);
	j["name"] = getName();
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

#undef TEXT
