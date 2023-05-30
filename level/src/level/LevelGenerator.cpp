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

float Generation::drawPreviewImpl(sf::Vector2f offset) const
{
	return 0.f;
}

float Generation::drawSubGenerationsPreview(sf::Vector2f offset) const
{
	return 0.f;
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
	if (ImGui::TreeNodeEx(std::format("{}##{}", getName(), ImGui_id).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool canPrev = canPreview();
		if (!canPrev) ImGui::Text("Can't preview");
		else ImGui::Checkbox("Preview", &preview);
		toImGuiImpl();
		ImGui::TreePop();
	}
}

bool Generation::canPreview() const
{
	return false;
}

float Generation::drawPreview(sf::Vector2f offset) const
{
	if (!preview) return drawSubGenerationsPreview(offset);
	return drawPreviewImpl(offset);
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
	if (ImGui::TreeNodeEx("##settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Repeat count (-1 for infinite):"); ImGui::SameLine(); ::toImGui(repeate_count);
		ImGui::TreePop();
	}
}

LevelGenerator::LevelGenerator():
	m_generator(getGenerator())
{}

void LevelGenerator::reset()
{
	if (Level* level = getLevelForGeneration(); level) m_generated_height = level->window.getSize().y;
	else m_generated_height = 1000;
	m_generator = getGenerator();
}

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
	if (ImGui::TreeNodeEx("Level Generator", ImGuiTreeNodeFlags_DefaultOpen))
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
		float height = (m_generation ? m_generation->generate(m_generated_height, m_generating_area.left, m_generating_area.left + m_generating_area.width) : 0.f);
		height = std::max(1.f, height);
		m_generated_height -= height;
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

float TileGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	float mean_height = 0.f;
	if(item_generation) mean_height += item_generation->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	if(position_returner) position_returner->drawPreview(offset);
	if(height_returner) height_returner->drawPreview({ offset.x + (position_returner ? position_returner->getMeanValue() : sf::Vector2f{}).x, offset.y });
	mean_height += (height_returner ? height_returner->getMeanValue() : 0.f);
	return mean_height + Generation::drawPreviewImpl(offset);
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

float ItemGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	if(tile_offset_returner) tile_offset_returner->drawPreview(offset);
	return Generation::drawPreviewImpl(offset);
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

float MonsterGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	if(position_returner) position_returner->drawPreview(offset);
	return Generation::drawPreviewImpl(offset);
}



Tile* NormalTileGeneration::getTile()
{
	auto* tile = new NormalTile;
	return tile;
}

float NormalTileGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite tile = global_sprites["tiles_normal"].createSprite();
	tile.setScale(0.65, 0.65);
	tile.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	tile.setOrigin(64, 20);
	Previews::window->draw(tile);
	return TileGeneration::drawPreviewImpl(offset);
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

float HorizontalSlidingTileGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite tile = global_sprites["tiles_horizontal"].createSprite();
	tile.setScale(0.65, 0.65);
	tile.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	tile.setOrigin(64, 20);
	Previews::window->draw(tile);
	float mean_height = TileGeneration::drawPreviewImpl(offset);
	if(speed_returner) speed_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	if(left_returner) left_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	if(right_returner) right_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	return mean_height;
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

float VerticalSlidingTileGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite tile = global_sprites["tiles_vertical"].createSprite();
	tile.setScale(0.65, 0.65);
	tile.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	tile.setOrigin(64, 20);
	Previews::window->draw(tile);
	float mean_height = TileGeneration::drawPreviewImpl(offset);
	if(speed_returner) speed_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	if(top_returner) top_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	if(bottom_returner) bottom_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	return mean_height;
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

float DecayedTileGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite tile = global_sprites["tiles_decayed_0"].createSprite();
	tile.setScale(0.65, 0.65);
	tile.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	tile.setOrigin(64, 20);
	Previews::window->draw(tile);
	float mean_height = TileGeneration::drawPreviewImpl(offset);
	if(speed_returner) speed_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	if(left_returner) left_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	if(right_returner) right_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	return mean_height;
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

float BombTileGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite tile = global_sprites["tiles_bomb_0"].createSprite();
	tile.setScale(0.65, 0.65);
	tile.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	tile.setOrigin(64, 20);
	Previews::window->draw(tile);
	float mean_height = TileGeneration::drawPreviewImpl(offset);
	if(exploding_height_returner) exploding_height_returner->drawPreview({ offset.x + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}).x, Previews::window->mapPixelToCoords(sf::Vector2i{ Previews::window->getSize() } / 2).y });
	return mean_height;
}

Tile* OneTimeTileGeneration::getTile()
{
	auto* tile = new OneTimeTile;
	return tile;
}

float OneTimeTileGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite tile = global_sprites["tiles_one_time"].createSprite();
	tile.setScale(0.65, 0.65);
	tile.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	tile.setOrigin(64, 20);
	Previews::window->draw(tile);
	return TileGeneration::drawPreviewImpl(offset);
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
	if (ImGui::TreeNodeEx(std::format("(size: {})###rets", offset_returners.size()).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextDisabled("[0]: (0, 0)");
		for (size_t i = 0; i < offset_returners.size(); i++)
		{
			::toImGui<Returner<Position>>(offset_returners[i], std::format("[{}]:", i + 1).c_str());
		}
		if (ImGui::SmallButton("New")) offset_returners.emplace_back();
		if (ImGui::SmallButton("Delete")) ImGui::OpenPopup("For deleting");
		if (ImGui::BeginPopup("For deleting"))
		{
			static size_t ind = 1;
			ImGui::InputScalar("No. to delete", ImGuiDataType_U32, &ind);
			if (ind <= 0) ind = 1;
			if (ind > offset_returners.size()) ind = offset_returners.size();
			if (ImGui::SmallButton("Delete"))
			{ 
				if (offset_returners[ind - 1]) copyReturner<Returner<Position>>(offset_returners[ind - 1], true);
				offset_returners.erase(offset_returners.begin() + ind - 1);
			}
			ImGui::EndPopup();
		}
		ImGui::TreePop();
	}
}

float TeleportTileGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite tile = global_sprites["tiles_teleport_0"].createSprite();
	tile.setScale(0.65, 0.65);
	tile.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	tile.setOrigin(64, 20);
	Previews::window->draw(tile);
	float mean_height = TileGeneration::drawPreviewImpl(offset);
	tile.setColor(sf::Color(255, 255, 255, 128));
	for (size_t i = 0; i < offset_returners.size(); i++)
	{
		const auto& returner = offset_returners[i];
		tile.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}) + utils::yFlipped(returner ? returner->getMeanValue() : sf::Vector2f{}));
		Previews::window->draw(tile);
		if(returner) returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	}
	return mean_height;
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
	if (ImGui::TreeNodeEx(std::format("(size: {})###rets", offset_returners.size()).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextDisabled("[1]: (0, 0)");
		for (size_t i = 0; i < offset_returners.size(); i++)
		{
			::toImGui<Returner<Position>>(offset_returners[i], std::format("[{}]:", i + 2).c_str());
		}
		if (ImGui::SmallButton("New")) offset_returners.emplace_back();
		if (ImGui::SmallButton("Delete")) ImGui::OpenPopup("For deleting");
		if (ImGui::BeginPopup("For deleting"))
		{
			static size_t ind = 2;
			ImGui::InputScalar("No. to delete", ImGuiDataType_U32, &ind);
			if (ind <= 1) ind = 2;
			if (ind > offset_returners.size() + 1) ind = offset_returners.size() + 1;
			if (ImGui::SmallButton("Delete"))
			{ 
				if (offset_returners[ind - 2]) copyReturner<Returner<Position>>(offset_returners[ind - 2], true);
				offset_returners.erase(offset_returners.begin() + ind - 2);
			}
			ImGui::EndPopup();
		}
		ImGui::TreePop();
	}
	ImGui::Text("Id:"); ImGui::SameLine(); id.toImGui();
}

float ClusterTileGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite tile = global_sprites["tiles_cluster"].createSprite();
	tile.setScale(0.65, 0.65);
	tile.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	tile.setOrigin(64, 20);
	Previews::window->draw(tile);
	float mean_height = TileGeneration::drawPreviewImpl(offset);
	tile.setColor(sf::Color(255, 255, 255, 128));
	for (size_t i = 0; i < offset_returners.size(); i++)
	{
		const auto& returner = offset_returners[i];
		tile.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}) + utils::yFlipped(returner ? returner->getMeanValue() : sf::Vector2f{}));
		Previews::window->draw(tile);
		if(returner) returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	}
	return mean_height;
}



Item* SpringGeneration::getItem()
{
	auto* item = new Spring(tile);
	item->setOffsetFromTile(tile_offset_returner ? tile_offset_returner->getValue() : 0.0f);
	return item;
}

float SpringGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite item = global_sprites["items_spring_0"].createSprite();
	item.setScale(0.65, 0.65);
	item.setPosition(offset + sf::Vector2f{ (tile_offset_returner ? tile_offset_returner->getMeanValue() : 0.0f), -14 });
	item.setOrigin(17, 12);
	Previews::window->draw(item);
	return ItemGeneration::drawPreviewImpl(offset);
}

Item* TrampolineGeneration::getItem()
{
	auto* item = new Trampoline(tile);
	item->setOffsetFromTile(tile_offset_returner ? tile_offset_returner->getValue() : 0.0f);
	return item;
}

float TrampolineGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite item = global_sprites["items_trampoline_0"].createSprite();
	item.setScale(0.65, 0.65);
	item.setPosition(offset + sf::Vector2f{ (tile_offset_returner ? tile_offset_returner->getMeanValue() : 0.0f), -16 });
	item.setOrigin(36, 14);
	Previews::window->draw(item);
	return ItemGeneration::drawPreviewImpl(offset);
}

Item* PropellerHatGeneration::getItem()
{
	auto* item = new PropellerHat(tile);
	item->setOffsetFromTile(tile_offset_returner ? tile_offset_returner->getValue() : 0.0f);
	return item;
}

float PropellerHatGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite item = global_sprites["items_propeller_hat_0"].createSprite();
	item.setScale(0.65, 0.65);
	item.setPosition(offset + sf::Vector2f{ (tile_offset_returner ? tile_offset_returner->getMeanValue() : 0.0f), -20 });
	item.setOrigin(29, 19);
	Previews::window->draw(item);
	return ItemGeneration::drawPreviewImpl(offset);
}

Item* JetpackGeneration::getItem()
{
	auto* item = new Jetpack(tile);
	item->setOffsetFromTile(tile_offset_returner ? tile_offset_returner->getValue() : 0.0f);
	return item;
}

float JetpackGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite item = global_sprites["items_jetpack_1"].createSprite();
	item.setScale(0.65, 0.65);
	item.setPosition(offset + sf::Vector2f{ (tile_offset_returner ? tile_offset_returner->getMeanValue() : 0.0f), -30 });
	item.setOrigin(24, 36);
	Previews::window->draw(item);
	return ItemGeneration::drawPreviewImpl(offset);
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

float SpringShoesGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite item = global_sprites["items_spring_shoes_3"].createSprite();
	item.setScale(0.65, 0.65);
	item.setPosition(offset + sf::Vector2f{ (tile_offset_returner ? tile_offset_returner->getMeanValue() : 0.0f), -24 });
	item.setOrigin(26, 14);
	Previews::window->draw(item);
	return ItemGeneration::drawPreviewImpl(offset);
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

float BlueOneEyedMonsterGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite monster = global_sprites["monsters_blue_one_eyed"].createSprite();
	monster.setScale(0.65, 0.65);
	monster.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	monster.setOrigin(37, 49);
	Previews::window->draw(monster);
	float mean_height = MonsterGeneration::drawPreviewImpl(offset);
	if(speed_returner) speed_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	if(left_returner) left_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	if(right_returner) right_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	return mean_height;
}

