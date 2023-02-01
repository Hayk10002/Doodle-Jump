#include "Items.hpp"

#include <gameObjects/Doodle.hpp>
#include <gameObjects/Tiles.hpp>
#include <common/Resources.hpp>
#include <common/Utils.hpp>

Item::DoodleManipulator::DoodleManipulator(Doodle* doodle)
{
	setDoodle(doodle);
}

void Item::DoodleManipulator::setDoodle(Doodle * doodle)
{
	m_doodle = doodle;
}

void Item::DoodleManipulator::jumpWithSpeed(float speed)
{
	if (!m_doodle) return;
	float curr_jump_speed = m_doodle->getJumpingSpeed();
	m_doodle->setJumpingSpeed(speed);
	m_doodle->jump();
	m_doodle->setJumpingSpeed(curr_jump_speed);
}

void Item::DoodleManipulator::setRotation(float rotation)
{
	if (m_doodle) m_doodle->setRotation(rotation);
}

auto Item::DoodleManipulator::getBodyStatus()
{
	if (!m_doodle) return Doodle::Right;
	return m_doodle->m_body_status;
}

void Item::DoodleManipulator::setBodyStatus(int status)
{
	if (!m_doodle) return;
	m_doodle->m_body_status = Doodle::BodyStatus(status);
}

Item::Item() : Item(nullptr)
{}

Item::Item(sf::Texture * texture_ptr):
	m_animations(new thor::AnimationMap<sf::Sprite, std::string>()),
	m_animator(*m_animations)
{
	if (texture_ptr) setTexture(*texture_ptr);
}

sf::FloatRect Item::getCollisionBox() const
{
	return m_collision_box;
}

sf::Vector2f Item::getCollisionBoxSize() const
{
	return m_collision_box_size;
}

bool Item::isDestroyed() const
{
	return false;
}

bool Item::isReadyToBeDeleted() const
{
	return m_is_ready_to_be_deleted;
}

void Item::setDoodleCollisionCallback(std::function<void(Doodle*)> on_doodle_collision)
{
	m_on_doodle_collision = on_doodle_collision;
}

void Item::updatePhysics(sf::Time dt)
{
	m_velocity += m_gravity * dt.asSeconds();
	move(0, m_velocity * dt.asSeconds());
}

void Item::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(sf::Sprite(*this), states);
}

Spring::Spring(Tile* tile) :
	Item(&global_textures["items"]),
	m_tile(tile)
{
	m_collision_box_size = sf::Vector2f{34, 24} *m_texture_scale;
	setDoodleCollisionCallback([this](Doodle* doodle)
	{
		if (!doodle) return;
		if (doodle->getVelocity().y < 0) return;
		if (!doodle->getFeetCollisionBox().intersects(getCollisionBox())) return;
		m_animator.play() << "expand" << thor::Playback::loop("default");
		m_doodle_manip.setDoodle(doodle);
		m_doodle_manip.jumpWithSpeed(m_jumping_speed);
		m_doodle_manip.setDoodle(nullptr);
	});
	tile->setReadyToBeDeleted(false);
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 0, 0, 34, 24 }, { 17, 12 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	thor::FrameAnimation expand_animation;
	expand_animation.addFrame(1, { 0, 24, 34, 54 }, { 17, 42 });
	m_animations->addAnimation("expand", expand_animation, sf::seconds(1));
	m_animator.play() << thor::Playback::loop("default");
}

void Spring::setOffsetFromTile(float offset)
{
	m_offset.x = offset;
}

void Spring::update(sf::Time dt)
{
	if (m_tile)
	{
		setPosition(m_tile->getPosition() + m_offset);
		if (m_tile->isFallenOffScreen() || m_tile->isDestroyed())
		{
			m_tile->setReadyToBeDeleted(true);
			m_tile = nullptr;
		}
	}
	else updatePhysics(dt);
	
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_animator.update(dt);
	m_animator.animate(*this);
}

float Trampoline::getDoodleRotation(float progress)
{
	//return m_is_doodle_in_rotation * 180 * (-thor::TrigonometricTraits<float>::sin(180 * progress + 90) + 1);
	return m_is_doodle_in_rotation * 360 * (1 - (progress - 1) * (progress - 1));
}

