#include "Resources.hpp"

#include <DoodleJumpConfig.hpp>

thor::ResourceHolder <sf::Texture, std::string> textures_holder;

void init_resources()
{
	textures_holder.acquire("background", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"background.jpg"));
}
