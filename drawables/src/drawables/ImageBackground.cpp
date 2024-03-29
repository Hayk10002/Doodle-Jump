#include "ImageBackground.hpp"

#include <common/Resources.hpp>


ImageBackground::ImageBackground(sf::Texture* texture_ptr, sf::FloatRect background_covering_area)
{
	setTexture(texture_ptr);
	m_texture_rect = sf::IntRect({ 0, 0 }, (sf::Vector2i)m_texture_ptr->getSize());
	setBackgroundSetGetters(background_covering_area);
}

ImageBackground::ImageBackground(sf::Texture* texture_ptr, sf::FloatRect background_covering_area, sf::IntRect texture_rect):
	m_texture_rect(texture_rect)
{
	setTexture(texture_ptr);
	setBackgroundSetGetters(background_covering_area);
}

ImageBackground::ImageBackground(sf::Texture* texture_ptr, sf::FloatRect* background_covering_area_ptr)
{
	setTexture(texture_ptr);
	m_texture_rect = sf::IntRect({ 0, 0 }, (sf::Vector2i)m_texture_ptr->getSize());
	setBackgroundSetGetters(background_covering_area_ptr);
}

ImageBackground::ImageBackground(sf::Texture* texture_ptr, sf::FloatRect* background_covering_area_ptr, sf::IntRect texture_rect) :
	m_texture_rect(texture_rect)
{
	setTexture(texture_ptr);
	setBackgroundSetGetters(background_covering_area_ptr);
}

ImageBackground::ImageBackground(sf::Texture* texture_ptr, sf::RenderTarget* background_target_ptr)
{
	setTexture(texture_ptr);
	m_texture_rect = sf::IntRect({ 0, 0 }, (sf::Vector2i)m_texture_ptr->getSize());
	setBackgroundSetGetters(background_target_ptr);
}

ImageBackground::ImageBackground(sf::Texture* texture_ptr, sf::RenderTarget* background_target_ptr, sf::IntRect texture_rect) :
	m_texture_rect(texture_rect)
{
	setTexture(texture_ptr);
	setBackgroundSetGetters(background_target_ptr);
}

void ImageBackground::update()
{
	//moves the object in the way that there are no visual changes but the position is closer to zero
	if(move_to_0_enabled) moveTo0();

	//other update stuff
	update_impl();
}

void ImageBackground::update() const
{
	update_impl();
}

void ImageBackground::moveTo0()
{
	//computes the transformation of this object, without counting the position
	sf::Transform tr_no_move;
	tr_no_move.translate(getOrigin());
	tr_no_move.scale(getScale().x, getScale().y);
	tr_no_move.rotate(getRotation());
	sf::Transform inv_tr_no_move = tr_no_move.getInverse();

	//moves the object in the way that there are no visual changes but the position is closer to zero
	sf::Vector2f curr_pos = getPosition();
	curr_pos = inv_tr_no_move.transformPoint(curr_pos);
	curr_pos.x = std::fmodf(curr_pos.x, m_texture_rect.width);
	curr_pos.y = std::fmodf(curr_pos.y, m_texture_rect.height);
	curr_pos = tr_no_move.transformPoint(curr_pos);
	setPosition(curr_pos);
}

void ImageBackground::setTexture(sf::Texture* texture_ptr)
{
	if (texture_ptr) m_texture_ptr = texture_ptr;
	else m_texture_ptr = &global_textures["empty_white_texture"];
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

void ImageBackground::setMovingTo0(bool enabled)
{
	move_to_0_enabled = enabled;
}

sf::Texture* ImageBackground::getTexture()
{
	return m_texture_ptr;
}

const sf::Texture* ImageBackground::getTexture() const
{
	return m_texture_ptr;
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

bool ImageBackground::getMovingTo0() const
{
	return move_to_0_enabled;
}

void ImageBackground::update_impl() const
{
	//transforms the corner points of the background area to the image "space"
	sf::Transform inv_tr = getInverseTransform();
	auto [background_covering_area, background_covering_area_rotation] = m_get_background_covering_area();
	sf::Transform tr;
	tr.translate(background_covering_area.left + background_covering_area.width / 2.f, background_covering_area.top + background_covering_area.height / 2.0);
	tr.rotate(background_covering_area_rotation);
	tr.translate(-background_covering_area.left - background_covering_area.width / 2.f, -background_covering_area.top - background_covering_area.height / 2.0);

	background_covering_area = tr.transformRect(background_covering_area);
	sf::Vector2f corners[4] =
	{
		inv_tr.transformPoint({ background_covering_area.left, background_covering_area.top }),
		inv_tr.transformPoint({background_covering_area.left + background_covering_area.width, background_covering_area.top}),
		inv_tr.transformPoint({background_covering_area.left + background_covering_area.width, background_covering_area.top + background_covering_area.height}),
		inv_tr.transformPoint({background_covering_area.left, background_covering_area.top + background_covering_area.height})
	};

	//computes where to draw the image and how many of them needed to cover the 4 corners 
	int top = int(corners[0].y / m_texture_rect.height - 1);
	int bottom = int(corners[0].y / m_texture_rect.height + 1);
	int left = int(corners[0].x / m_texture_rect.width - 1);
	int right = int(corners[0].x / m_texture_rect.width + 1);
	for (int i = 1; i < 4; i++)
	{
		top = std::min(top, int(corners[i].y / m_texture_rect.height - 1));
		bottom = std::max(bottom, int(corners[i].y / m_texture_rect.height + 1));
		left = std::min(left, int(corners[i].x / m_texture_rect.width - 1));
		right = std::max(right, int(corners[i].x / m_texture_rect.width + 1));
	}

	m_needed_sprites.top = top;
	m_needed_sprites.height = bottom - top;
	m_needed_sprites.left = left;
	m_needed_sprites.width = right - left;
}

void ImageBackground::setBackgroundSetGetters(sf::FloatRect background_covering_area)
{
	m_background_covering_area = background_covering_area;
	m_get_background_covering_area = [this]()
	{
		return std::pair{ m_background_covering_area, 0.f };
	};

	m_set_background_covering_area = [this](sf::FloatRect bca)
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
	sf::IntRect texture_rect = (m_texture_ptr->isRepeated() ?
								sf::IntRect(0, 0, 
											m_needed_sprites.width * m_texture_rect.width,
											m_needed_sprites.height * m_texture_rect.height) :
								m_texture_rect);
	sf::Sprite sp(*m_texture_ptr, texture_rect);
	sp.setColor(m_color);
	if(m_texture_ptr->isRepeated()) 
	{
		sp.setPosition(m_needed_sprites.left * m_texture_rect.width, m_needed_sprites.top * m_texture_rect.height);
		target.draw(sp, states);
	}
	else
	{
		for (int i = m_needed_sprites.top; i < m_needed_sprites.top + m_needed_sprites.height; i++) for (int j = m_needed_sprites.left; j < m_needed_sprites.left + m_needed_sprites.width; j++)
		{
			sp.setPosition(j * m_texture_rect.width, i * m_texture_rect.height);
			target.draw(sp, states);
		}
	}
}
