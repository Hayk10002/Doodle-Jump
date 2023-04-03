#pragma once
#include <coroutine>
#include <memory>
#include <string>
#include <functional>
#include <deque>

#include <SFML/Graphics.hpp>
#include <gameObjects/Tiles.hpp>
#include <gameObjects/Items.hpp>
#include <gameObjects/Monsters.hpp>

float getYFrom5Nums(float x1, float y1, float x2, float y2, float x);
float getYFrom5NumsClamped(float x1, float y1, float x2, float y2, float x);
float getYFrom5NumsLeftClamped(float x1, float y1, float x2, float y2, float x);
float getYFrom5NumsRightClamped(float x1, float y1, float x2, float y2, float x);

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
	LevelGenerator(sf::RenderWindow* window, Tiles* tiles, Items* items, Monsters* monsters);
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
		inline static thor::Distribution<float> from_0_to_1 = thor::Distributions::uniform(0.f, 1.f);
		static bool getTrueWithChance(float chance);
		static float randomSignWithChance(float positive_chance);

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
	Tile* generateNormalTile(float& generated_height, float left, float right);


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

class LevelGeneratorNew
{

};

class Generation
{

};