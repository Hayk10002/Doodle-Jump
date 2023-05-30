#include "Doodle.hpp"

#include <Thor/Math.hpp>
#include <Thor/Vectors.hpp>

#include <common/Utils.hpp>
#include <gameObjects/Tiles.hpp>
#include <gameObjects/Monsters.hpp>

Doodle::Doodle(sf::Vector2f starting_pos)
{
	setPosition(starting_pos);
	m_body_normal_exhind = m_body.addExhibit({ (sf::FloatRect)global_sprites["doodle_body_right"].texture_rect, {40, 40}});
	m_body_shooting_exhind = m_body.addExhibit({ (sf::FloatRect)global_sprites["doodle_body_up"].texture_rect, {40, 40} });
	m_feet_normal_exhind = m_feet.addExhibit({ (sf::FloatRect)global_sprites["doodle_feet_right"].texture_rect, {40, 10} });
	m_feet_shooting_exhind = m_feet.addExhibit({ (sf::FloatRect)global_sprites["doodle_feet_up"].texture_rect, {40, 10} });
	m_nose_exhind = m_nose.addExhibit({ (sf::FloatRect)global_sprites["doodle_nose"].texture_rect, { 20, 20 } });

	m_body.set(m_body_normal_exhind);
	m_feet.set(m_feet_normal_exhind);
	m_feet.setOrigin(m_feet_offset);
	m_nose.set(m_nose_exhind);
	m_nose.setOrigin(m_nose_not_shooting_offset);

	m_head_bump_star.setOrigin(12, 12);
	m_head_bump_star.setScale(m_texture_scale);

	m_bullet_sprite.setTextureRect(global_sprites["doodle_bullet"].texture_rect);
	m_bullet_sprite.setScale(m_texture_scale);
	m_bullet_sprite.setOrigin(11, 11);
}

