#pragma once

#include <SFML/Graphics.hpp>
#include <Thor/Resources.hpp>

extern thor::ResourceHolder <sf::Texture, std::string> textures_holder;

void init_resources();