#pragma once
#include <coroutine>
#include <memory>
#include <string>
#include <functional>
#include <deque>
#include <utility>
#include <typeinfo>
#include <fstream>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>

#include <gameObjects/Tiles.hpp>
#include <gameObjects/Items.hpp>
#include <gameObjects/Monsters.hpp>
#include <common/Utils.hpp>
#include <common/Returners.hpp>
#include <DoodleJumpConfig.hpp>


class Level;
class Generation
{
	inline static Level* level = nullptr;
	size_t ImGui_id{ 0 };
	inline static size_t ImGui_id_counter = 0;

protected:
	virtual float generateImpl(float generated_height, float left, float right);
	virtual void toImGuiImpl();

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
	virtual float drawSubGenerationsPreview(sf::Vector2f offset) const;

public:
	Generation();
	float generate(float generated_height, float left, float right); // returns the height of the generation
	static void setCurrentLevelForGenerating(Level* level);
	static Level* getCurrentLevelForGenerating();
	
	virtual std::string getName() const;

	virtual void to_json(nl::json& j) const;
	virtual void from_json(const nl::json& j);
	void toImGui();

	bool preview = true;
	virtual bool canPreview() const;
	
	float drawPreview(sf::Vector2f offset = sf::Vector2f{}) const;
};

class LevelGenerator
{
	struct Generator
	{
		struct promise_type;
		using handle_type = std::coroutine_handle<promise_type>;

		handle_type coro;
		Generator(handle_type h) : coro(h) {}

		~Generator();
		Generator(Generator&& oth) noexcept;
		Generator& operator=(Generator&& oth) noexcept;

		bool resume();

		struct promise_type
		{
			promise_type() = default;
			promise_type(promise_type&& oth) = default;
			promise_type& operator=(promise_type&& oth) = default;
			~promise_type() = default;

			std::suspend_always initial_suspend() { return{}; }
			std::suspend_always final_suspend() noexcept { return{}; }
			Generator get_return_object();
			void return_void() {};
			void unhandled_exception() { throw std::current_exception(); }

		};

	};

public:
	struct GenerationSettings
	{
		int repeate_count = 1; // -1 for infinite
		void toImGui();
		friend void to_json(nl::json& j, const GenerationSettings& generation_settings);
		friend void from_json(const nl::json& j, GenerationSettings& generation_settings);
	};

private:
	GenerationSettings m_settings{};
	Generator m_generator;
	std::unique_ptr<Generation> m_generation{};
	float m_generated_height{1000};
	sf::FloatRect m_generating_area{};

public:
	LevelGenerator();
	void reset();
	void update();
	template <std::derived_from<Generation> T>
	void setGeneration(T&& gen)
	{
		m_generation = std::make_unique<T>(std::forward<T>(gen));
	}
	const std::unique_ptr<Generation>& getGeneration() const;
	void setGenerationSettings(GenerationSettings settings);
	float getGeneratedHeight();

	void toImGui();

	static void setLevelForGeneration(Level* level_ptr);
	static Level* getLevelForGeneration();

private:
	sf::FloatRect getGeneratingArea();
	Generator getGenerator();

	friend void to_json(nl::json& j, const LevelGenerator& level_generator);
	friend void from_json(const nl::json& j, LevelGenerator& level_generator);
};

