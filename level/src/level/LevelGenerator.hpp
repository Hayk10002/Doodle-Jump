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

protected:
	virtual float generateImpl(float generated_height, float left, float right);

public:
	float generate(float generated_height, float left, float right); // returns the height if the generation
	static void setCurrentLevelForGenerating(Level* level);
	static Level* getCurrentLevelForGenerating();

	virtual void to_json(nl::json& j) const;
	virtual void from_json(const nl::json& j);
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
	void setGenerationSettings(GenerationSettings settings);
	float getGeneratedHeight();

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
	std::unique_ptr<Returner<Position>> position_returner{ new Returner<Position>{} };
	std::unique_ptr<Returner<Height>> height_returner{new Returner<Height>{}};
	std::unique_ptr<ItemGeneration> item_generation{};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	float generateImpl(float generated_height, float left, float right) override;
	virtual Tile* getTile();
};

class ItemGeneration : public Generation
{
public:
	Tile* tile{};
	std::unique_ptr<Returner<XOffset>> tile_offset_returner{new Returner<XOffset>{}};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	float generateImpl(float, float, float) override;
	virtual Item* getItem();
};

class MonsterGeneration : public Generation
{
public:
	std::unique_ptr<Returner<Position>> position_returner{new Returner<Position>{}};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	float generateImpl(float generated_height, float, float) override;
	virtual Monster* getMonster();
};



class NormalTileGeneration : public TileGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
};

class HorizontalSlidingTileGeneration : public TileGeneration
{
public:
	std::unique_ptr<Returner<XSpeed>> speed_returner{new Returner<XSpeed>{}};
	std::unique_ptr<Returner<XBoundary>> left_returner{new Returner<XBoundary>{}};
	std::unique_ptr<Returner<XBoundary>> right_returner{new Returner<XBoundary>{}};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
};

class VerticalSlidingTileGeneration : public TileGeneration
{
public:
	std::unique_ptr<Returner<YSpeed>> speed_returner{new Returner<YSpeed>{}};
	std::unique_ptr<Returner<YBoundary>> top_returner{new Returner<YBoundary>{}};
	std::unique_ptr<Returner<YBoundary>> bottom_returner{new Returner<YBoundary>{}};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
};

class DecayedTileGeneration : public TileGeneration
{
public:
	std::unique_ptr<Returner<XSpeed>> speed_returner{new Returner<XSpeed>{}};
	std::unique_ptr<Returner<XBoundary>> left_returner{new Returner<XBoundary>{}};
	std::unique_ptr<Returner<XBoundary>> right_returner{new Returner<XBoundary>{}};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
};

class BombTileGeneration : public TileGeneration
{
public:
	std::unique_ptr<Returner<Height>> exploding_height_returner{new Returner<Height>{}};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
};

class OneTimeTileGeneration : public TileGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;
	
protected:
	virtual Tile* getTile() override;
};

class TeleportTileGeneration : public TileGeneration
{
public:
	std::deque<std::unique_ptr<Returner<Offset>>> offset_returners{};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
};

class ClusterTileGeneration : public TileGeneration
{
public:
	std::deque<std::unique_ptr<Returner<Offset>>> offset_returners{};
	ClusterTile::Id id{};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
};



class SpringGeneration : public ItemGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Item* getItem() override;
};

class TrampolineGeneration: public ItemGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Item* getItem() override;
};

class PropellerHatGeneration : public ItemGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Item* getItem() override;
};

class JetpackGeneration : public ItemGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Item* getItem() override;
};

class SpringShoesGeneration : public ItemGeneration
{
public:
	std::unique_ptr<Returner<Count>> max_use_count_returner{new Returner<Count>{}};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

 protected:
	virtual Item* getItem() override;
};



class BlueOneEyedMonsterGeneration : public MonsterGeneration
{
public:
	std::unique_ptr<Returner<XSpeed>> speed_returner{new Returner<XSpeed>{}};
	std::unique_ptr<Returner<XBoundary>> left_returner{new Returner<XBoundary>{}};
	std::unique_ptr<Returner<XBoundary>> right_returner{new Returner<XBoundary>{}};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;

};

class CamronMonsterGeneration : public MonsterGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class PurpleSpiderMonsterGeneration : public MonsterGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};
 
class LargeBlueMonsterGeneration : public MonsterGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class UFOGeneration : public MonsterGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class BlackHoleGeneration : public MonsterGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class OvalGreenMonsterGeneration : public MonsterGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class FlatGreenMonsterGeneration : public MonsterGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class LargeGreenMonsterGeneration : public MonsterGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class BlueWingedMonsterGeneration : public MonsterGeneration
{
public:
	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;
};

class TheTerrifyingMonsterGeneration : public MonsterGeneration
{
public:
	std::unique_ptr<Returner<Speed>> speed_returner{new Returner<Speed>{}};
	std::unique_ptr<Returner<XBoundary>> left_returner{new Returner<XBoundary>{}};
	std::unique_ptr<Returner<XBoundary>> right_returner{new Returner<XBoundary>{}};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;

};



class GenerationWithChance : public Generation
{
public:
	std::unique_ptr<Generation> generation{};
	std::unique_ptr<Returner<Chance>> chance_returner{new Returner<Chance>{}};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual float generateImpl(float generated_height, float left, float right) override;
};

class GroupGeneration : public Generation
{
public:
	std::deque<std::unique_ptr<Generation>> generations{};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual float generateImpl(float generated_height, float left, float right) override;
};

class ConsecutiveGeneration : public Generation
{
public:
	std::deque<std::unique_ptr<Generation>> generations{};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual float generateImpl(float generated_height, float left, float right) override;
};

class PickOneGeneration : public Generation
{
public:
	struct ProbabilityGenerationPair
	{
		std::unique_ptr<Returner<RelativeProbability>> relative_probability_returner{new Returner<RelativeProbability>{}};
		std::unique_ptr<Generation> generation{};
		friend void to_json(nl::json& j, const ProbabilityGenerationPair& pair);
		friend void from_json(const nl::json& j, ProbabilityGenerationPair& pair);
	};
	std::deque<ProbabilityGenerationPair> generations{};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual float generateImpl(float generated_height, float left, float right) override;
};

template <std::derived_from<Generation> T>
T* getGenerationPointerFromJson(const nl::json& j)
{
#define TEXT(x) #x
	std::string name = (j.contains("name") ? j["name"] : TEXT(Generation));
#undef TEXT
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
#define TEXT(x) #x
			ptr = std::unique_ptr<T>(getGenerationPointerFromJson<T>(j));
#undef TEXT
			ptr->from_json(j);
		};
	};
}