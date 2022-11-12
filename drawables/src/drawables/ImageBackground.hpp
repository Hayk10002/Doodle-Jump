#pragma once

#include <functional>
#include <SFML/Graphics.hpp>

class ImageBackground : public sf::Transformable, public sf::Drawable
{

public:

	ImageBackground(sf::Texture& texture, sf::FloatRect background_covering_area);
	ImageBackground(sf::Texture& texture, sf::FloatRect background_covering_area, sf::IntRect texture_rect);
	ImageBackground(sf::Texture& texture, sf::FloatRect* background_covering_area_ptr);
	ImageBackground(sf::Texture& texture, sf::FloatRect* background_covering_area_ptr, sf::IntRect texture_rect);
	ImageBackground(sf::Texture& texture, sf::RenderTarget* background_target_ptr);
	ImageBackground(sf::Texture& texture, sf::RenderTarget* background_target_ptr, sf::IntRect texture_rect);

	void update();
	void setTexture(sf::Texture& texture);
	void setTextureRect(sf::IntRect texture_rect);
	void setBackgroundCoveringArea(sf::FloatRect background_covering_area);
	void setBackgroundCoveringArea(sf::FloatRect* background_covering_area_ptr);
	void setBackgroundCoveringArea(sf::RenderTarget* background_target_ptr);
	void setColor(sf::Color color);

	sf::Texture& getTexture();
	const sf::Texture& getTexture() const;
	sf::IntRect getTextureRect() const;
	sf::FloatRect getBackgroundCoveringArea() const;
	sf::Color getColor() const;

private:

	sf::Texture& m_texture;
	sf::IntRect m_texture_rect;


	std::function<sf::FloatRect()> m_get_background_covering_area;
	std::function<void(sf::FloatRect)> m_set_background_covering_area;
	sf::FloatRect m_background_covering_area;
	sf::FloatRect* m_background_covering_area_ptr{ nullptr };
	sf::RenderTarget* m_background_target_ptr{ nullptr };


	sf::Color m_color{sf::Color::White};
	sf::IntRect m_needed_sprites;

	void setBackgroundSetGetters(sf::FloatRect background_covering_area);
	void setBackgroundSetGetters(sf::FloatRect* background_covering_area_ptr);
	void setBackgroundSetGetters(sf::RenderTarget* background_target_ptr);

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};