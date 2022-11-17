#include "Resources.hpp"

#include <DoodleJumpConfig.hpp>

thor::ResourceHolder <sf::Texture, std::string> textures_holder;

void init_resources()
{
	textures_holder.acquire("background", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"background.jpg"));
	textures_holder["background"].setRepeated(true);
	textures_holder.acquire("empty_white_texture", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"empty_white_square.png"));
	textures_holder["empty_white_texture"].setRepeated(true);
}

void release_resources()
{
	textures_holder.release("background");
	textures_holder.release("empty_white_texture");
}