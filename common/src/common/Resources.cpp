#include "Resources.hpp"

#include <DoodleJumpConfig.hpp>

thor::ResourceHolder <sf::Texture, std::string> global_textures;

void init_resources()
{
	global_textures.acquire("background", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"background.jpg"));
	global_textures["background"].setRepeated(true);
	global_textures["background"].setSmooth(true);
	global_textures.acquire("empty_white_texture", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"empty_white_square.png"));
	global_textures["empty_white_texture"].setRepeated(true);
}

void release_resources()
{
	global_textures.release("background");
	global_textures.release("empty_white_texture");
}