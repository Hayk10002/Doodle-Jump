#include "Doodle.hpp"

#include <common/Utils.hpp>
#include <gameObjects/Tiles.hpp>

Doodle::Doodle(sf::Vector2f starting_pos)
{
	setPosition(starting_pos);
	m_body_normal_exhind = m_body.addExhibit({ {0, 0, 400, 400}, {200, 200} });
	m_body_shooting_exhind = m_body.addExhibit({ {400, 0, 400, 400}, {200, 200} });
	m_feet_normal_exhind = m_feet.addExhibit({ {0, 400, 400, 100}, {200, 50} });
	m_feet_shooting_exhind = m_feet.addExhibit({ {400, 400, 400, 100}, {200, 50} });
	m_nose_exhind = m_nose.addExhibit({ {800, 0, 200, 200},{ 100, 100 } });

	m_body.set(m_body_normal_exhind);
	m_feet.set(m_feet_normal_exhind);
	m_feet.setOrigin(m_feet_offset);
	m_nose.set(m_nose_exhind);
	m_nose.setOrigin(m_nose_not_shooting_offset);
}

void Doodle::update(sf::Time dt)
{
	m_speed_change_rate = 2 * m_moving_speed;
	if (m_is_jumping) if (m_jumping_clock.getElapsedTime() > m_jumping_interval) m_is_jumping = false;
	if (m_is_shooting)
	{
		if (m_shooting_clock.getElapsedTime() > m_shooting_interval) m_is_shooting = false;
		m_body_status = Up;
	}

	m_velocity += m_gravity * dt.asSeconds();
	m_velocity.x /= std::pow(m_speed_decreasing_rate, dt.asSeconds());
	move(m_velocity * dt.asSeconds());

	if (m_area != sf::FloatRect{})
	{
		if (getPosition().x > m_area.left + m_area.width) move(-m_area.width, 0);
		if (getPosition().x < m_area.left) move(m_area.width, 0);
		m_is_fallen_out = (m_body.getGlobalBounds().top > m_area.top + m_area.height);
		m_is_too_high = (getPosition().y < m_area.top);
	}

	if (m_velocity.x > 0 && !m_is_shooting) m_body_status = Right;
	if (m_velocity.x < 0 && !m_is_shooting) m_body_status = Left;

	m_feet.set(m_is_shooting ? m_feet_shooting_exhind : m_feet_normal_exhind);
	m_feet.setOrigin(m_is_jumping ? -m_feet_offset - m_feet_jumping_offset : -m_feet_offset);
	m_feet.setPosition(getPosition());
	m_feet.setRotation(getRotation());
	m_feet.setScale(getScale());
	m_feet.scale((m_body_status == Up || m_body_status == Right) ? sf::Vector2f{ 1, 1 } : sf::Vector2f{ -1, 1 });
	m_feet.scale(m_texture_scale);							
	m_feet_collision_box = sf::FloatRect{ getPosition() + utils::element_wiseProduct(m_feet_offset, m_texture_scale) - utils::element_wiseProduct(m_feet_collision_box_size / 2.f, m_texture_scale), utils::element_wiseProduct(m_feet_collision_box_size, m_texture_scale)};

	m_body.set(m_is_shooting ? m_body_shooting_exhind : m_body_normal_exhind);
	m_body.setPosition(getPosition());
	m_body.setRotation(getRotation());
	m_body.setScale(getScale());
	m_body.scale((m_body_status == Up || m_body_status == Right) ? sf::Vector2f{ 1, 1 } : sf::Vector2f{ -1, 1 });
	m_body.scale(m_texture_scale);

	m_nose.set(m_nose_exhind);
	m_nose.setOrigin(m_is_shooting ? sf::Vector2f{ -m_nose_distance_from_rotation_center, 0 } : -m_nose_not_shooting_offset);
	m_nose.setPosition(getPosition());
	m_nose.move(m_is_shooting ? utils::element_wiseProduct(m_nose_rotation_center, m_texture_scale) : sf::Vector2f{0, 0});
	m_nose.setRotation(getRotation());
	m_nose.rotate(m_is_shooting ? (m_nose_angle - 90) : 0);
	m_nose.setScale(getScale());
	m_nose.scale((m_body_status == Up || m_body_status == Right) ? sf::Vector2f{ 1, 1 } : sf::Vector2f{ -1, 1 });
	m_nose.scale(m_texture_scale);
}

