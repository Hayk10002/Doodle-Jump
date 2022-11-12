#pragma once

#include <functional>
#include <SFML/Graphics.hpp>

//class that renders a background for some area filling everything with an image independently from rotation scale or position of the image
class ImageBackground : public sf::Transformable, public sf::Drawable
{

public:
	//constructors -> takes the texture of the image, the area (in some way), and optionally the texture rect

	//as area takes simple sf::FloatRect, if the area changes in the program, it will be needed to update the area for the background
	ImageBackground(sf::Texture& texture, sf::FloatRect background_covering_area);
	ImageBackground(sf::Texture& texture, sf::FloatRect background_covering_area, sf::IntRect texture_rect);

	//as area takes a pointer to sf::FloatRect, thus, if the area ander the pointer is changed, the class will update its behavior automatically
	ImageBackground(sf::Texture& texture, sf::FloatRect* background_covering_area_ptr);
	ImageBackground(sf::Texture& texture, sf::FloatRect* background_covering_area_ptr, sf::IntRect texture_rect);

	//as area takes a pointer to sf::RenderTarget, thus, it will completely fill the background of the target even when its size changes
	ImageBackground(sf::Texture& texture, sf::RenderTarget* background_target_ptr);
	ImageBackground(sf::Texture& texture, sf::RenderTarget* background_target_ptr, sf::IntRect texture_rect);

	//update (needed to be called every frame)
	void update();

	//set the texture of the image
	void setTexture(sf::Texture& texture);

	//set the texture rect of the texture
	void setTextureRect(sf::IntRect texture_rect);

	//set the background area (see constructors for more information)
	void setBackgroundCoveringArea(sf::FloatRect background_covering_area);
	void setBackgroundCoveringArea(sf::FloatRect* background_covering_area_ptr);
	void setBackgroundCoveringArea(sf::RenderTarget* background_target_ptr);

	//set the color of the image
	void setColor(sf::Color color);

	//get the texture of the image
	sf::Texture& getTexture();
	const sf::Texture& getTexture() const;

	//get the texture rect of the texture
	sf::IntRect getTextureRect() const;

	//get the background area
	sf::FloatRect getBackgroundCoveringArea() const;

	//get the background area's rotation (can return non-zero only if the area was setted with sf::RenderTarget)
	float getBackgroundCoveringAreaRotation() const;

	//get the color of the image
	sf::Color getColor() const;

private:

	sf::Texture& m_texture;
	sf::IntRect m_texture_rect;

	//functions to get or set the background area and its rotations (if needed)
	std::function<std::pair<sf::FloatRect, float>()> m_get_background_covering_area;
	std::function<void(sf::FloatRect)> m_set_background_covering_area;


	sf::FloatRect m_background_covering_area;
	sf::FloatRect* m_background_covering_area_ptr{ nullptr };
	sf::RenderTarget* m_background_target_ptr{ nullptr };

	sf::Color m_color{sf::Color::White};

	//represents where to draw image, and how many of them needed
	sf::IntRect m_needed_sprites;

	//sets the functions needed to get or set the bacground area and its rotations (if needed)
	void setBackgroundSetGetters(sf::FloatRect background_covering_area);
	void setBackgroundSetGetters(sf::FloatRect* background_covering_area_ptr);
	void setBackgroundSetGetters(sf::RenderTarget* background_target_ptr);

	//draws the background on a target
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};