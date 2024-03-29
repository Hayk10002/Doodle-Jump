#include "Items.hpp"

#include <Thor/Vectors.hpp>

#include <gameObjects/Doodle.hpp>
#include <gameObjects/Tiles.hpp>
#include <gameObjects/Monsters.hpp>
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

bool Item::DoodleManipulator::hasDoodle() const
{
	return m_doodle;
}

void Item::DoodleManipulator::jumpWithSpeed(float speed)
{
	if (!m_doodle) return;
	float curr_jump_speed = m_doodle->getJumpingSpeed();
	m_doodle->setJumpingSpeed(speed);
	m_doodle->jump();
	m_doodle->setJumpingSpeed(curr_jump_speed);
}

float Item::DoodleManipulator::getRotation() const
{
	if (!m_doodle) return 0;
	return m_doodle->getRotation();
}

void Item::DoodleManipulator::setRotation(float rotation)
{
	if (m_doodle) m_doodle->setRotation(rotation);
}

sf::Vector2f Item::DoodleManipulator::getPosition() const
{
	if (!m_doodle) return {};
	return m_doodle->getPosition();
}

sf::Vector2f Item::DoodleManipulator::getVelocity() const
{
	if (!m_doodle) return {};
	return m_doodle->getVelocity();
}

void Item::DoodleManipulator::setVelocity(sf::Vector2f velocity)
{
	if (!m_doodle) return;
	m_doodle->setVelocity(velocity);
}

void Item::DoodleManipulator::move(sf::Vector2f offset)
{
	if (!m_doodle) return;
	m_doodle->move(offset);
}

auto Item::DoodleManipulator::getBodyStatus() const
{
	if (!m_doodle) return Doodle::Right;
	return m_doodle->m_body_status;
}

void Item::DoodleManipulator::setBodyStatus(int status)
{
	if (!m_doodle) return;
	m_doodle->m_body_status = Doodle::BodyStatus(status);
}

bool Item::DoodleManipulator::hasItem() const
{
	if (!m_doodle) return false;
	return m_doodle->hasItem();
}

void Item::DoodleManipulator::setItem(sf::Drawable* item)
{
	if (!m_doodle) return;
	m_doodle->setItem(item);
}

bool Item::DoodleManipulator::hasShoes() const
{
	if (!m_doodle) return false;
	return m_doodle->hasShoes();
}

void Item::DoodleManipulator::setHasShoes(bool val)
{
	if (!m_doodle) return;
	m_doodle->setHasShoes(val);
}

bool Item::DoodleManipulator::canShoot() const
{
	if (!m_doodle) return false;
	return m_doodle->canShoot();
}

void Item::DoodleManipulator::setCanShoot(bool val)
{
	if (!m_doodle) return;
	m_doodle->setCanShoot(val);
}

bool Item::DoodleManipulator::drawFeet() const
{
	if(!m_doodle) return false;
	return m_doodle->drawFeet();
}

void Item::DoodleManipulator::setDrawFeet(bool val)
{
	if (!m_doodle) return;
	m_doodle->setDrawFeet(val);
}

bool Item::DoodleManipulator::canJump() const
{
	if(!m_doodle) return false;
	return m_doodle->canJump();
}

void Item::DoodleManipulator::setCanJump(bool val)
{
	if (!m_doodle) return;
	m_doodle->setCanJump(val);
}

bool Item::DoodleManipulator::isDead() const
{
	if (!m_doodle) return false;
	return m_doodle->isDead();
}