Trampoline::Trampoline(Tile* tile) :
	Item(&global_textures["items"]),
	m_tile(tile)
{
	m_collision_box_size = sf::Vector2f{ 72, 28 } *m_texture_scale;
	setDoodleCollisionCallback([this](Doodle* doodle)
		{
			if (!doodle) return;
			if (doodle->getVelocity().y < 0) return;
			if (!doodle->getFeetCollisionBox().intersects(getCollisionBox())) return;
			m_animator.play() << "bounce" << thor::Playback::loop("default");
			m_is_ready_to_be_deleted = false;
			m_doodle_rotation_start = m_existing_time;
			m_doodle_manip.setDoodle(doodle);
			m_is_doodle_in_rotation = (m_doodle_manip.getBodyStatus() ? -1 : 1);
			m_doodle_body_status = m_doodle_manip.getBodyStatus();
			m_doodle_manip.jumpWithSpeed(m_jumping_speed);
		});
	tile->setReadyToBeDeleted(false);
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 34, 0, 72, 28 }, { 36, 14 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	thor::FrameAnimation bounce_animation;
	bounce_animation.addFrame(1, { 34, 28, 72, 28 }, { 36, 14 });
	bounce_animation.addFrame(1, { 34, 56, 72, 34 }, { 36, 20 });
	bounce_animation.addFrame(1, { 34, 28, 72, 28 }, { 36, 14 });
	m_animations->addAnimation("bounce", bounce_animation, sf::seconds(0.1));
	m_animator.play() << thor::Playback::loop("default");
}

void Trampoline::setOffsetFromTile(float offset)
{
	m_offset.x = offset;
}

void Trampoline::update(sf::Time dt)
{
	m_existing_time += dt;
	if (m_tile)
	{
		setPosition(m_tile->getPosition() + m_offset);
		if (m_tile->isFallenOffScreen() || m_tile->isDestroyed())
		{
			m_tile->setReadyToBeDeleted(true);
			m_tile = nullptr;
		}
	}
	else updatePhysics(dt);

	if (m_is_doodle_in_rotation)
	{
		if (m_existing_time - m_doodle_rotation_start >= m_doodle_rotation_duration)
		{
			m_doodle_manip.setRotation(0);
			m_doodle_manip.setDoodle(nullptr);
			m_is_doodle_in_rotation = false;
			m_is_ready_to_be_deleted = true;
		}
		else
		{
			m_doodle_manip.setRotation(getDoodleRotation((m_existing_time - m_doodle_rotation_start) / m_doodle_rotation_duration));
			m_doodle_manip.setBodyStatus(m_doodle_body_status);
		}
	}

	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_animator.update(dt);
	m_animator.animate(*this);
}

Items::Items(sf::RenderWindow& game_window):
	m_game_window(game_window)
{
}

void Items::update(sf::Time dt)
{
	for (auto& item : m_items) item->update(dt);

	for (size_t i = 0; i < m_items.size(); i++)
		if (sf::FloatRect area = utils::getViewArea(m_game_window); m_items[i]->isDestroyed() || m_items[i]->getCollisionBox().top > area.top + area.height)
			if(m_items[i]->isReadyToBeDeleted())
				m_items.erase(m_items.begin() + i--);
}

void Items::updateDoodleCollisions(Doodle* doodle)
{
	for (auto& item : m_items)
		if (item->getCollisionBox().intersects(doodle->getFeetCollisionBox())
		 || item->getCollisionBox().intersects(doodle->getBodyCollisionBox()))
			item->m_on_doodle_collision(doodle);
}

size_t Items::getItemsCount()
{
	return m_items.size();
}

void Items::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (const auto& item : m_items)
	{
		target.draw(*item, states);/*
		sf::RectangleShape sh{ {item->getCollisionBox().width, item->getCollisionBox().height} };
		sh.setPosition(item->getCollisionBox().left, item->getCollisionBox().top);
		sh.setFillColor(sf::Color(255, 0, 0, 100));
		target.draw(sh, states);*/
	}
}

