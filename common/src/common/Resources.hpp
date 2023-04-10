#pragma once

#include <SFML/Graphics.hpp>
#include <Thor/Resources.hpp>

extern thor::ResourceHolder <sf::Texture, std::string> global_textures;

void init_resources();
void release_resources();