Monster* CamronMonsterGeneration::getMonster()
{
	auto* monster = new CamronMonster;
	return monster;
}

float CamronMonsterGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite monster = global_sprites["monsters_camron"].createSprite();
	monster.setScale(0.65, 0.65);
	monster.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	monster.setOrigin(44, 35);
	Previews::window->draw(monster);
	return MonsterGeneration::drawPreviewImpl(offset);
}

Monster* PurpleSpiderMonsterGeneration::getMonster()
{
	auto* monster = new PurpleSpiderMonster;
	return monster;
}

float PurpleSpiderMonsterGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite monster = global_sprites["monsters_purple_spider"].createSprite();
	monster.setScale(0.65, 0.65);
	monster.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	monster.setOrigin(55, 49);
	Previews::window->draw(monster);
	return MonsterGeneration::drawPreviewImpl(offset);
}

Monster* LargeBlueMonsterGeneration::getMonster()
{
	auto* monster = new LargeBlueMonster;
	return monster;
}

float LargeBlueMonsterGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite monster = global_sprites["monsters_large_blue"].createSprite();
	monster.setScale(0.65, 0.65);
	monster.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	monster.setOrigin(85, 106);
	Previews::window->draw(monster);
	return MonsterGeneration::drawPreviewImpl(offset);
}

Monster* UFOGeneration::getMonster()
{
	auto* monster = new UFO;
	return monster;
}

float UFOGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite monster = global_sprites["monsters_ufo_0"].createSprite();
	monster.setScale(0.65, 0.65);
	monster.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	monster.setOrigin(78, 34);
	sf::Sprite light = global_sprites["monsters_ufo_1"].createSprite();
	light.setOrigin(80, 0);
	light.setPosition(monster.getPosition());
	light.setScale(monster.getScale());
	Previews::window->draw(light);
	Previews::window->draw(monster);
	return MonsterGeneration::drawPreviewImpl(offset);
}

Monster* BlackHoleGeneration::getMonster()
{
	auto* monster = new BlackHole;
	return monster;
}

float BlackHoleGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite monster = global_sprites["monsters_black_hole"].createSprite();
	monster.setScale(0.65, 0.65);
	monster.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	monster.setOrigin(70, 65);
	Previews::window->draw(monster);
	return MonsterGeneration::drawPreviewImpl(offset);
}

Monster* OvalGreenMonsterGeneration::getMonster()
{
	auto* monster = new OvalGreenMonster;
	return monster;
}

float OvalGreenMonsterGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite monster = global_sprites["monsters_oval_green_0"].createSprite();
	monster.setScale(0.65, 0.65);
	monster.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	monster.setOrigin(62, 168);
	Previews::window->draw(monster);
	return MonsterGeneration::drawPreviewImpl(offset);
}

Monster* FlatGreenMonsterGeneration::getMonster()
{
	auto* monster = new FlatGreenMonster;
	return monster;
}

float FlatGreenMonsterGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite monster = global_sprites["monsters_flat_green_0"].createSprite();
	monster.setScale(0.65, 0.65);
	monster.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	monster.setOrigin(91, 62);
	Previews::window->draw(monster);
	return MonsterGeneration::drawPreviewImpl(offset);
}

Monster* LargeGreenMonsterGeneration::getMonster()
{
	auto* monster = new LargeGreenMonster;
	return monster;
}

float LargeGreenMonsterGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite monster = global_sprites["monsters_large_green_0"].createSprite();
	monster.setScale(0.65, 0.65);
	monster.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	monster.setOrigin(81, 102);
	Previews::window->draw(monster);
	return MonsterGeneration::drawPreviewImpl(offset);
}

Monster* BlueWingedMonsterGeneration::getMonster()
{
	auto* monster = new BlueWingedMonster;
	return monster;
}

float BlueWingedMonsterGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite monster = global_sprites["monsters_blue_winged_0"].createSprite();
	monster.setScale(0.65, 0.65);
	monster.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	monster.setOrigin(78, 44);
	Previews::window->draw(monster);
	return MonsterGeneration::drawPreviewImpl(offset);
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

float TheTerrifyingMonsterGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	sf::Sprite monster = global_sprites["monsters_the_terrifying_0"].createSprite();
	monster.setScale(0.65, 0.65);
	monster.setPosition(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	monster.setOrigin(63, 87);
	Previews::window->draw(monster);
	float mean_height = MonsterGeneration::drawPreviewImpl(offset);
	if(speed_returner) speed_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	if(left_returner) left_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	if(right_returner) right_returner->drawPreview(offset + utils::yFlipped(position_returner ? position_returner->getMeanValue() : sf::Vector2f{}));
	return mean_height;
}



float GenerationWithChance::generateImpl(float generated_height, float left, float right)
{
	if (!generation) return 0.0f;
	if (utils::getTrueWithChance(chance_returner ? chance_returner->getValue() : 0.0f)) return (generation ? generation->generate(generated_height, left, right) : 0.f);
	return 0.0f;

}

void GenerationWithChance::toImGuiImpl()
{
	Generation::toImGuiImpl();
	::toImGui<Generation>(generation, "Generation:");
	::toImGui<Returner<Chance>>(chance_returner, "Chance:");
}

float GenerationWithChance::drawPreviewImpl(sf::Vector2f offset) const
{
	if (generation) return generation->drawPreview(offset);
	return 0.f;
}

float GenerationWithChance::drawSubGenerationsPreview(sf::Vector2f offset) const
{
	if (generation) return generation->drawPreview(offset);
	return 0.f;
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
	if (ImGui::TreeNodeEx(std::format("(size: {})###gens", generations.size()).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (size_t i = 0; i < generations.size(); i++) ::toImGui<Generation>(generations[i], std::format("[{}]:", i + 1));
		if (ImGui::SmallButton("New")) generations.emplace_back();
		if (ImGui::SmallButton("Delete")) ImGui::OpenPopup("For deleting");
		if (ImGui::BeginPopup("For deleting"))
		{
			static size_t ind = 1;
			ImGui::InputScalar("No. to delete", ImGuiDataType_U32, &ind);
			if (ind <= 0) ind = 1;
			if (ind > generations.size()) ind = generations.size();
			if (ImGui::SmallButton("Delete"))
			{ 
				if (generations[ind - 1]) copyGeneration<Generation>(generations[ind - 1], true);
				generations.erase(generations.begin() + ind - 1);
			}
			ImGui::EndPopup();
		}
		ImGui::TreePop();
	}
}

float GroupGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	float max_mean_height = 0.f;
	for (const auto& gen : generations) if (gen) max_mean_height = std::max(max_mean_height, gen->drawPreview(offset));
	return max_mean_height;
}

float GroupGeneration::drawSubGenerationsPreview(sf::Vector2f offset) const
{
	float sum_mean_height = 0.0f;
	for (const auto& gen : generations) if (gen) sum_mean_height += gen->drawPreview(offset - sf::Vector2f{ 0, sum_mean_height });
	return sum_mean_height;
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
	if (ImGui::TreeNodeEx(std::format("(size: {})###gens", generations.size()).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (size_t i = 0; i < generations.size(); i++) ::toImGui<Generation>(generations[i], std::format("[{}]:", i + 1));
		if (ImGui::SmallButton("New")) generations.emplace_back();
		if (ImGui::SmallButton("Delete")) ImGui::OpenPopup("For deleting");
		if (ImGui::BeginPopup("For deleting"))
		{
			static size_t ind = 1;
			ImGui::InputScalar("No. to delete", ImGuiDataType_U32, &ind);
			if (ind <= 0) ind = 1;
			if (ind > generations.size()) ind = generations.size();
			if (ImGui::SmallButton("Delete"))
			{ 
				if (generations[ind - 1]) copyGeneration<Generation>(generations[ind - 1], true);
				generations.erase(generations.begin() + ind - 1);
			}
			ImGui::EndPopup();
		}
		ImGui::TreePop();
	}
}

float ConsecutiveGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	float sum_mean_height = 0.0f;
	for (const auto& gen : generations) if (gen) sum_mean_height += gen->drawPreview(offset - sf::Vector2f{ 0, sum_mean_height });
	return sum_mean_height;
}