void Doodle::update(sf::Time dt)
{
	m_existing_time += dt;
	m_speed_change_rate = 2 * m_moving_speed;
	if (m_is_jumping) if (m_existing_time - m_jumping_start > m_jumping_interval) m_is_jumping = false;
	if (m_is_shooting)
	{
		if (m_existing_time - m_shooting_start > m_shooting_interval || !m_can_shoot) m_is_shooting = false;
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

	m_feet_collision_box = sf::FloatRect{ getPosition() + utils::element_wiseProduct(m_feet_offset, m_current_texture_scale) - utils::element_wiseProduct(m_feet_collision_box_size / 2.f, m_current_texture_scale), utils::element_wiseProduct(m_feet_collision_box_size, m_current_texture_scale)};
	m_body_collision_box = sf::FloatRect{ getPosition() - utils::element_wiseProduct(m_body_collision_box_size / 2.f, m_current_texture_scale), utils::element_wiseProduct(m_body_collision_box_size, m_current_texture_scale) };

	if (m_is_dead && m_is_shrinking)
	{
		float progress = std::clamp(m_existing_time - m_shrinking_start, sf::Time::Zero, m_shrinking_duration) / m_shrinking_duration;
		setPosition(std::lerp(m_shrinking_start_pos.x, m_shrinking_pos.x, 1 - (1 - progress) * (1 - progress)), std::lerp(m_shrinking_start_pos.y, m_shrinking_pos.y, progress));
		setRotation(progress * m_shrinking_duration.asSeconds() * m_shrinking_rotation_speed * 360);
		m_current_texture_scale = m_texture_scale * (1 - progress * progress);
	}

	for (size_t i = 0; i < m_bullets.size(); i++)
	{
		m_bullets[i].update(dt);
		if (!sf::FloatRect(m_area.left, m_area.top - 3 * m_area.height, m_area.width, 4 * m_area.height).intersects(m_bullets[i].getGlobalBounds())) m_bullets.erase(m_bullets.begin() + i--);
	}

	m_is_updating_for_drawing_needed = true;
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
	if (!m_can_shoot || m_is_dead) return;
	m_is_shooting = true;
	m_nose_angle = std::clamp(angle, -m_max_nose_angle_dev, m_max_nose_angle_dev);
	m_shooting_start = m_existing_time;

	Bullet bullet(m_bullet_sprite);
	bullet.setVelocity(thor::rotatedVector({ 0, -m_bullet_speed }, m_nose_angle));
	bullet.setPosition(m_nose.getPosition());
	m_bullets.push_back(bullet);
}

void Doodle::updateArea(sf::FloatRect area)
{
	m_area = sf::FloatRect{ area.left, area.top + area.height / 2, area.width, area.height / 2};
}

void Doodle::updateTiles(Tiles& tiles)
{
	if (m_velocity.y < 0) return;
	if (tiles.willDoodleJump(m_feet_collision_box)) jump();
}

void Doodle::updateItems(Items& items)
{
	items.updateDoodleCollisions(this);
}

void Doodle::updateMonsters(Monsters& monsters)
{
	for (size_t i = 0; i < m_bullets.size(); i++) if (monsters.updateBullet(m_bullets[i])) m_bullets.erase(m_bullets.begin() + i--);

	if (m_velocity.y >= 0 && monsters.willDoodleJump(m_feet_collision_box))
	{
		float jumping_speed = getJumpingSpeed();
		setJumpingSpeed(jumping_speed + 150);
		jump();
		setJumpingSpeed(jumping_speed);
		return;
	}
	if(!hasItem() && !hasShield()) monsters.updateDoodleBump(this);
}

sf::FloatRect Doodle::getArea() const
{
	return m_area;
}

void Doodle::dieHeadBump()
{
	die();
	m_velocity = { 0, 0 };
}

void Doodle::dieShrink(sf::Vector2f shrinking_pos, sf::Time duration, float rotation_speed)
{
	die();
	m_is_shrinking = true;
	m_shrinking_start_pos = getPosition();
	m_shrinking_pos = shrinking_pos;
	m_shrinking_duration = duration;
	m_shrinking_rotation_speed = rotation_speed;
	m_shrinking_start = m_existing_time;
}

void Doodle::ressurrect(sf::Vector2f position)
{
	m_is_dead = false;
	setCanJump(true);
	setCanShoot(true);
	m_is_shrinking = false;
	m_current_texture_scale = m_texture_scale;
	setVelocity({ 0, 0 });
	setPosition(position);
	setRotation(0);
}

bool Doodle::isJumping() const
{
	return m_is_jumping;
}

bool Doodle::isShooting() const
{
	return m_is_shooting;
}

bool Doodle::isFallenOutOfScreen() const
{
	return m_is_fallen_out;
}

bool Doodle::isTooHigh() const
{
	return m_is_too_high;
}

bool Doodle::hasItem() const
{
	return m_item;
}

bool Doodle::hasShoes() const
{
	return m_has_shoes;
}

bool Doodle::canShoot() const
{
	return m_can_shoot;
}

bool Doodle::canJump() const
{
	return m_can_jump;
}

bool Doodle::drawFeet() const
{
	return m_draw_feet;
}

bool Doodle::hasShield() const
{
	return m_shield;
}

bool Doodle::isDead() const
{
	return m_is_dead;
}

bool Doodle::isCompletelyDead() const
{
	return m_current_texture_scale == sf::Vector2f{ 0, 0 } || getPosition().y > getArea().top + getArea().height + 50;
}

sf::Vector2f Doodle::getVelocity() const
{
	return m_velocity;
}

sf::Vector2f Doodle::getGravity() const
{
	return m_gravity;
}

float Doodle::getJumpingSpeed() const
{
	return m_jumping_speed;
}

sf::FloatRect Doodle::getFeetCollisionBox() const
{
	return m_feet_collision_box;
}

sf::FloatRect Doodle::getBodyCollisionBox() const
{
	return m_body_collision_box;
}

void Doodle::jump()
{
	if (!m_can_jump || m_is_dead) return;
	if (m_velocity.y < 0) return;
	m_is_jumping = true;
	m_velocity.y = -m_jumping_speed;
	m_jumping_start = m_existing_time;
}

void Doodle::setVelocity(sf::Vector2f velocity)
{
	m_velocity = velocity;
}

void Doodle::setGravity(sf::Vector2f gravity)
{
	m_gravity = gravity;
}

void Doodle::setJumpingSpeed(float speed)
{
	m_jumping_speed = speed;
}

void Doodle::setItem(sf::Drawable* item)
{
	m_item = item;
}

void Doodle::setHasShoes(bool val)
{
	m_has_shoes = val;
}

void Doodle::setCanShoot(bool val)
{
	m_can_shoot = val;
}

void Doodle::setCanJump(bool val)
{
	m_can_jump = val;
}

void Doodle::setDrawFeet(bool val)
{
	m_draw_feet = val;
}

void Doodle::setShield(Shield* shield)
{
	m_shield = shield;
}

void Doodle::updateForDrawing() const
{
	if(m_is_updating_for_drawing_needed)
	{
		m_feet.set(m_is_shooting ? m_feet_shooting_exhind : m_feet_normal_exhind);
		m_feet.setOrigin(m_is_jumping ? -m_feet_offset - m_feet_jumping_offset : -m_feet_offset);
		m_feet.setPosition(getPosition());
		m_feet.setRotation(getRotation());
		m_feet.setScale(getScale());
		m_feet.scale((m_body_status == Up || m_body_status == Right) ? sf::Vector2f{ 1, 1 } : sf::Vector2f{ -1, 1 });
		m_feet.scale(m_current_texture_scale);

		m_body.set(m_is_shooting ? m_body_shooting_exhind : m_body_normal_exhind);
		m_body.setPosition(getPosition());
		m_body.setRotation(getRotation());
		m_body.setScale(getScale());
		m_body.scale((m_body_status == Up || m_body_status == Right) ? sf::Vector2f{ 1, 1 } : sf::Vector2f{ -1, 1 });
		m_body.scale(m_current_texture_scale);

		m_nose.set(m_nose_exhind);
		m_nose.setOrigin(m_is_shooting ? sf::Vector2f{ -m_nose_distance_from_rotation_center, 0 } : -m_nose_not_shooting_offset);
		m_nose.setPosition(getPosition());
		m_nose.move(m_is_shooting ? utils::element_wiseProduct(m_nose_rotation_center, m_current_texture_scale) : sf::Vector2f{ 0, 0 });
		m_nose.setRotation(getRotation());
		m_nose.rotate(m_is_shooting ? (m_nose_angle - 90) : 0);
		m_nose.setScale(getScale());
		m_nose.scale((m_body_status == Up || m_body_status == Right) ? sf::Vector2f{ 1, 1 } : sf::Vector2f{ -1, 1 });
		m_nose.scale(m_current_texture_scale);

		m_is_updating_for_drawing_needed = false;
	}
}

void Doodle::animateHeadBumpStars(size_t index) const
{
	float angle = m_existing_time / m_head_bump_stars_rotation_time * 360 + index * 90;
	sf::Vector2f offset{ thor::TrigonometricTraits<float>::sin(angle) * m_head_bump_stars_rotating_boundaries.x / 2, thor::TrigonometricTraits<float>::cos(angle) * m_head_bump_stars_rotating_boundaries.y / 2 };
	m_head_bump_star.setPosition(getPosition() + m_head_bump_stars_offset + offset);
}

void Doodle::die()
{
	if (isDead()) return;
	m_is_dead = true;
	setCanJump(false);
	setCanShoot(false);
}

void Doodle::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	updateForDrawing();
	
	for (auto& bullet : m_bullets) target.draw(bullet, states);

	if(m_draw_feet) target.draw(m_feet, states);
	target.draw(m_body, states);
	target.draw(m_nose, states);

	if (hasItem()) target.draw(*m_item, states);
	
	if (hasShield()) target.draw(*m_shield, states);

	if (isDead() && !isFallenOutOfScreen() && !m_is_shrinking) for (size_t i = 0; i < 4; i++)
	{
		animateHeadBumpStars(i);
		target.draw(m_head_bump_star, states);
	}


	/*sf::RectangleShape sh_feet{ {m_feet_collision_box.width, m_feet_collision_box.height} };
	sh_feet.setPosition(m_feet_collision_box.left, m_feet_collision_box.top);
	sh_feet.setFillColor(sf::Color(255, 0, 0, 100));
	target.draw(sh_feet, states); 

	sf::RectangleShape sh_body{ {m_body_collision_box.width, m_body_collision_box.height} };
	sh_body.setPosition(m_body_collision_box.left, m_body_collision_box.top);
	sh_body.setFillColor(sf::Color(255, 0, 0, 100));
	target.draw(sh_body, states);*/
}

Bullet::Bullet(const sf::Sprite& sprite):
	Sprite(sprite)
{}

void Bullet::setVelocity(sf::Vector2f velocity)
{
	m_velocity = velocity;
}

void Bullet::setGravity(sf::Vector2f gravity)
{
	m_gravity = gravity;
}

sf::Vector2f Bullet::getVelocity() const
{
	return m_velocity;
}

sf::Vector2f Bullet::getGravity() const
{
	return m_gravity;
}

void Bullet::update(sf::Time dt)
{
	m_velocity += m_gravity * dt.asSeconds();
	move(m_velocity * dt.asSeconds());
}

void Bullet::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(sf::Sprite(*this), states);
}
