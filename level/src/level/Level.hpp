#pragma once
#include <SFML/Graphics.hpp>
#include <Thor/Input.hpp>

#include <common/GameStuff.hpp>
#include <drawables/ImageBackground.hpp>
#include <drawables/Scene.hpp>
#include <gameObjects/Doodle.hpp>
#include <gameObjects/Tiles.hpp>
#include <gameObjects/Items.hpp>
#include <gameObjects/Monsters.hpp>
#include <level/LevelGenerator.hpp>

struct Level
{
	sf::RenderWindow& window;

	ImageBackground ib;

	Doodle doodle;
	Tiles tiles;
	Items items;
	Monsters monsters;

	Scene scene;

	LevelGeneratorNew level_generator{};

	Level(sf::Vector2f doodle_pos, sf::RenderWindow& window);
	void handleGameEvents(thor::ActionMap<UserActions>& action_map, sf::Time dt);
	void update(sf::Time dt);
	void addTile(Tile* tile);
	void addItem(Item* item);
	void addMonster(Monster* monster);
};