#pragma once
#include <coroutine>
#include <memory>

#include <SFML/Graphics.hpp>
#include <gameObjects/Tiles.hpp>
#include <gameObjects/Items.hpp>
#include <gameObjects/Monsters.hpp>

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
	Tile* generateNormalTile(float& generated_height, float left, float right);
};