float ConsecutiveGeneration::drawSubGenerationsPreview(sf::Vector2f offset) const
{
	float sum_mean_height = 0.0f;
	for (const auto& gen : generations) if (gen) sum_mean_height += gen->drawPreview(offset - sf::Vector2f{ 0, sum_mean_height });
	return sum_mean_height;
}

void PickOneGeneration::ProbabilityGenerationPair::toImGui()
{
	if (ImGui::TreeNodeEx(std::format("Gen. & prob.##{}", (uintptr_t)this).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
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
	if (ImGui::TreeNodeEx(std::format("(size: {})###gens", generations.size()).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (size_t i = 0; i < generations.size(); i++)
		{
			ImGui::Text(std::format("[{}]:", i + 1).c_str()); ImGui::SameLine(); generations[i].toImGui();
		}
		if (ImGui::SmallButton("New")) generations.emplace_back(std::unique_ptr<Returner<RelativeProbability>>(), std::unique_ptr<Generation>());
		if (ImGui::SmallButton("Delete")) ImGui::OpenPopup("For deleting");
		if (ImGui::BeginPopup("For deleting"))
		{
			static size_t ind = 1;
			ImGui::InputScalar("No. to delete", ImGuiDataType_U32, &ind);
			if (ind <= 0) ind = 1;
			if (ind > generations.size()) ind = generations.size();
			if (ImGui::SmallButton("Delete"))
			{ 
				if (generations[ind - 1].generation) copyGeneration<Generation>(generations[ind - 1].generation, true);
				if (generations[ind - 1].relative_probability_returner) copyReturner<Returner<RelativeProbability>>(generations[ind - 1].relative_probability_returner);
				generations.erase(generations.begin() + ind - 1);
			}
			ImGui::EndPopup();
		}
		ImGui::TreePop();
	}
}

float PickOneGeneration::drawPreviewImpl(sf::Vector2f offset) const
{
	return 0.0f;
}

float PickOneGeneration::drawSubGenerationsPreview(sf::Vector2f offset) const
{
	float sum_mean_height = 0.0f;
	for (const auto& pair : generations) if (pair.generation) sum_mean_height += pair.generation->drawPreview(offset - sf::Vector2f{ 0, sum_mean_height });
	return sum_mean_height;
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
	return "Tile";
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

bool TileGeneration::canPreview() const
{
	return false;
}

std::string ItemGeneration::getName() const
{
	return "Item";
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

bool ItemGeneration::canPreview() const
{
	return false;
}

std::string MonsterGeneration::getName() const
{
	return "Monster";
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

bool MonsterGeneration::canPreview() const
{
	return false;
}

std::string NormalTileGeneration::getName() const
{
	return "Normal Tile";
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

bool NormalTileGeneration::canPreview() const
{
	return true;
}

std::string HorizontalSlidingTileGeneration::getName() const
{
	return "Horisontal Sliding Tile";
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

bool HorizontalSlidingTileGeneration::canPreview() const
{
	return true;
}

std::string VerticalSlidingTileGeneration::getName() const
{
	return "Vertical Sliding Tile";
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

bool VerticalSlidingTileGeneration::canPreview() const
{
	return true;
}

std::string DecayedTileGeneration::getName() const
{
	return "Decayed Tile";
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

bool DecayedTileGeneration::canPreview() const
{
	return true;
}

std::string BombTileGeneration::getName() const
{
	return "Bomb Tile";
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

bool BombTileGeneration::canPreview() const
{
	return true;
}

std::string OneTimeTileGeneration::getName() const
{
	return "One Time Tile";
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

bool OneTimeTileGeneration::canPreview() const
{
	return true;
}

std::string TeleportTileGeneration::getName() const
{
	return "Teleport Tile";
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

bool TeleportTileGeneration::canPreview() const
{
	return true;
}

std::string ClusterTileGeneration::getName() const
{
	return "Cluster Tile";
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

bool ClusterTileGeneration::canPreview() const
{
	return true;
}

std::string SpringGeneration::getName() const
{
	return "Spring";
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

bool SpringGeneration::canPreview() const
{
	return true;
}

std::string TrampolineGeneration::getName() const
{
	return "Trampoline";
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

bool TrampolineGeneration::canPreview() const
{
	return true;
}

std::string PropellerHatGeneration::getName() const
{
	return "Propeller Hat";
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

bool PropellerHatGeneration::canPreview() const
{
	return true;
}

std::string JetpackGeneration::getName() const
{
	return "JetPack";
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

bool JetpackGeneration::canPreview() const
{
	return true;
}

std::string SpringShoesGeneration::getName() const
{
	return "Spring Shoes";
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

bool SpringShoesGeneration::canPreview() const
{
	return true;
}

std::string BlueOneEyedMonsterGeneration::getName() const
{
	return "Blue One-Eyed Monster";
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

bool BlueOneEyedMonsterGeneration::canPreview() const
{
	return true;
}

std::string CamronMonsterGeneration::getName() const
{
	return "Camron Monster";
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

bool CamronMonsterGeneration::canPreview() const
{
	return true;
}

std::string PurpleSpiderMonsterGeneration::getName() const
{
	return "Purple Spider Monster";
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

bool PurpleSpiderMonsterGeneration::canPreview() const
{
	return true;
}

std::string LargeBlueMonsterGeneration::getName() const
{
	return "Larget Blue Monster";
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

bool LargeBlueMonsterGeneration::canPreview() const
{
	return true;
}

std::string UFOGeneration::getName() const
{
	return "UFO";
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

bool UFOGeneration::canPreview() const
{
	return true;
}

std::string BlackHoleGeneration::getName() const
{
	return "Black Hole";
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

bool BlackHoleGeneration::canPreview() const
{
	return true;
}

std::string OvalGreenMonsterGeneration::getName() const
{
	return "Oval Green Monster";
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

bool OvalGreenMonsterGeneration::canPreview() const
{
	return true;
}

std::string FlatGreenMonsterGeneration::getName() const
{
	return "Flat Green Monster";
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

bool FlatGreenMonsterGeneration::canPreview() const
{
	return true;
}

std::string LargeGreenMonsterGeneration::getName() const
{
	return "Large Green Monster";
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

bool LargeGreenMonsterGeneration::canPreview() const
{
	return true;
}

std::string BlueWingedMonsterGeneration::getName() const
{
	return "Blue Winged Monster";
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

bool BlueWingedMonsterGeneration::canPreview() const
{
	return true;
}

std::string TheTerrifyingMonsterGeneration::getName() const
{
	return "The Terrifying Monster";
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

bool TheTerrifyingMonsterGeneration::canPreview() const
{
	return true;
}

std::string GenerationWithChance::getName() const
{
	return "Generation With Chance";
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

bool GenerationWithChance::canPreview() const
{
	if (!generation) return false;
	return generation->canPreview();
}

std::string GroupGeneration::getName() const
{
	return "Group Generation";
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

bool GroupGeneration::canPreview() const
{
	bool res = false;
	for (const auto& gen : generations) if (gen) res |= gen->canPreview();
	return res;
}

std::string ConsecutiveGeneration::getName() const
{
	return "Consecutive Generation";
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

bool ConsecutiveGeneration::canPreview() const
{
	bool res = false;
	for (const auto& gen : generations) if (gen) res |= gen->canPreview();
	return res;
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
	return "Pick One Generation";
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

bool PickOneGeneration::canPreview() const
{
	return false;
}

#undef TEXT
