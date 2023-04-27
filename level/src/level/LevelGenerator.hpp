#pragma once
#include <coroutine>
#include <memory>
#include <string>
#include <functional>
#include <deque>

#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include <gameObjects/Tiles.hpp>
#include <gameObjects/Items.hpp>
#include <gameObjects/Monsters.hpp>
#include <common/Utils.hpp>
#include <common/Returners.hpp>


class Level;
class Generation
{
	inline static Level* level = nullptr;
	size_t ImGui_id{ 0 };
	inline static size_t ImGui_id_counter = 0;

protected:
	virtual float generateImpl(float generated_height, float left, float right);
	virtual void toImGuiImpl();

public:
	Generation();
	float generate(float generated_height, float left, float right); // returns the height if the generation
	static void setCurrentLevelForGenerating(Level* level);
	static Level* getCurrentLevelForGenerating();
	
	virtual std::string getName() const;

	virtual void to_json(nl::json& j) const;
	virtual void from_json(const nl::json& j);
	void toImGui();
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

protected:
	float generateImpl(float generated_height, float left, float right) override;
	virtual void toImGuiImpl() override;
	virtual Tile* getTile();
};

class ItemGeneration : public Generation
{
public:
	Tile* tile{};
	std::unique_ptr<Returner<XOffset>> tile_offset_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	float generateImpl(float, float, float) override;
	virtual void toImGuiImpl() override;
	virtual Item* getItem();
};

class MonsterGeneration : public Generation
{
public:
	std::unique_ptr<Returner<Position>> position_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	float generateImpl(float generated_height, float, float) override;
	virtual void toImGuiImpl() override;
	virtual Monster* getMonster();
};



class NormalTileGeneration : public TileGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
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

protected:
	virtual Tile* getTile() override;
	virtual void toImGuiImpl() override;
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

protected:
	virtual Tile* getTile() override;
	virtual void toImGuiImpl() override;
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

protected:
	virtual Tile* getTile() override;
	virtual void toImGuiImpl() override;
};

class BombTileGeneration : public TileGeneration
{
public:
	std::unique_ptr<Returner<Height>> exploding_height_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
	virtual void toImGuiImpl() override;
};

class OneTimeTileGeneration : public TileGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;
	
protected:
	virtual Tile* getTile() override;
};

class TeleportTileGeneration : public TileGeneration
{
public:
	std::deque<std::unique_ptr<Returner<Offset>>> offset_returners{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
	virtual void toImGuiImpl() override;
};

class ClusterTileGeneration : public TileGeneration
{
public:
	std::deque<std::unique_ptr<Returner<Offset>>> offset_returners{};
	ClusterTile::Id id{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
	virtual void toImGuiImpl() override;
};



class SpringGeneration : public ItemGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Item* getItem() override;
};

class TrampolineGeneration: public ItemGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Item* getItem() override;
};

class PropellerHatGeneration : public ItemGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Item* getItem() override;
};

class JetpackGeneration : public ItemGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Item* getItem() override;
};

class SpringShoesGeneration : public ItemGeneration
{
public:
	std::unique_ptr<Returner<SizeTValue>> max_use_count_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

 protected:
	virtual Item* getItem() override;
	virtual void toImGuiImpl() override;
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

protected:
	virtual Monster* getMonster() override;
	virtual void toImGuiImpl() override;

};

class CamronMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class PurpleSpiderMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};
 
class LargeBlueMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class UFOGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class BlackHoleGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class OvalGreenMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class FlatGreenMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class LargeGreenMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class BlueWingedMonsterGeneration : public MonsterGeneration
{
public:
	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
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

protected:
	virtual Monster* getMonster() override;
	virtual void toImGuiImpl() override;

};



class GenerationWithChance : public Generation
{
public:
	std::unique_ptr<Generation> generation{};
	std::unique_ptr<Returner<Chance>> chance_returner{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual float generateImpl(float generated_height, float left, float right) override;
	virtual void toImGuiImpl() override;
};

class GroupGeneration : public Generation
{
public:
	std::deque<std::unique_ptr<Generation>> generations{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual float generateImpl(float generated_height, float left, float right) override;
	virtual void toImGuiImpl() override;
};

class ConsecutiveGeneration : public Generation
{
public:
	std::deque<std::unique_ptr<Generation>> generations{};

	virtual std::string getName() const override;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual float generateImpl(float generated_height, float left, float right) override;
	virtual void toImGuiImpl() override;
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

protected:
	virtual float generateImpl(float generated_height, float left, float right) override;
	virtual void toImGuiImpl() override;
};

template <std::derived_from<Generation> T>
T* getGenerationPointerFromName(const std::string& name)
{
#define RETURN_FOR_TYPE(x) if constexpr (std::derived_from<x, T>) if(name == #x) return new x;
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
void toImGui(std::unique_ptr<T>& generation, const std::string& format)
{
	ImGui::Text(format.c_str()); 
	if (generation)
	{
		if (ImGui::BeginPopupContextItem(std::format("For reseting##{}", (uintptr_t)&generation).c_str()))
		{
			if (ImGui::SmallButton("Reset to None")) generation.reset();
			ImGui::EndPopup();
		}
	}
	else
	{
		if (ImGui::BeginPopupContextItem(std::format("For new generation##{}", (uintptr_t)&generation).c_str()))
		{
			if (ImGui::SmallButton("Create new")) ImGui::OpenPopup("Choose a type");
			if (ImGui::BeginPopup("Choose a type"))
			{
#define IMGUI_BUTTON_FOR_TYPE(x) if constexpr (std::derived_from<x, T>) if(ImGui::SmallButton(#x)) generation = std::unique_ptr<T>(new x);
				IMGUI_BUTTON_FOR_TYPE(Generation);
				IMGUI_BUTTON_FOR_TYPE(TileGeneration);
				IMGUI_BUTTON_FOR_TYPE(ItemGeneration);
				IMGUI_BUTTON_FOR_TYPE(MonsterGeneration);
				IMGUI_BUTTON_FOR_TYPE(NormalTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(HorizontalSlidingTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(VerticalSlidingTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(DecayedTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(BombTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(OneTimeTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(TeleportTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(ClusterTileGeneration);
				IMGUI_BUTTON_FOR_TYPE(SpringGeneration);
				IMGUI_BUTTON_FOR_TYPE(TrampolineGeneration);
				IMGUI_BUTTON_FOR_TYPE(PropellerHatGeneration);
				IMGUI_BUTTON_FOR_TYPE(JetpackGeneration);
				IMGUI_BUTTON_FOR_TYPE(SpringShoesGeneration);
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
				IMGUI_BUTTON_FOR_TYPE(GenerationWithChance);
				IMGUI_BUTTON_FOR_TYPE(GroupGeneration);
				IMGUI_BUTTON_FOR_TYPE(ConsecutiveGeneration);
				IMGUI_BUTTON_FOR_TYPE(PickOneGeneration);

#undef IMGUI_BUTTON_FOR_TYPE
				ImGui::EndPopup();
			}
			ImGui::EndPopup();
		}
	}
	ImGui::SameLine();
	if (generation) generation->toImGui();
	else ImGui::Text("None");
}

