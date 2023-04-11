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
using namespace utils;


class Level;
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
	LevelGenerator(Level* level);
	void update();

private:
	Tiles* m_tiles;
	Items* m_items;
	Monsters* m_monsters;
	sf::RenderWindow* m_window;
	Generator m_generator;
	float m_generated_height{};
	sf::FloatRect m_generating_area{};
	
	sf::FloatRect getGeneratingArea();
	Generator getGenerator();

	struct NormalGenerationStats
	{
		sf::Vector2f tile_minimum_interval{ 100, 20 };
		sf::Vector2f tile_maximum_interval{ 450, 200 };
		float minimum_density_inverse = 7;
		float density_inverse_deviation = 2;
		float minimum_density_height = 20000;
		float getTileDensityInverseFromGeneratedHeight(float generated_height);
		float getCurrentTileDensityInverse(float generated_height, float left, float right);
		
		float horizontal_tile_minimum_height = 3000;
		float all_horizontal_tile_height = 50000;
		float getHorizontalTileChanceFromGeneratedHeight(float generated_height);
		float horizontal_tile_maximum_speed_height = 100000;
		thor::Distribution<float> horizontal_tile_speed_distribution{ thor::Distributions::uniform(200.f, 300.f) };
		float horizontal_tile_minimum_speed_multiplier = 0.5;
		float getHorizontalTileSpeedMultiplierFromGeneratedHeight(float generated_height);

		float decayed_tile_minimum_height = 0;
		float decayed_tile_maximum_chance_height = 20000;
		float decayed_tile_maximum_chance = 0.5;
		float getDecayedTileChanceFromGeneratedHeight(float generated_height);
		float decayed_moving_tile_minimum_height = 3000;
		float decayed_all_moving_tile_height = 50000;
		float getDecayedMovingTileChanceFromGeneratedHeight(float generated_height);
		float decayed_moving_tile_maximum_speed_height = 100000;
		thor::Distribution<float> decayed_moving_tile_speed_distribution{ thor::Distributions::uniform(200.f, 300.f) };
		float decayed_moving_tile_minimum_speed_multiplier = 0.5;
		float getDecayedMovingTileSpeedMultiplierFromGeneratedHeight(float generated_height);

		float spring_maximum_chance = 0.05;
		float spring_minimum_chance = 0.04;
		float spring_minimum_chance_height = 20000;
		float getSpringChanceFromGeneratedHeight(float generated_height);

		float spring_shoes_chance = 0.001;
		thor::Distribution<int> spring_shoes_max_use_count_distribution{ thor::Distributions::uniform(4, 6) };

		float trampoline_maximum_chance = 0.04;
		float trampoline_minimum_chance = 0.00001;
		float trampoline_minimum_chance_height = 20000;
		float getTrampolineChanceFromGeneratedHeight(float generated_height);
		
		float propeller_hat_maximum_chance = 0.01;
		float propeller_hat_minimum_chance = 0.01;
		float propeller_hat_minimum_chance_height = 50000;
		float getPropellerHatChanceFromGeneratedHeight(float generated_height);

		float jetpack_maximum_chance = 0.005;
		float jetpack_minimum_chance = 0.005;
		float jetpack_minimum_chance_height = 50000;
		float getJetpackChanceFromGeneratedHeight(float generated_height);

		

	} m_gen_stats;

	struct CustonGenerationStats
	{
		std::string name;
		float start_chance;
		float end_chance;
		float start_height;
		float end_height;
		std::function<void(float, float, float)> generator;
		float getChanceFromGeneratedHeight(float generated_height);
	};

	std::deque<CustonGenerationStats> m_custom_gen_stats
	{
		{
			.name{"normal monster"},
			.start_chance = 0.01,
			.end_chance = 0.02,
			.start_height = 5000,
			.end_height = 30000,
			.generator{[](float generated_height, float left, float right)
			{
				size_t type = thor::Distributions::uniform(1, 4)();
				switch (type)
				{
				case 1:
					break;
				}
			}}
		}
	};


	Tile* generateNormalTiles(float generated_height, float left, float right, float& tile_density_inv);
	Tile* generateHorizontalTile(float generated_height, float left, float right, float& tile_density_inv);
	void generateDecayedTile(float generated_height, float left, float right, float tile_density_inv);
	void generateSpringOnTile(Tile* tile);
	void generateSpringShoesOnTile(Tile* tile);
	void generateTrampolineOnTile(Tile* tile);
	void generatePropellerHatOnTile(Tile* tile);
	void generateJetpackOnTile(Tile* tile);
	
	
	void generateNormalGeneration(float& generated_height, float left, float right);
};


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