class ItemGeneration;
class TileGeneration : public Generation
{
public:
	std::unique_ptr<Returner<Position>> position_returner{};
	std::unique_ptr<Returner<Height>> height_returner{};
	std::unique_ptr<ItemGeneration> item_generation{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;
	
	virtual bool canPreview() const override;

protected:
	float generateImpl(float generated_height, float left, float right) override;
	virtual void toImGuiImpl() override;
	virtual Tile* getTile();

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class ItemGeneration : public Generation
{
public:
	Tile* tile{};
	std::unique_ptr<Returner<XOffset>> tile_offset_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	float generateImpl(float, float, float) override;
	virtual void toImGuiImpl() override;
	virtual Item* getItem();

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class MonsterGeneration : public Generation
{
public:
	std::unique_ptr<Returner<Position>> position_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	float generateImpl(float generated_height, float, float) override;
	virtual void toImGuiImpl() override;
	virtual Monster* getMonster();

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};



class NormalTileGeneration : public TileGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Tile* getTile() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class HorizontalSlidingTileGeneration : public TileGeneration
{
public:
	std::unique_ptr<Returner<XSpeed>> speed_returner{};
	std::unique_ptr<Returner<XBoundary>> left_returner{};
	std::unique_ptr<Returner<XBoundary>> right_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Tile* getTile() override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class VerticalSlidingTileGeneration : public TileGeneration
{
public:
	std::unique_ptr<Returner<YSpeed>> speed_returner{};
	std::unique_ptr<Returner<YBoundary>> top_returner{};
	std::unique_ptr<Returner<YBoundary>> bottom_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Tile* getTile() override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class DecayedTileGeneration : public TileGeneration
{
public:
	std::unique_ptr<Returner<XSpeed>> speed_returner{};
	std::unique_ptr<Returner<XBoundary>> left_returner{};
	std::unique_ptr<Returner<XBoundary>> right_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Tile* getTile() override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class BombTileGeneration : public TileGeneration
{
public:
	std::unique_ptr<Returner<Height>> exploding_height_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Tile* getTile() override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class OneTimeTileGeneration : public TileGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;
	
	virtual bool canPreview() const override;

protected:
	virtual Tile* getTile() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class TeleportTileGeneration : public TileGeneration
{
public:
	std::deque<std::unique_ptr<Returner<Position>>> offset_returners{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Tile* getTile() override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class ClusterTileGeneration : public TileGeneration
{
public:
	std::deque<std::unique_ptr<Returner<Position>>> offset_returners{};
	ClusterTile::Id id{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Tile* getTile() override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};



class SpringGeneration : public ItemGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Item* getItem() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class TrampolineGeneration: public ItemGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Item* getItem() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class PropellerHatGeneration : public ItemGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Item* getItem() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class JetpackGeneration : public ItemGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Item* getItem() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class SpringShoesGeneration : public ItemGeneration
{
public:
	std::unique_ptr<Returner<SizeTValue>> max_use_count_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

 protected:
	virtual Item* getItem() override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};



class BlueOneEyedMonsterGeneration : public MonsterGeneration
{
public:
	std::unique_ptr<Returner<XSpeed>> speed_returner{};
	std::unique_ptr<Returner<XBoundary>> left_returner{};
	std::unique_ptr<Returner<XBoundary>> right_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Monster* getMonster() override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class CamronMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Monster* getMonster() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class PurpleSpiderMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Monster* getMonster() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};
 
class LargeBlueMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Monster* getMonster() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class UFOGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Monster* getMonster() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class BlackHoleGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Monster* getMonster() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class OvalGreenMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Monster* getMonster() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class FlatGreenMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Monster* getMonster() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class LargeGreenMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Monster* getMonster() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class BlueWingedMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Monster* getMonster() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};

class TheTerrifyingMonsterGeneration : public MonsterGeneration
{
public:
	std::unique_ptr<Returner<Speed>> speed_returner{};
	std::unique_ptr<Returner<XBoundary>> left_returner{};
	std::unique_ptr<Returner<XBoundary>> right_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual Monster* getMonster() override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
};



class GenerationWithChance : public Generation
{
public:
	std::unique_ptr<Generation> generation{};
	std::unique_ptr<Returner<Chance>> chance_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual float generateImpl(float generated_height, float left, float right) override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
	
	virtual float drawSubGenerationsPreview(sf::Vector2f offset) const;
};

class GroupGeneration : public Generation
{
public:
	std::deque<std::unique_ptr<Generation>> generations{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual float generateImpl(float generated_height, float left, float right) override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
	
	virtual float drawSubGenerationsPreview(sf::Vector2f offset) const;
};

class ConsecutiveGeneration : public Generation
{
public:
	std::deque<std::unique_ptr<Generation>> generations{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual float generateImpl(float generated_height, float left, float right) override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
	
	virtual float drawSubGenerationsPreview(sf::Vector2f offset) const;
};

class PickOneGeneration : public Generation
{
public:
	struct ProbabilityGenerationPair
	{
		std::unique_ptr<Returner<RelativeProbability>> relative_probability_returner{};
		std::unique_ptr<Generation> generation{};
		void toImGui();
		friend void to_json(nl::json& j, const ProbabilityGenerationPair& pair);
		friend void from_json(const nl::json& j, ProbabilityGenerationPair& pair);
	};
	std::deque<ProbabilityGenerationPair> generations{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

	virtual bool canPreview() const override;

protected:
	virtual float generateImpl(float generated_height, float left, float right) override;
	virtual void toImGuiImpl() override;

	virtual float drawPreviewImpl(sf::Vector2f offset) const;
	
	virtual float drawSubGenerationsPreview(sf::Vector2f offset) const;
};

template <std::derived_from<Generation> T>
T* getGenerationPointerFromName(const std::string& name)
{
#define RETURN_FOR_TYPE(x) if constexpr (std::derived_from<x, T>) if(name == x().getName()) return new x;
	RETURN_FOR_TYPE(Generation);
	RETURN_FOR_TYPE(TileGeneration);
	RETURN_FOR_TYPE(ItemGeneration);
	RETURN_FOR_TYPE(MonsterGeneration);
	RETURN_FOR_TYPE(NormalTileGeneration);
	RETURN_FOR_TYPE(HorizontalSlidingTileGeneration);
	RETURN_FOR_TYPE(VerticalSlidingTileGeneration);
	RETURN_FOR_TYPE(DecayedTileGeneration);
	RETURN_FOR_TYPE(BombTileGeneration);
	RETURN_FOR_TYPE(OneTimeTileGeneration);
	RETURN_FOR_TYPE(TeleportTileGeneration);
	RETURN_FOR_TYPE(ClusterTileGeneration);
	RETURN_FOR_TYPE(SpringGeneration);
	RETURN_FOR_TYPE(TrampolineGeneration);
	RETURN_FOR_TYPE(PropellerHatGeneration);
	RETURN_FOR_TYPE(JetpackGeneration);
	RETURN_FOR_TYPE(SpringShoesGeneration);
	RETURN_FOR_TYPE(BlueOneEyedMonsterGeneration);
	RETURN_FOR_TYPE(CamronMonsterGeneration);
	RETURN_FOR_TYPE(PurpleSpiderMonsterGeneration);
	RETURN_FOR_TYPE(LargeBlueMonsterGeneration);
	RETURN_FOR_TYPE(UFOGeneration);
	RETURN_FOR_TYPE(BlackHoleGeneration);
	RETURN_FOR_TYPE(OvalGreenMonsterGeneration);
	RETURN_FOR_TYPE(FlatGreenMonsterGeneration);
	RETURN_FOR_TYPE(LargeGreenMonsterGeneration);
	RETURN_FOR_TYPE(BlueWingedMonsterGeneration);
	RETURN_FOR_TYPE(TheTerrifyingMonsterGeneration);
	RETURN_FOR_TYPE(GenerationWithChance);
	RETURN_FOR_TYPE(GroupGeneration);
	RETURN_FOR_TYPE(ConsecutiveGeneration);
	RETURN_FOR_TYPE(PickOneGeneration);

#undef RETURN_FOR_TYPE
	return nullptr;
}

template <std::derived_from<Generation> T>
T* getGenerationPointerFromJson(const nl::json& j)
{
	if (!j.contains("name")) return nullptr;
	return getGenerationPointerFromName<T>(j["name"].get<std::string>());
}

namespace nlohmann
{
	template<std::derived_from<Generation> T>
	struct adl_serializer<std::unique_ptr<T>>
	{
		static void to_json(nl::json& j, const std::unique_ptr<T>& ptr) 
		{
			if (ptr) ptr->to_json(j);
			else j = nullptr;
		};

		static void from_json(const nl::json& j, std::unique_ptr<T>& ptr) 
		{
			if (j.is_null())
			{
				ptr = std::unique_ptr<T>{};
				return;
			}
			ptr = std::unique_ptr<T>(getGenerationPointerFromJson<T>(j));
			ptr->from_json(j);
		};
	};
}

template <std::derived_from<Generation> T>
void copyGeneration(const std::unique_ptr<T>& generation, bool deleted = false)
{
	nl::json j;
	generation->to_json(j);
	if (!deleted) doodle_jump_clipboard.recent_copies.emplace_front(typeid(Generation), j.dump());
	else doodle_jump_clipboard.recent_deletions.emplace_front(typeid(Generation), j.dump());
}

template <std::derived_from<Generation> T>
void pasteGeneration(std::unique_ptr<T>& generation, size_t ind, bool from_deletions = false)
{
	nl::json j = nl::json::parse((from_deletions ? doodle_jump_clipboard.recent_deletions : doodle_jump_clipboard.recent_copies)[ind].second);
	generation = std::unique_ptr<T>(getGenerationPointerFromJson<T>(j));
	if (generation) generation->from_json(j);
	else ImGui::OpenPopup("Paste failed");
}

template <std::derived_from<Generation> T>
void pasteGenerationImGuiButton(std::unique_ptr<T>& generation)
{
	if (!doodle_jump_clipboard.empty() && ImGui::SmallButton("Paste")) ImGui::OpenPopup("For pasting");
	if (ImGui::BeginPopup("For pasting"))
	{
		static size_t ind = 1;
		ImGui::InputScalar("No. to paste", ImGuiDataType_U32, &ind);
		if (ind <= 0) ind = 1;
		if (ind > std::max(doodle_jump_clipboard.recent_copies.size(), doodle_jump_clipboard.recent_deletions.size())) ind = std::max(doodle_jump_clipboard.recent_copies.size(), doodle_jump_clipboard.recent_deletions.size());
		if (ind <= doodle_jump_clipboard.recent_copies.size() && ImGui::SmallButton("Paste from recent copies")) pasteGeneration<T>(generation, ind - 1);
		if (ind <= doodle_jump_clipboard.recent_deletions.size() && ImGui::SmallButton("Paste from recent deletions")) pasteGeneration<T>(generation, ind - 1, true);
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopup("Paste failed"))
	{
		ImGui::Text("Cannot paste here");
		ImGui::EndPopup();
	}
}

template <std::derived_from<Generation> T>
void loadGenerationFromFileImGuiButton(std::unique_ptr<T>& generation)
{
	if (ImGui::SmallButton("Load from file")) ImGui::OpenPopup("For loading from file");
	if (ImGui::BeginPopup("For loading from file"))
	{
		static std::string file_name, name;
		ImGui::InputText(".json  File name", &file_name);
		ImGui::InputText("Generation name", &name);
		if (ImGui::SmallButton("Load"))
		{
			std::ifstream fin(RESOURCES_PATH "Saved generations and returners/" + file_name + ".json");
			if (fin)
			{
				nl::json j;
				try
				{
					fin >> j;
				}
				catch (...) {}
				if (j.contains(name))
				{
					T* new_gen = getGenerationPointerFromJson<T>(j[name]);
					if (new_gen)
					{
						generation = std::unique_ptr<T>(new_gen);
						generation->from_json(j[name]);
					}
					else ImGui::OpenPopup("Load failed, no generation");
				}
				else ImGui::OpenPopup("Load failed, no name");
			}
			else ImGui::OpenPopup("Load failed, no file");
		}
		
		if (ImGui::BeginPopup("Load failed, no generation"))
		{
			ImGui::Text("Can't load the specified generation here");
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("Load failed, no name"))
		{
			ImGui::Text("A generation with that name does not exist in that file");
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("Load failed, no file"))
		{
			ImGui::Text("File does not exist");
			ImGui::EndPopup();
		}

		ImGui::EndPopup();
	}
}

template <std::derived_from<Generation> T>
void saveGenerationToFileImGuiButton(const std::unique_ptr<T>& generation)
{
	if (ImGui::SmallButton("Save to file")) ImGui::OpenPopup("For saving to file");
	if (ImGui::BeginPopup("For saving to file"))
	{
		static std::string file_name, name;
		ImGui::InputText(".json  File name", &file_name);
		ImGui::InputText("Generation name", &name);
		if (ImGui::SmallButton("Save"))
		{
			nl::json j;
			std::fstream file(RESOURCES_PATH "Saved generations and returners/" + file_name + ".json", std::ios_base::in | std::ios_base::out | std::ios_base::app);
			try
			{
				file >> j;
			}
			catch (...) {}
			file.close();
			generation->to_json(j[name]);
			file.open(RESOURCES_PATH "Saved generations and returners/" + file_name + ".json", std::ios_base::out | std::ios_base::trunc);
			file << j;
		}
		ImGui::EndPopup();
	}
}

template <std::derived_from<Generation> T>
void toImGui(std::unique_ptr<T>& generation, const std::string& format)
{
	ImGui::Text(format.c_str()); 
	if (generation)
	{
		if (ImGui::BeginPopupContextItem(std::format("For reseting##{}", (uintptr_t)&generation).c_str()))
		{
			if (ImGui::SmallButton("Reset to None"))
			{
				copyGeneration<T>(generation, true);
				generation.reset();
			}
			if (ImGui::SmallButton("Copy")) copyGeneration<T>(generation);
			pasteGenerationImGuiButton<T>(generation);
			loadGenerationFromFileImGuiButton<T>(generation);
			saveGenerationToFileImGuiButton<T>(generation);
			ImGui::EndPopup();
		}
	}
	else
	{
		if (ImGui::BeginPopupContextItem(std::format("For new generation##{}", (uintptr_t)&generation).c_str()))
		{
#define IMGUI_BUTTON_FOR_TYPE(x) if constexpr (std::derived_from<x, T>) if(ImGui::SmallButton(x().getName().c_str())) generation = std::unique_ptr<T>(new x);
			if (ImGui::TreeNodeEx("Tiles", ImGuiTreeNodeFlags_DefaultOpen))
			{
				IMGUI_BUTTON_FOR_TYPE(NormalTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(HorizontalSlidingTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(VerticalSlidingTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(DecayedTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(BombTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(OneTimeTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(TeleportTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(ClusterTileGeneration);
				ImGui::TreePop();
			}

			if (ImGui::TreeNodeEx("Items", ImGuiTreeNodeFlags_DefaultOpen))
			{
				IMGUI_BUTTON_FOR_TYPE(SpringGeneration);
				IMGUI_BUTTON_FOR_TYPE(TrampolineGeneration);
				IMGUI_BUTTON_FOR_TYPE(PropellerHatGeneration);
				IMGUI_BUTTON_FOR_TYPE(JetpackGeneration);
				IMGUI_BUTTON_FOR_TYPE(SpringShoesGeneration);
				ImGui::TreePop();
			}

			if (ImGui::TreeNodeEx("Monsters", ImGuiTreeNodeFlags_DefaultOpen))
			{
				IMGUI_BUTTON_FOR_TYPE(BlueOneEyedMonsterGeneration);
				IMGUI_BUTTON_FOR_TYPE(CamronMonsterGeneration);
				IMGUI_BUTTON_FOR_TYPE(PurpleSpiderMonsterGeneration);
				IMGUI_BUTTON_FOR_TYPE(LargeBlueMonsterGeneration);
				IMGUI_BUTTON_FOR_TYPE(UFOGeneration);
				IMGUI_BUTTON_FOR_TYPE(BlackHoleGeneration);
				IMGUI_BUTTON_FOR_TYPE(OvalGreenMonsterGeneration);
				IMGUI_BUTTON_FOR_TYPE(FlatGreenMonsterGeneration);
				IMGUI_BUTTON_FOR_TYPE(LargeGreenMonsterGeneration);
				IMGUI_BUTTON_FOR_TYPE(BlueWingedMonsterGeneration);
				IMGUI_BUTTON_FOR_TYPE(TheTerrifyingMonsterGeneration);
				ImGui::TreePop();
			}

			IMGUI_BUTTON_FOR_TYPE(GenerationWithChance);
			IMGUI_BUTTON_FOR_TYPE(GroupGeneration);
			IMGUI_BUTTON_FOR_TYPE(ConsecutiveGeneration);
			IMGUI_BUTTON_FOR_TYPE(PickOneGeneration);

#undef IMGUI_BUTTON_FOR_TYPE
			ImGui::Separator();
			pasteGenerationImGuiButton<T>(generation);
			loadGenerationFromFileImGuiButton<T>(generation);
			ImGui::EndPopup();
		}
	}
	ImGui::SameLine();
	if (generation) generation->toImGui();
	else ImGui::Text("None");
}

