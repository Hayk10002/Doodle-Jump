#pragma once

#include <SFML/Graphics.hpp>
#include <Thor/Resources.hpp>

enum class UserActions
{
	None,
	Close,
	Resize,
	Left,
	Right,
	Shoot,
	Die,
	Ressurect,
	BreakPoint
};

extern thor::ResourceHolder <sf::Texture, std::string> global_textures;

void init_resources();
void release_resources();