#include "ImageBackground.hpp"

ImageBackground::ImageBackground(sf::Texture& texture, sf::FloatRect background_covering_area):
	m_texture(texture),
	m_texture_rect(sf::IntRect({ 0, 0 }, (sf::Vector2i)texture.getSize()))
{
	setBackgroundSetGetters(background_covering_area);
}

ImageBackground::ImageBackground(sf::Texture& texture, sf::FloatRect background_covering_area, sf::IntRect texture_rect):
	m_texture(texture),
	m_texture_rect(texture_rect)
{
	setBackgroundSetGetters(background_covering_area);
}

ImageBackground::ImageBackground(sf::Texture& texture, sf::FloatRect* background_covering_area_ptr) :
	m_texture(texture),
	m_texture_rect(sf::IntRect({ 0, 0 }, (sf::Vector2i)texture.getSize()))
{
	setBackgroundSetGetters(background_covering_area_ptr);
}

ImageBackground::ImageBackground(sf::Texture& texture, sf::FloatRect* background_covering_area_ptr, sf::IntRect texture_rect) :
	m_texture(texture),
	m_texture_rect(texture_rect)
{
	setBackgroundSetGetters(background_covering_area_ptr);
}

ImageBackground::ImageBackground(sf::Texture& texture, sf::RenderTarget* background_target_ptr) :
	m_texture(texture),
	m_texture_rect(sf::IntRect({ 0, 0 }, (sf::Vector2i)texture.getSize()))
{
	setBackgroundSetGetters(background_target_ptr);
}

ImageBackground::ImageBackground(sf::Texture& texture, sf::RenderTarget* background_target_ptr, sf::IntRect texture_rect) :
	m_texture(texture),
	m_texture_rect(texture_rect)
{
	setBackgroundSetGetters(background_target_ptr);
}

void ImageBackground::update()
{
	sf::Transform tr_no_move;
	tr_no_move.translate(getOrigin());
	tr_no_move.scale(getScale().x, getScale().y);
	tr_no_move.rotate(getRotation());
	sf::Transform inv_tr_no_move = tr_no_move.getInverse();

	sf::Vector2f curr_pos = getPosition();
	curr_pos = inv_tr_no_move.transformPoint(curr_pos);
	curr_pos.x = std::fmodf(curr_pos.x, m_texture_rect.width);
	curr_pos.y = std::fmodf(curr_pos.y, m_texture_rect.height);
	curr_pos = tr_no_move.transformPoint(curr_pos);
	setPosition(curr_pos);

	sf::Transform inv_tr = getInverseTransform();
	auto [background_covering_area, background_covering_area_rotation] = m_get_background_covering_area();
	sf::Transform tr;
	tr.rotate(background_covering_area_rotation);
	sf::Vector2f corners[4] =
	{
		(inv_tr * tr).transformPoint({ background_covering_area.left, background_covering_area.top }),
		(inv_tr * tr).transformPoint({background_covering_area.left + background_covering_area.width, background_covering_area.top}),
		(inv_tr * tr).transformPoint({background_covering_area.left + background_covering_area.width, background_covering_area.top + background_covering_area.height}),
		(inv_tr * tr).transformPoint({background_covering_area.left, background_covering_area.top + background_covering_area.height})
	};

	int top = int(corners[0].y / m_texture_rect.height - 3);
	int bottom = int(corners[0].y / m_texture_rect.height + 3);
	int left = int(corners[0].x / m_texture_rect.width - 3);
	int right = int(corners[0].x / m_texture_rect.width + 3);
	for (int i = 1; i < 4; i++)
	{
		top = std::min(top, int(corners[i].y / m_texture_rect.height - 3));
		bottom = std::max(bottom, int(corners[i].y / m_texture_rect.height + 3));
		left = std::min(left, int(corners[i].x / m_texture_rect.width - 3));
		right = std::max(right, int(corners[i].x / m_texture_rect.width + 3));
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
	setBackgroundSetGetters(background_covering_area);
}

void ImageBackground::setBackgroundCoveringArea(sf::FloatRect* background_covering_area_ptr)
{
	setBackgroundSetGetters(background_covering_area_ptr);
}

void ImageBackground::setBackgroundCoveringArea(sf::RenderTarget* background_target_ptr)
{
	setBackgroundSetGetters(background_target_ptr);
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
	return m_get_background_covering_area().first;
}

float ImageBackground::getBackgroundCoveringAreaRotation() const
{
	return m_get_background_covering_area().second;
}

sf::Color ImageBackground::getColor() const
{
	return m_color;
}

void ImageBackground::setBackgroundSetGetters(sf::FloatRect background_covering_area)
{
	m_background_covering_area = background_covering_area;
	m_get_background_covering_area = [=]()
	{
		return std::pair{ m_background_covering_area, 0.f };
	};

	m_set_background_covering_area = [&](sf::FloatRect bca)
	{
		m_background_covering_area = bca;
	};
}

void ImageBackground::setBackgroundSetGetters(sf::FloatRect* background_covering_area_ptr)
{
	m_background_covering_area_ptr = background_covering_area_ptr;
	m_get_background_covering_area = [=]()
	{
		return std::pair{ *m_background_covering_area_ptr, 0.f };
	};

	m_set_background_covering_area = [](sf::FloatRect) {};
}

void ImageBackground::setBackgroundSetGetters(sf::RenderTarget* background_target_ptr)
{
	m_background_target_ptr = background_target_ptr;
	m_get_background_covering_area = [=]()
	{
		return std::pair{ sf::FloatRect{ m_background_target_ptr->getView().getCenter() - m_background_target_ptr->getView().getSize() / 2.f, m_background_target_ptr->getView().getSize() }, m_background_target_ptr->getView().getRotation()};
	};

	m_set_background_covering_area = [](sf::FloatRect) {};
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
