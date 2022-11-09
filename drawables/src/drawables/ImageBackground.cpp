#include "ImageBackground.hpp"

ImageBackground::ImageBackground(sf::Texture& texture, sf::FloatRect background_covering_area):
	m_texture(texture),
	m_texture_rect(sf::IntRect({ 0, 0 }, (sf::Vector2i)texture.getSize())),
	m_background_covering_area(background_covering_area)
{}

ImageBackground::ImageBackground(sf::Texture& texture, sf::FloatRect background_covering_area, sf::IntRect texture_rect):
	m_texture(texture),
	m_texture_rect(texture_rect),
	m_background_covering_area(background_covering_area)
{}

void ImageBackground::update()
{
	sf::Transform inv_tr = getInverseTransform();

	sf::Vector2f corners[4] =
	{
		inv_tr.transformPoint({ m_background_covering_area.left, m_background_covering_area.top }),
		inv_tr.transformPoint({m_background_covering_area.left + m_background_covering_area.width, m_background_covering_area.top + m_background_covering_area.height}),
		inv_tr.transformPoint({m_background_covering_area.left + m_background_covering_area.width, m_background_covering_area.top}),
		inv_tr.transformPoint({m_background_covering_area.left, m_background_covering_area.top + m_background_covering_area.height})
	};

	int top = 0, bottom = 0, left = 0, right = 0;

	for (auto corner : corners)
	{
		top = std::min(top, int(corner.y / m_texture_rect.height - 1));
		bottom = std::max(bottom, int(corner.y / m_texture_rect.height + 1));
		left = std::min(left, int(corner.x / m_texture_rect.width - 1));
		right = std::max(right, int(corner.x / m_texture_rect.width + 1));
	}

	m_needed_sprites.top = top;
	m_needed_sprites.height = bottom - top;
	m_needed_sprites.left = left;
	m_needed_sprites.width = right - left;
}

void ImageBackground::setTexture(sf::Texture& texture)
{
	m_texture = texture;
}

void ImageBackground::setTextureRect(sf::IntRect texture_rect)
{
	m_texture_rect = texture_rect;
}

void ImageBackground::setBackgroundCoveringArea(sf::FloatRect background_covering_area)
{
	m_background_covering_area = background_covering_area;
}

void ImageBackground::setColor(sf::Color color)
{
	m_color = color;
}

sf::Texture& ImageBackground::getTexture()
{
	return m_texture;
}

const sf::Texture& ImageBackground::getTexture() const
{
	return m_texture;
}

sf::IntRect ImageBackground::getTextureRect() const
{
	return m_texture_rect;
}

sf::FloatRect ImageBackground::getBackgroundCoveringArea() const
{
	return m_background_covering_area;
}

sf::Color ImageBackground::getColor() const
{
	return m_color;
}

void ImageBackground::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	sf::Sprite sp(m_texture, m_texture_rect);
	sp.setColor(m_color);
	for(int i = m_needed_sprites.top; i < m_needed_sprites.top + m_needed_sprites.height; i++) for (int j = m_needed_sprites.left; j < m_needed_sprites.left + m_needed_sprites.width; j++)
	{
		sp.setPosition(j * m_texture_rect.width, i * m_texture_rect.height);
		target.draw(sp, states);
	}
}