void Item::DoodleManipulator::setShield(Shield* shield)
{
	if (!m_doodle) return;
	m_doodle->setShield(shield);
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

void Item::setDoodleCollisionCallback(OnDoodleCollisionFunctionType on_doodle_collision)
{
	m_on_doodle_collision = on_doodle_collision;
}

void Item::setOffsetFromTile(float offset)
{
	m_tile_offset.x = offset;
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
	Item(&global_sprites["items_spring_0"].getTexture())
{
	m_tile = tile;
	m_tile_offset.y = -14;
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
	default_animation.addFrame(1, global_sprites["items_spring_0"].texture_rect, { 17, 12 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	thor::FrameAnimation expand_animation;
	expand_animation.addFrame(1, global_sprites["items_spring_1"].texture_rect, { 17, 42 });
	m_animations->addAnimation("expand", expand_animation, sf::seconds(1));
	m_animator.play() << thor::Playback::loop("default");
}

void Spring::update(sf::Time dt)
{
	if (m_tile)
	{
		setPosition(m_tile->getPosition() + m_tile_offset);
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
	Item(&global_sprites["items_trampoline_0"].getTexture())
{
	m_tile = tile;
	m_tile_offset.y = -16;
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
	default_animation.addFrame(1, global_sprites["items_trampoline_0"].texture_rect, { 36, 14 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	thor::FrameAnimation bounce_animation;
	bounce_animation.addFrame(1, global_sprites["items_trampoline_0"].texture_rect, { 36, 14 });
	bounce_animation.addFrame(1, global_sprites["items_trampoline_1"].texture_rect, { 36, 20 });
	bounce_animation.addFrame(1, global_sprites["items_trampoline_2"].texture_rect, { 36, 14 });
	m_animations->addAnimation("bounce", bounce_animation, sf::seconds(0.1));
	m_animator.play() << thor::Playback::loop("default");
}

void Trampoline::update(sf::Time dt)
{
	m_existing_time += dt;
	if (m_tile)
	{
		setPosition(m_tile->getPosition() + m_tile_offset);
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

float PropellerHat::getDoodleSpeed(float progress)
{
	return -m_max_speed * (1 - std::pow((progress - 1) / 1.1f, 4));
}

PropellerHat::PropellerHat(Tile* tile):
	Item(&global_sprites["items_propeller_hat_0"].getTexture())
{
	m_tile = tile;
	m_tile_offset.y = -20;
	m_collision_box_size = sf::Vector2f{ 58, 38 } *m_texture_scale;
	setDoodleCollisionCallback([this](Doodle* doodle)
	{
		if (m_is_used) return;
		if (!doodle) return;
		if (doodle->hasItem()) return;
		m_doodle_manip.setDoodle(doodle);
		m_doodle_manip.setItem(this);
		m_doodle_manip.setCanShoot(false);
		if(m_tile)
		{
			m_tile->setReadyToBeDeleted(true);
			m_tile = nullptr;
		}
		m_animator.play() << thor::Playback::loop("rotate");
		m_use_start = m_existing_time;
	});
	m_tile->setReadyToBeDeleted(false);
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, global_sprites["items_propeller_hat_0"].texture_rect, { 29, 19 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	thor::FrameAnimation rotate_animation;
	rotate_animation.addFrame(1, global_sprites["items_propeller_hat_1"].texture_rect, { 29, 27 });
	rotate_animation.addFrame(1, global_sprites["items_propeller_hat_2"].texture_rect, { 29, 33 });
	rotate_animation.addFrame(1, global_sprites["items_propeller_hat_1"].texture_rect, { 29, 27 });
	rotate_animation.addFrame(1, global_sprites["items_propeller_hat_3"].texture_rect, { 29, 33 });
	m_animations->addAnimation("rotate", rotate_animation, sf::seconds(0.5));
	m_animator.play() << thor::Playback::loop("default");
}

void PropellerHat::update(sf::Time dt)
{
	m_existing_time += dt;
	if (m_tile)
	{
		setPosition(m_tile->getPosition() + m_tile_offset);
		if (m_tile->isFallenOffScreen() || m_tile->isDestroyed())
		{
			m_tile->setReadyToBeDeleted(true);
			m_tile = nullptr;
			m_after_use_horizontal_speed = 0;
			m_after_use_rotation_speed = 0;
		}
	}
	else if (m_doodle_manip.hasDoodle())
	{
		if (m_existing_time - m_use_start >= m_use_duration)
		{
			m_doodle_manip.setItem(nullptr);
			m_doodle_manip.setCanShoot(true);
			m_velocity = m_doodle_manip.getVelocity().y - 100;
			m_doodle_manip.setDoodle(nullptr);
			m_is_used = true;
			m_gravity += 400;
			m_animator.play() << thor::Playback::loop("default");
			m_after_use_horizontal_speed = 100;
			m_after_use_rotation_speed = 90;
		}
		else
		{
			m_doodle_manip.setVelocity(sf::Vector2f{ m_doodle_manip.getVelocity().x, getDoodleSpeed((m_existing_time - m_use_start) / m_use_duration) });
			setScale(std::abs(getScale().x) * (m_doodle_manip.getBodyStatus() ? -1 : 1), getScale().y);
			setPosition(m_doodle_manip.getPosition() + thor::rotatedVector(m_doodle_offset, m_doodle_manip.getRotation()));
			setRotation(m_doodle_manip.getRotation());
		}
	}
	else
	{
		rotate((getScale().x > 0 ? 1 : -1) * m_after_use_rotation_speed * dt.asSeconds());
		move((getScale().x > 0 ? -1 : 1) * m_after_use_horizontal_speed * dt.asSeconds(), 0);
		updatePhysics(dt);
	}


	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_animator.update(dt);
	m_animator.animate(*this);
}

PropellerHat::~PropellerHat()
{
	if (m_doodle_manip.hasDoodle())
	{
		m_doodle_manip.setCanShoot(true);
		m_doodle_manip.setItem(nullptr);
	}
}

float Jetpack::getDoodleSpeed(float progress)
{
	if (progress < 0.1) return -m_max_speed * (8 * progress + 0.2);
	if (progress > 0.9) return -m_max_speed * (8 * (1 - progress) + 0.2);
	return -m_max_speed;
}

void Jetpack::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if(m_doodle_manip.hasDoodle()) target.draw(m_body, states);
	target.draw(sf::Sprite(*this), states);
}

Jetpack::Jetpack(Tile* tile) :
	Item(&global_sprites["items_jetpack_0"].getTexture()),
	m_body(global_sprites["items_jetpack_0"].getTexture())
{
	m_tile = tile;
	m_tile_offset.y = -30;
	m_collision_box_size = sf::Vector2f{ 48, 72 } *m_texture_scale;
	setDoodleCollisionCallback([this](Doodle* doodle)
	{
		if (m_is_used) return;
		if (!doodle) return;
		if (doodle->hasItem()) return;
		m_doodle_manip.setDoodle(doodle);
		m_doodle_manip.setItem(this);
		m_doodle_manip.setCanShoot(false);
		if (m_tile)
		{
			m_tile->setReadyToBeDeleted(true);
			m_tile = nullptr;
		}
		m_animator.play() << "start" << thor::Playback::repeat("fly", 10) << "end";
		m_use_start = m_existing_time;
	});
	m_tile->setReadyToBeDeleted(false);
	setScale(m_texture_scale, m_texture_scale);
	m_body.setScale(getScale());
	m_body.setTextureRect(global_sprites["items_jetpack_0"].texture_rect);
	m_body.setOrigin({12, 36});
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, global_sprites["items_jetpack_1"].texture_rect, { 24, 36 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	thor::FrameAnimation start_animation;
	start_animation.addFrame(1, global_sprites["items_jetpack_2"].texture_rect, { 10, 0 });
	start_animation.addFrame(1, global_sprites["items_jetpack_3"].texture_rect, { 14, 0 });
	start_animation.addFrame(1, global_sprites["items_jetpack_4"].texture_rect, { 16, 0 });
	start_animation.addFrame(1, global_sprites["items_jetpack_5"].texture_rect, { 24, 0 });
	m_animations->addAnimation("start", start_animation, m_use_duration / 10.f);
	thor::FrameAnimation fly_animation;
	fly_animation.addFrame(1, global_sprites["items_jetpack_6"].texture_rect, { 26, 0 });
	fly_animation.addFrame(1, global_sprites["items_jetpack_7"].texture_rect, { 28, 0 });
	m_animations->addAnimation("fly", fly_animation, m_use_duration / 12.5f);
	thor::FrameAnimation end_animation;
	end_animation.addFrame(1, global_sprites["items_jetpack_8"].texture_rect, { 14, 0 });
	end_animation.addFrame(1, global_sprites["items_jetpack_9"].texture_rect, { 13, 0 });
	end_animation.addFrame(1, global_sprites["items_jetpack_10"].texture_rect, { 12, 0 });
	m_animations->addAnimation("end", end_animation, m_use_duration / 10.f);
	thor::FrameAnimation ended_animation;
	ended_animation.addFrame(1, global_sprites["items_jetpack_0"].texture_rect, { 12, 36 });
	m_animations->addAnimation("ended", ended_animation, sf::seconds(1));
	m_animator.play() << thor::Playback::loop("default");
}

void Jetpack::update(sf::Time dt)
{
	m_existing_time += dt;
	if (m_tile)
	{
		setPosition(m_tile->getPosition() + m_tile_offset);
		if (m_tile->isFallenOffScreen() || m_tile->isDestroyed())
		{
			m_tile->setReadyToBeDeleted(true);
			m_tile = nullptr;
			m_after_use_horizontal_speed = 0;
			m_after_use_rotation_speed = 0;
		}
	}
	else if (m_doodle_manip.hasDoodle())
	{
		if (m_existing_time - m_use_start >= m_use_duration)
		{
			m_doodle_manip.setItem(nullptr);
			m_doodle_manip.setCanShoot(true);
			m_velocity = m_doodle_manip.getVelocity().y - 100;
			m_doodle_manip.setDoodle(nullptr);
			m_is_used = true;
			m_gravity += 400;
			m_animator.play() << thor::Playback::loop("ended");
			m_after_use_horizontal_speed = 100;
			m_after_use_rotation_speed = 90;
		}
		else
		{
			m_doodle_manip.setVelocity(sf::Vector2f{ m_doodle_manip.getVelocity().x, getDoodleSpeed((m_existing_time - m_use_start) / m_use_duration) });
			setScale(std::abs(getScale().x) * (m_doodle_manip.getBodyStatus() ? -1 : 1), getScale().y);
			m_body.setScale(getScale());
			setPosition(m_doodle_manip.getPosition() + thor::rotatedVector({ (getScale().x > 0 ? 1.f : -1.f) * m_doodle_offset.x, m_doodle_offset.y }, m_doodle_manip.getRotation()));
			m_body.setPosition(m_doodle_manip.getPosition() + thor::rotatedVector({ (getScale().x > 0 ? 1.f : -1.f) * m_body_doodle_offset.x, m_body_doodle_offset.y }, m_doodle_manip.getRotation()));
			setRotation(m_doodle_manip.getRotation());
			m_body.setRotation(getRotation());
		}
	}
	else
	{
		rotate((getScale().x > 0 ? -1 : 1) * m_after_use_rotation_speed * dt.asSeconds());
		move((getScale().x > 0 ? -1 : 1) * m_after_use_horizontal_speed * dt.asSeconds(), 0);
		updatePhysics(dt);
	}


	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_animator.update(dt);
	m_animator.animate(*this);
}

Jetpack::~Jetpack()
{
	if (m_doodle_manip.hasDoodle())
	{
		m_doodle_manip.setCanShoot(true);
		m_doodle_manip.setItem(nullptr);
	}
}

float SpringShoes::getCurrentCompressionWithoutClamp()
{
	if (!std::visit([](auto* current_platform) -> bool {return current_platform; }, m_current_platform)) return 1;
	return std::visit([this](auto* current_platform) { return (current_platform->getCollisionBox().top - getPosition().y) / (m_collision_box_size.y - m_shoes.getSize().y * m_texture_scale); }, m_current_platform);
}

float SpringShoes::getCurrentCompression()
{
	return std::clamp(getCurrentCompressionWithoutClamp(), 0.f, 1.f);
}

void SpringShoes::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_shoes, states);
	target.draw(sf::Sprite(*this), states);
}

SpringShoes::SpringShoes(Tile* tile, size_t max_use_count, Tiles* tiles, Monsters* monsters):
	Item(&global_sprites["items_spring_shoes_0"].getTexture()),
	m_shoes(global_sprites["items_spring_shoes_0"].getTexture()),
	m_max_use_count(max_use_count),
	m_tiles(tiles),
	m_monsters(monsters)
{
	m_tile = tile;
	m_tile_offset.y = -24;
	m_collision_box_size = sf::Vector2f{ 52, 38 } *m_texture_scale;
	setTextureRect(global_sprites["items_spring_shoes_0"].texture_rect);
	setOrigin(26, 0);
	setDoodleCollisionCallback([this](Doodle* doodle)
	{
		if (m_use_count) return;
		if (!doodle) return;
		if (doodle->hasShoes()) return;
		if (doodle->getVelocity().y < 0) return;
		if (!doodle->getFeetCollisionBox().intersects(getCollisionBox())) return;
		m_doodle_manip.setDoodle(doodle);
		m_doodle_manip.setHasShoes(true);
		m_doodle_manip.setDrawFeet(false);
		m_doodle_manip.setCanJump(false);
		if (m_tile)
		{
			m_tile->setReadyToBeDeleted(true);
			m_tile = nullptr;
		}
	}); 
	m_tile->setReadyToBeDeleted(false);
	setScale(m_texture_scale, m_texture_scale);
	m_shoes.setScale(getScale());
	m_shoes.addExhibit({ (sf::FloatRect)global_sprites["items_spring_shoes_1"].texture_rect, {26, 14} });
	m_shoes.addExhibit({ (sf::FloatRect)global_sprites["items_spring_shoes_2"].texture_rect, {24, 14} });
	m_shoes.set(1);
}

void SpringShoes::update(sf::Time dt)
{
	if (m_tile)
	{
		setPosition(m_tile->getPosition() + m_tile_offset);
		if (m_tile->isFallenOffScreen() || m_tile->isDestroyed())
		{
			m_tile->setReadyToBeDeleted(true);
			m_tile = nullptr;
			m_after_use_horizontal_speed = 0;
			m_after_use_rotation_speed = 0;
		}
	}
	else if (m_doodle_manip.hasDoodle())
	{
		if (m_use_count >= m_max_use_count || m_doodle_manip.isDead())
		{
			m_velocity = m_doodle_manip.getVelocity().y - 100;
			m_doodle_manip.setCanJump(true);
			m_doodle_manip.setDrawFeet(true);
			m_doodle_manip.setHasShoes(false);
			m_doodle_manip.setDoodle(nullptr);
			m_current_platform = (Tile*)nullptr;
			m_gravity += 400;
			m_after_use_horizontal_speed = 100;
			m_after_use_rotation_speed = 90;
		}
		else
		{
			setPosition(m_doodle_manip.getPosition() + m_doodle_offset);
			m_shoes.set((m_doodle_manip.getBodyStatus() == 2) ? 2 : 1);
			setScale(std::abs(getScale().x) * (m_doodle_manip.getBodyStatus() ? 1 : -1), getScale().y);
			setRotation(m_doodle_manip.getRotation());
			if (std::visit([](auto* current_platform) -> bool {return current_platform;}, m_current_platform))
			{
				float compr = getCurrentCompressionWithoutClamp();
				if ((compr <= 0 || std::holds_alternative<Monster*>(m_current_platform)) && m_doodle_manip.getVelocity().y > 0 && !m_jumping)
				{
					m_doodle_manip.setCanJump(true);
					m_doodle_manip.jumpWithSpeed(m_jumping_speed);
					m_doodle_manip.setCanJump(false);
					m_use_count++;
					m_jumping = true;
				}
				else if (compr > 0 && m_jumping) m_jumping = false;
				if (compr > 1) m_current_platform = (Tile*)nullptr;
			}
			else if (Tile* jump_tile; m_doodle_manip.getVelocity().y > 0 && (jump_tile = m_tiles->getTileDoodleWillJump(getCollisionBox()))) m_current_platform = jump_tile;
			else if (Monster* jump_monster; m_doodle_manip.getVelocity().y > 0 && (jump_monster = m_monsters->getMonsterDoodleWillJump(getCollisionBox()))) m_current_platform = jump_monster;
		}
	}
	else
	{
		rotate((getScale().x > 0 ? -1 : 1) * m_after_use_rotation_speed * dt.asSeconds());
		move((getScale().x > 0 ? -1 : 1) * m_after_use_horizontal_speed * dt.asSeconds(), 0);
		updatePhysics(dt);
		
	}
	m_collision_box = sf::FloatRect(getPosition() - sf::Vector2f{m_collision_box_size.x / 2.f, m_shoes.getSize().y * m_texture_scale}, m_collision_box_size);
	setScale(getScale().x, getCurrentCompression() * m_texture_scale);
	m_shoes.setPosition(getPosition());
	m_shoes.setRotation(getRotation());
	m_shoes.setScale(getScale().x, m_texture_scale);
}

SpringShoes::~SpringShoes()
{
	if (m_doodle_manip.hasDoodle()) 
	{
		m_doodle_manip.setCanJump(true);
		m_doodle_manip.setDrawFeet(true);
		m_doodle_manip.setHasShoes(false);
	}
}

Shield::Shield(Tile* tile):
	Item(&global_textures["items"])
{
	m_tile = tile;
	m_tile_offset.y = -40;
	m_collision_box_size = sf::Vector2f{ 66, 66 } *m_texture_scale;
	setDoodleCollisionCallback([this](Doodle* doodle)
		{
			if (!doodle) return;
			if (doodle->hasShield()) return;
			m_animator.play() << thor::Playback::loop("use");
			m_doodle_manip.setDoodle(doodle);
			m_doodle_manip.setShield(this);
			if (m_tile)
			{
				m_tile->setReadyToBeDeleted(true);
				m_tile = nullptr;
			}
			m_use_start = m_existing_time;
		});
	tile->setReadyToBeDeleted(false);
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, global_sprites["items_shield_0"].texture_rect, { 33, 33 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	thor::FrameAnimation use_animation;
	use_animation.addFrame(1, global_sprites["items_shield_1"].texture_rect, { 96, 96 });
	use_animation.addFrame(1, global_sprites["items_shield_2"].texture_rect, { 96, 96 });
	use_animation.addFrame(1, global_sprites["items_shield_3"].texture_rect, { 96, 96 });
	m_animations->addAnimation("use", use_animation, sf::seconds(0.2));
	m_animator.play() << thor::Playback::loop("default");
}

void Shield::update(sf::Time dt)
{
	m_existing_time += dt;
	if (m_tile)
	{
		setPosition(m_tile->getPosition() + m_tile_offset);
		if (m_tile->isFallenOffScreen() || m_tile->isDestroyed())
		{
			m_tile->setReadyToBeDeleted(true);
			m_tile = nullptr;
		}
	}
	else if (m_doodle_manip.hasDoodle())
	{
		if (m_existing_time - m_use_start >= m_use_duration)
		{
			m_doodle_manip.setShield(nullptr);
			m_doodle_manip.setDoodle(nullptr);
			m_is_destroyed = true;
		}
		else
		{
			if (m_existing_time - m_use_start >= m_end_start)
			{
				sf::Color col = getColor();
				col.a = thor::Distributions::uniform(0, 1)() * 255;
				setColor(col);
			}
			setPosition(m_doodle_manip.getPosition());
		}
	}
	else updatePhysics(dt);


	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_animator.update(dt);
	m_animator.animate(*this);	
}

bool Shield::isDestroyed() const
{
	if (!m_doodle_manip.hasDoodle()) return false;
	return m_is_destroyed;
}

Shield::~Shield()
{
	m_doodle_manip.setShield(nullptr);
	m_doodle_manip.setDoodle(nullptr);
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