class LevelGeneratorNew
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
	LevelGeneratorNew();
	void update();
	template <std::derived_from<Generation> T>
	void setGeneration(T&& gen)
	{
		m_generation = std::make_unique<T>(std::forward<T>(gen));
	}
	void setGenerationSettings(GenerationSettings settings);

	static void setLevelForGeneration(Level* level_ptr);
	static Level* getLevelForGeneration();

private:
	sf::FloatRect getGeneratingArea();
	Generator getGenerator();

	friend void to_json(nl::json& j, const LevelGeneratorNew& level_generator);
	friend void from_json(const nl::json& j, LevelGeneratorNew& level_generator);
};

class ItemGeneration;
class TileGeneration : public Generation
{
public:
	Returner<sf::Vector2f> position_returner = def_returner<sf::Vector2f>;
	Returner<float> height_returner = def_returner<float>;
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
	Returner<float> tile_offset_returner = def_returner<float>;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	float generateImpl(float, float, float) override;
	virtual Item* getItem();
};

class MonsterGeneration : public Generation
{
public:
	Returner<sf::Vector2f> position_returner = def_returner<sf::Vector2f>;

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
	Returner<float> speed_returner = def_returner<float>;
	Returner<float> left_returner = def_returner<float>;
	Returner<float> right_returner = def_returner<float>;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
};

class VerticalSlidingTileGeneration : public TileGeneration
{
public:
	Returner<float> speed_returner = def_returner<float>;
	Returner<float> top_returner = def_returner<float>;
	Returner<float> bottom_returner = def_returner<float>;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
};

class DecayedTileGeneration : public TileGeneration
{
public:
	Returner<float> speed_returner = def_returner<float>;
	Returner<float> left_returner = def_returner<float>;
	Returner<float> right_returner = def_returner<float>;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
};

class BombTileGeneration : public TileGeneration
{
public:
	Returner<float> exploding_height_returner = def_returner<float>;

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
	std::deque<Returner<sf::Vector2f>> offset_returners{};

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Tile* getTile() override;
};

class ClusterTileGeneration : public TileGeneration
{
public:
	std::deque<Returner<sf::Vector2f>> offset_returners{};
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
	Returner<size_t> max_use_count_returner = def_returner<size_t>;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

 protected:
	virtual Item* getItem() override;
};



class BlueOneEyedMonsterGeneration : public MonsterGeneration
{
public:
	Returner<float> speed_returner = def_returner<float>;
	Returner<float> left_returner = def_returner<float>;
	Returner<float> right_returner = def_returner<float>;

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
	Returner<sf::Vector2f> speed_returner = def_returner<sf::Vector2f>;
	Returner<float> left_returner = def_returner<float>;
	Returner<float> right_returner = def_returner<float>;

	virtual void to_json(nl::json& j) const override;
	virtual void from_json(const nl::json& j) override;

protected:
	virtual Monster* getMonster() override;

};



class GenerationWithChance : public Generation
{
public:
	std::unique_ptr<Generation> generation{};
	Returner<float> chance_returner = def_returner<float>;

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
		Returner<float> relative_probability_returner = def_returner<float>;
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
T* getGenerationPointerFromName(std::string name)
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

void to_json(nl::json& j, std::derived_from<Generation> auto* generation)
{
	if (generation) generation->to_json(j);
	else j = nullptr;
}

template<std::derived_from<Generation> T>
void from_json(const nl::json& j, T*& generation)
{
	if (j.is_null())
	{
		generation = nullptr;
		return;
	}
#define TEXT(x) #x
	generation = getGenerationPointerFromName<T>(j.contains("name") ? j["name"] : TEXT(Generation));
#undef TEXT
	generation->from_json(j);
}