void Doodle::left(sf::Time dt)
{
	m_velocity.x += dt.asSeconds() * m_speed_change_rate * ((-m_velocity.x < m_moving_speed) ? -1 : 1);
}

void Doodle::right(sf::Time dt)
{
	m_velocity.x += dt.asSeconds() * m_speed_change_rate * ((m_velocity.x < m_moving_speed) ? 1 : -1);
}

void Doodle::shoot(float angle)
{
	if (m_is_shooting) return;
	m_is_shooting = true;
	m_nose_angle = std::clamp(angle, -m_max_nose_angle_dev, m_max_nose_angle_dev);
	m_shooting_clock.restart();
	//TODO: Generate a bullet
}

void Doodle::updateArea(sf::FloatRect area)
{
	m_area = sf::FloatRect{ area.left, area.top + area.height / 2, area.width, area.height / 2};
}

void Doodle::updateTiles(Tiles& tiles)
{
	if (m_velocity.y < 0) return;
	if (tiles.willDoodleJump(m_feet_collision_box)) jump();
	if (m_feet_collision_box.left + m_feet_collision_box.width > m_area.left + m_area.width)
	{
		sf::FloatRect moved_collision_box = m_feet_collision_box;
		moved_collision_box.left -= m_area.width;
		if (tiles.willDoodleJump(moved_collision_box)) jump();
	}
	if (m_feet_collision_box.left < m_area.left)
	{
		sf::FloatRect moved_collision_box = m_feet_collision_box;
		moved_collision_box.left += m_area.width;
		if (tiles.willDoodleJump(moved_collision_box)) jump();
	}
}

sf::FloatRect Doodle::getArea()
{
	return m_area;
}

void Doodle::setTexture(sf::Texture* texture_ptr)
{
	if(texture_ptr) m_texture = texture_ptr;
}

bool Doodle::isJumping()
{
	return m_is_jumping;
}

bool Doodle::isShooting()
{
	return m_is_shooting;
}

bool Doodle::isFallenOutOfScreen()
{
	return m_is_fallen_out;
}

bool Doodle::isTooHigh()
{
	return m_is_too_high;
}

void Doodle::jump()
{
	m_is_jumping = true;
	m_velocity.y = -m_jumping_speed;
	m_jumping_clock.restart();
}

void Doodle::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_feet, states);
	target.draw(m_body, states);
	target.draw(m_nose, states);

	sf::FloatRect m_body_bound = m_body.getGlobalBounds();
	if (m_body_bound.left + m_body_bound.width > m_area.left + m_area.width);
	{
		m_feet.move(-m_area.width, 0);
		m_body.move(-m_area.width, 0);
		m_nose.move(-m_area.width, 0);

		target.draw(m_feet, states);
		target.draw(m_body, states);
		target.draw(m_nose, states);

		m_feet.move(m_area.width, 0);
		m_body.move(m_area.width, 0);
		m_nose.move(m_area.width, 0);
	}

	if (m_body_bound.left < m_area.left);
	{
		m_feet.move(m_area.width, 0);
		m_body.move(m_area.width, 0);
		m_nose.move(m_area.width, 0);

		target.draw(m_feet, states);
		target.draw(m_body, states);
		target.draw(m_nose, states);

		m_feet.move(-m_area.width, 0);
		m_body.move(-m_area.width, 0);
		m_nose.move(-m_area.width, 0);
	}
}
