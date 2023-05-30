#pragma once

#include <unordered_map>

#include <SFML/Graphics.hpp>
#include <Thor/Resources.hpp>

inline thor::ResourceHolder <sf::Texture, std::string> global_textures;

struct SpriteStat
{
	std::string texture_name{};
	sf::IntRect texture_rect{};
	sf::Texture& getTexture();
	sf::Sprite createSprite();
};

inline std::unordered_map<std::string, SpriteStat> global_sprites;

void init_resources();
void release_resources();