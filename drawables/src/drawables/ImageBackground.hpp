#pragma once
#include <SFML/Graphics.hpp>

class ImageBackground : public sf::Transformable, public sf::Drawable
{

public:

	ImageBackground(sf::Texture& texture, sf::FloatRect background_covering_area);
	ImageBackground(sf::Texture& texture, sf::FloatRect background_covering_area, sf::IntRect texture_rect);

	void update();
	void setTexture(sf::Texture& texture);
	void setTextureRect(sf::IntRect texture_rect);
	void setBackgroundCoveringArea(sf::FloatRect background_covering_area);
	void setColor(sf::Color color);

	sf::Texture& getTexture();
	const sf::Texture& getTexture() const;
	sf::IntRect getTextureRect() const;
	sf::FloatRect getBackgroundCoveringArea() const;
	sf::Color getColor() const;

private:

	sf::Texture& m_texture;
	sf::IntRect m_texture_rect;
	sf::FloatRect m_background_covering_area;
	sf::Color m_color{sf::Color::White};
	sf::IntRect m_needed_sprites;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};