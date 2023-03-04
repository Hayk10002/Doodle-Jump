#include "Resources.hpp"

#include <DoodleJumpConfig.hpp>

thor::ResourceHolder <sf::Texture, std::string> global_textures;

void init_resources()
{
	global_textures.acquire("background", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"background.png"));
	global_textures["background"].setRepeated(true);
	global_textures["background"].setSmooth(true);

	global_textures.acquire("empty_white_texture", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"empty_white_square.png"));
	global_textures["empty_white_texture"].setRepeated(true);

	global_textures.acquire("doodle", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"doodle_basic.png"));
	global_textures.acquire("tiles", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"tiles.png"));
	global_textures.acquire("items", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"items.png"));
	global_textures.acquire("monsters", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"monsters.png"));
}

void release_resources()
{
	global_textures.release("background");
	global_textures.release("empty_white_texture");
	global_textures.release("doodle");
	global_textures.release("tiles");
	global_textures.release("items");
	global_textures.release("monsters");
}