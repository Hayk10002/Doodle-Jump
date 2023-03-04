#include "Monsters.hpp"

#include <Thor/Math.hpp>

#include <common/Resources.hpp>
#include <gameObjects/Doodle.hpp>

Monster::Monster() :Monster(nullptr)
{}

Monster::Monster(sf::Texture* texture_ptr) :
	m_animations(new thor::AnimationMap<sf::Sprite, std::string>()),
	m_animator(*m_animations)
{
	if (texture_ptr) setTexture(*texture_ptr);
	m_on_doodle_bump = [this](Doodle* doodle)
	{
		if (isDead()) return;
		doodle->dieHeadBump();
	};
}

sf::FloatRect Monster::getCollisionBox() const
{
	return m_collision_box;
}

sf::Vector2f Monster::getCollisionBoxSize() const
{
	return m_collision_box_size;
}

bool Monster::isDestroyed() const
{
	return m_is_destroyed;
}

bool Monster::getShooted(const Bullet& bullet)
{
	m_is_dead = true;
	m_is_destroyed = true;
	return true;
}

bool Monster::isFallenOffScreen() const
{
	return m_is_fallen_off_screen;
}

bool Monster::isDead() const
{
	return m_is_dead;
}

void Monster::updatePhysics(sf::Time dt)
{
	m_vert_velocity += m_gravity * dt.asSeconds();
	move(0, m_vert_velocity * dt.asSeconds());
}

void Monster::setSpecUpdate(SpecUpdateFunctionType spec_update)
{
	m_spec_update = spec_update;
}

void Monster::setDoodleJumpCallback(OnDoodleJumpFunctionType on_doodle_jump)
{
	m_on_doodle_jump = on_doodle_jump;
}

void Monster::setDoodleBumpCallback(OnDoodleBumpFunctionType on_doodle_bump)
{
	m_on_doodle_bump = on_doodle_bump;
}

void Monster::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(sf::Sprite(*this), states);
}

HorizontalMovingMonster::HorizontalMovingMonster(float speed) :
	Monster(&global_textures["monsters"]),
	m_speed(speed)
{
	m_collision_box_size = sf::Vector2f{ 74, 78 } *m_texture_scale;
	setTextureRect({ 0, 0, 74, 98 });
	setOrigin({ 37, 49 });
	setScale(m_texture_scale, m_texture_scale);
}

void HorizontalMovingMonster::update(sf::Time dt)
{
	m_existing_time += dt;
	if (m_collision_box == sf::FloatRect{}) m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_spec_update(dt);
	if (!isDead())
	{
		float step = m_speed * dt.asSeconds();
		if (m_right > m_left)
		{
			if (step >= 0 && m_collision_box.left + m_collision_box.width + step >= m_right) m_speed = -m_speed;
			if (step < 0 && m_collision_box.left + step <= m_left) m_speed = -m_speed;
		}
		move(m_speed * dt.asSeconds(), 0);
		if (m_speed < 0) setScale(-std::abs(getScale().x), getScale().y);
		if (m_speed >= 0) setScale(std::abs(getScale().x), getScale().y);

		sf::Vector2f m_new_offset{};
		m_new_offset.y = thor::TrigonometricTraits<float>::sin(m_existing_time / m_oscillation_time * 360) * m_oscillation_size;
		move(m_new_offset - m_curr_pos_offset);
		m_curr_pos_offset = m_new_offset;
	}
	else updatePhysics(dt);
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
}

void HorizontalMovingMonster::updateMovingLocation(sf::FloatRect area)
{
	m_left = area.left;
	m_right = area.left + area.width;
}

void HorizontalMovingMonster::updateMovingLocation(float left, float right)
{
	m_left = left;
	m_right = right;
}

VirusMonster::VirusMonster() :
	Monster(&global_textures["monsters"])
{
	m_collision_box_size = sf::Vector2f{76, 52} * m_texture_scale;
	setTextureRect({ 184, 0, 92, 72 });
	setOrigin({ 44, 35 });
	setScale(m_texture_scale, m_texture_scale);
}

void VirusMonster::update(sf::Time dt)
{
	m_existing_time += dt;
	if (!isDead())
	{
		sf::Vector2f m_new_offset{};
		m_new_offset.x = thor::TrigonometricTraits<float>::sin(m_existing_time / m_oscillation_time * 360) * m_oscillation_size;
		move(m_new_offset - m_curr_pos_offset);
		m_curr_pos_offset = m_new_offset;
	}
	else updatePhysics(dt);
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
}

SpiderMonster::SpiderMonster() :
	Monster(&global_textures["monsters"])
{
	m_collision_box_size = sf::Vector2f{ 70, 110 } *m_texture_scale;
	setTextureRect({ 74, 0, 110, 98 });
	setOrigin({ 55, 49 });
	setScale(m_texture_scale, m_texture_scale);
}

void SpiderMonster::update(sf::Time dt)
{
	m_existing_time += dt;
	if (!isDead())
	{
		sf::Vector2f m_new_offset{};
		m_new_offset.x = thor::TrigonometricTraits<float>::sin(m_existing_time / m_oscillation_time * 360) * m_oscillation_size;
		move(m_new_offset - m_curr_pos_offset);
		m_curr_pos_offset = m_new_offset;
	}
	else updatePhysics(dt);
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
}

BigBlueMonster::BigBlueMonster():
	Monster(&global_textures["monsters"])
{
	m_collision_box_size = sf::Vector2f{ 130, 106 } *m_texture_scale;
	setTextureRect({ 432, 0, 170, 106 });
	setOrigin(85, 106);
	setScale(m_texture_scale, m_texture_scale);
}

void BigBlueMonster::update(sf::Time dt)
{
	m_existing_time += dt;
	if (!isDead())
	{
		sf::Vector2f m_new_offset{};
		m_new_offset.x = (-4 * std::abs((m_existing_time / m_oscillation_time - int(m_existing_time / m_oscillation_time)) - 0.5) + 1) * m_oscillation_size.x / 2;
		m_new_offset.y = -(-std::pow(2 * (m_existing_time / (0.5f * m_oscillation_time) - int(m_existing_time / (0.5f * m_oscillation_time))) - 1, 2) + 1) * m_oscillation_size.y;
		move(m_new_offset - m_curr_pos_offset);
		m_curr_pos_offset = m_new_offset;
	}
	else updatePhysics(dt);
	m_collision_box = sf::FloatRect(getPosition() - sf::Vector2f{m_collision_box_size.x / 2.f, m_collision_box_size.y}, m_collision_box_size);
}

UFOMonster::UFOMonster() :
	Monster(&global_textures["monsters"]),
	m_light(global_textures["monsters"])
{
	m_collision_box_size = sf::Vector2f{ 160, 240 } *m_texture_scale;
	setTextureRect({ 276, 0, 156, 68 });
	setOrigin({ 78, 34 });
	setScale(m_texture_scale, m_texture_scale);
	m_light.setTextureRect({ 0, 98, 160, 200 });
	m_light.setOrigin(80, 0);
	m_light.setScale(m_texture_scale, m_texture_scale);
	setDoodleJumpCallback([this](sf::FloatRect doodle_feet)
		{
			if (!doodle_feet.intersects(m_UFO_collision_boxes[0]) && !doodle_feet.intersects(m_UFO_collision_boxes[1])) return false;
			if (isDead()) return false;
			m_is_dead = true;
			m_gravity = 1200;
			m_vert_velocity = 400;
			return true;
		}
	);
	setDoodleBumpCallback([this](Doodle* doodle)
	{
		if (isDead()) return;
		if (!doodle->getBodyCollisionBox().intersects(m_UFO_collision_boxes[0]) &&
			!doodle->getBodyCollisionBox().intersects(m_UFO_collision_boxes[1]) &&
			!doodle->getFeetCollisionBox().intersects(m_UFO_collision_boxes[0]) &&
			!doodle->getFeetCollisionBox().intersects(m_UFO_collision_boxes[1]) &&
			!doodle->getBodyCollisionBox().intersects(m_light_collision_box) &&
			!doodle->getFeetCollisionBox().intersects(m_light_collision_box)) return;
		if (doodle->getBodyCollisionBox().intersects(m_UFO_collision_boxes[0]) ||
			doodle->getBodyCollisionBox().intersects(m_UFO_collision_boxes[1]) ||
			doodle->getFeetCollisionBox().intersects(m_UFO_collision_boxes[0]) ||
			doodle->getFeetCollisionBox().intersects(m_UFO_collision_boxes[1]))
		{
			doodle->dieHeadBump();
			return;
		}
		if (doodle->getBodyCollisionBox().intersects(m_light_collision_box) ||
			doodle->getFeetCollisionBox().intersects(m_light_collision_box))
		{
			m_animation_speed = 0;
			doodle->dieShrink(getPosition());
		}
	});
}

void UFOMonster::update(sf::Time dt)
{
	m_existing_time += m_animation_speed * dt;
	if (!isDead())
	{
		sf::Vector2f m_new_offset{};
		m_new_offset.x = thor::TrigonometricTraits<float>::sin(m_existing_time / m_oscillation_time * 360) * m_oscillation_size.x;
		m_new_offset.y = thor::TrigonometricTraits<float>::sin(m_existing_time / m_oscillation_time * 720) * m_oscillation_size.y;
		move(m_new_offset - m_curr_pos_offset);
		m_curr_pos_offset = m_new_offset;
	}
	else updatePhysics(dt);
	m_collision_box = sf::FloatRect(getPosition() + m_collision_box_offset, m_collision_box_size);
	m_UFO_collision_boxes[0] = sf::FloatRect(getPosition() + m_UFO_collision_box_offsets[0], m_UFO_collision_box_sizes[0]);
	m_UFO_collision_boxes[1] = sf::FloatRect(getPosition() + m_UFO_collision_box_offsets[1], m_UFO_collision_box_sizes[1]);
	m_light_collision_box = sf::FloatRect(getPosition() + m_light_collision_box_offset, m_light_collision_box_size);
	m_light.setPosition(getPosition());
	m_light.setColor({ 255, 255, 255, sf::Uint8((thor::Distributions::uniform(0, 1)()) ? 255 : 128) });
}

bool UFOMonster::getShooted(const Bullet& bullet)
{
	if (bullet.getGlobalBounds().intersects(m_UFO_collision_boxes[0]) ||
		bullet.getGlobalBounds().intersects(m_UFO_collision_boxes[1]))
		return Monster::getShooted(bullet);
	return false;
}

void UFOMonster::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_light, states);
	target.draw(sf::Sprite(*this), states);
}

BlackHoleMonster::BlackHoleMonster() :
	Monster(&global_textures["monsters"])
{
	m_collision_box_size = sf::Vector2f{ 110, 90 } *m_texture_scale;
	setTextureRect({160, 98, 140, 130});
	setOrigin({ 70, 65 });
	setScale(m_texture_scale, m_texture_scale);
	setDoodleJumpCallback([](sf::FloatRect) {return false; });
	setDoodleBumpCallback([this](Doodle* doodle)
	{
		if (doodle->isDead()) return;
		m_doodle = doodle;
		doodle->dieShrink(getPosition());
	});
}

void BlackHoleMonster::update(sf::Time dt)
{
	m_collision_box = sf::FloatRect(getPosition() + m_collision_box_offset, m_collision_box_size);
	if (m_doodle && !m_doodle->isDead()) m_doodle = nullptr;
}

bool BlackHoleMonster::getShooted(const Bullet& bullet)
{
	return false;
}

void BlackHoleMonster::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(Sprite(*this), states);
	if (m_doodle) target.draw(*m_doodle, states);
}

GreenOvalMonster::GreenOvalMonster():
	Monster(&global_textures["monsters"]),
	m_animations(new thor::AnimationMap<sf::Sprite, std::string>()),
	m_animator(*m_animations)
{
	m_collision_box_size = sf::Vector2f{ 92, 168 } *m_texture_scale;
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 300, 106, 124, 168 }, { 62, 168 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	thor::FrameAnimation hurting_animation;
	hurting_animation.addFrame(1, { 424, 106, 124, 168 }, { 62, 168 });
	m_animations->addAnimation("hurting", hurting_animation, sf::seconds(0.1));
	m_animator.play() << thor::Playback::loop("default");
}

void GreenOvalMonster::update(sf::Time dt)
{
	m_existing_time += dt;
	if (!isDead())
	{
		sf::Vector2f m_new_offset{};
		m_new_offset.x = (-4 * std::abs((m_existing_time / m_oscillation_time - int(m_existing_time / m_oscillation_time)) - 0.5) + 1) * m_oscillation_size.x / 2;
		m_new_offset.y = -(-std::pow(2 * (m_existing_time / (0.5f * m_oscillation_time) - int(m_existing_time / (0.5f * m_oscillation_time))) - 1, 2) + 1) * m_oscillation_size.y;
		move(m_new_offset - m_curr_pos_offset);
		m_curr_pos_offset = m_new_offset;
	}
	else updatePhysics(dt);
	m_animator.update(dt);
	m_animator.animate(*this);
	m_collision_box = sf::FloatRect(getPosition() - sf::Vector2f{ m_collision_box_size.x / 2.f, m_collision_box_size.y }, m_collision_box_size);
}

bool GreenOvalMonster::getShooted(const Bullet& bullet)
{
	m_hp--;
	if (m_hp > 0) m_animator.play() << "hurting" << thor::Playback::loop("default");
	else Monster::getShooted(bullet);
	return true;
}

FlatGreenMonster::FlatGreenMonster() :
	Monster(&global_textures["monsters"]),
	m_animations(new thor::AnimationMap<sf::Sprite, std::string>()),
	m_animator(*m_animations)
{
	m_collision_box_size = sf::Vector2f{ 182, 48 } *m_texture_scale;
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 602, 0, 182, 62 }, { 91, 62 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	thor::FrameAnimation hurting_animation;
	hurting_animation.addFrame(1, { 602, 62, 182, 62 }, { 91, 62 });
	m_animations->addAnimation("hurting", hurting_animation, sf::seconds(0.1));
	m_animator.play() << thor::Playback::loop("default");
}

void FlatGreenMonster::update(sf::Time dt)
{
	m_existing_time += dt;
	if (!isDead())
	{
		sf::Vector2f m_new_offset{};
		m_new_offset.y = -(-std::pow(2 * (m_existing_time / (0.5f * m_oscillation_time) - int(m_existing_time / (0.5f * m_oscillation_time))) - 1, 2) + 1) * m_oscillation_size;
		move(m_new_offset - m_curr_pos_offset);
		m_curr_pos_offset = m_new_offset;
	}
	else updatePhysics(dt);
	m_animator.update(dt);
	m_animator.animate(*this);
	m_collision_box = sf::FloatRect(getPosition() - sf::Vector2f{ m_collision_box_size.x / 2.f, m_collision_box_size.y }, m_collision_box_size);
}

bool FlatGreenMonster::getShooted(const Bullet& bullet)
{
	m_hp--;
	if (m_hp > 0) m_animator.play() << "hurting" << thor::Playback::loop("default");
	else Monster::getShooted(bullet);
	return true;
}

BigGreenMonster::BigGreenMonster() :
	Monster(&global_textures["monsters"]),
	m_animations(new thor::AnimationMap<sf::Sprite, std::string>()),
	m_animator(*m_animations)
{
	m_collision_box_size = sf::Vector2f{ 150, 96 } *m_texture_scale;
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 0, 298, 162, 102 }, { 81, 102 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	thor::FrameAnimation hurting_animation;
	hurting_animation.addFrame(1, { 162, 298, 162, 102 }, { 81, 102 });
	hurting_animation.addFrame(1, { 324, 298, 162, 116 }, { 81, 102 });
	m_animations->addAnimation("hurting", hurting_animation, sf::seconds(0.3));
	thor::FrameAnimation dead_animation;
	dead_animation.addFrame(1, { 162, 298, 162, 102 }, { 81, 102 });
	dead_animation.addFrame(1, { 486, 298, 162, 120 }, { 81, 102 });
	dead_animation.addFrame(1, { 648, 298, 162, 120 }, { 81, 102 });
	m_animations->addAnimation("dead", dead_animation, sf::seconds(0.3));
	m_animator.play() << thor::Playback::loop("default");
}

void BigGreenMonster::update(sf::Time dt)
{
	m_existing_time += dt;
	if(isDead()) updatePhysics(dt);
	m_animator.update(dt);
	m_animator.animate(*this);
	m_collision_box = sf::FloatRect(getPosition() - sf::Vector2f{ m_collision_box_size.x / 2.f, m_collision_box_size.y }, m_collision_box_size);
}

bool BigGreenMonster::getShooted(const Bullet& bullet)
{
	m_hp--;
	if (m_hp > 0) m_animator.play() << thor::Playback::loop("hurting");
	else if (!m_is_dead)
	{
		m_animator.play() << "dead" << thor::Playback::notify([this]() {m_is_destroyed = true; });
		m_is_dead = true;
	}
	return true;
}

BlueWingedMonster::BlueWingedMonster() :
	Monster(&global_textures["monsters"]),
	m_animations(new thor::AnimationMap<sf::Sprite, std::string>()),
	m_animator(*m_animations)
{
	m_collision_box_size = sf::Vector2f{ 156, 68 } *m_texture_scale;
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 0, 420, 156, 88 }, { 78, 44 });
	default_animation.addFrame(1, { 156, 420, 156, 88 }, { 78, 44 });
	default_animation.addFrame(1, { 312, 420, 156, 88 }, { 78, 44 });
	default_animation.addFrame(1, { 468, 420, 156, 88 }, { 78, 44 });
	default_animation.addFrame(1, { 624, 420, 156, 88 }, { 78, 44 });
	default_animation.addFrame(1, { 468, 420, 156, 88 }, { 78, 44 });
	default_animation.addFrame(1, { 312, 420, 156, 88 }, { 78, 44 });
	default_animation.addFrame(1, { 156, 420, 156, 88 }, { 78, 44 });
	m_animations->addAnimation("default", default_animation, sf::seconds(0.1));
	m_animator.play() << thor::Playback::loop("default");
}

void BlueWingedMonster::update(sf::Time dt)
{
	m_existing_time += dt;
	if (!isDead())
	{
		sf::Vector2f m_new_offset{};
		m_new_offset.x = thor::TrigonometricTraits<float>::sin(m_existing_time / m_oscillation_time * 360) * m_oscillation_size;
		move(m_new_offset - m_curr_pos_offset);
		m_curr_pos_offset = m_new_offset;
	}
	else updatePhysics(dt);
	m_animator.update(dt);
	m_animator.animate(*this);
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);

}

TheTerrifyMonster::TheTerrifyMonster(sf::Vector2f speed) :
	Monster(&global_textures["monsters"]),
	m_animations(new thor::AnimationMap<sf::Sprite, std::string>()),
	m_animator(*m_animations),
	m_speed(speed)
{
	m_collision_box_size = sf::Vector2f{ 86, 174 } *m_texture_scale;
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 126, 510, 126, 174 }, { 63, 87 });
	default_animation.addFrame(1, { 252, 510, 126, 174 }, { 63, 87 });
	default_animation.addFrame(1, { 378, 510, 126, 174 }, { 63, 87 });
	default_animation.addFrame(1, { 504, 510, 134, 174 }, { 67, 87 });
	default_animation.addFrame(1, { 378, 510, 126, 174 }, { 63, 87 });
	default_animation.addFrame(1, { 252, 510, 126, 174 }, { 63, 87 });
	m_animations->addAnimation("default", default_animation, sf::seconds(0.3)); 
	thor::FrameAnimation hurting_animation;
	hurting_animation.addFrame(1, { 0, 510, 126, 174 }, { 63, 87 });
	m_animations->addAnimation("hurting", hurting_animation, sf::seconds(0.1));
	m_animator.play() << thor::Playback::loop("default");
}

void TheTerrifyMonster::update(sf::Time dt)
{
	if (m_collision_box == sf::FloatRect{}) m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_spec_update(dt);
	if (!isDead())
	{
		float step = m_speed.x * dt.asSeconds();
		if (m_right > m_left)
		{
			if (step >= 0 && m_collision_box.left + m_collision_box.width + step >= m_right) m_speed.x = -m_speed.x;
			if (step < 0 && m_collision_box.left + step <= m_left) m_speed.x = -m_speed.x;
		}
		move(m_speed * dt.asSeconds());
	}
	else updatePhysics(dt);
	m_animator.update(dt);
	m_animator.animate(*this);
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
}

bool TheTerrifyMonster::getShooted(const Bullet& bullet)
{
	m_hp--;
	if (m_hp > 0) m_animator.play() << "hurting" << thor::Playback::loop("default");
	else Monster::getShooted(bullet);
	return true;
}

void TheTerrifyMonster::updateMovingLocation(sf::FloatRect area)
{
	m_left = area.left;
	m_right = area.left + area.width;
}

void TheTerrifyMonster::updateMovingLocation(float left, float right)
{
	m_left = left;
	m_right = right;
}

Monsters::Monsters(sf::RenderWindow& game_window):
	m_game_window(game_window)
{
}

void Monsters::update(sf::Time dt)
{
	for (auto& monster : m_monsters)
	{
		monster->update(dt);
		if (sf::FloatRect area = utils::getViewArea(m_game_window); monster->getCollisionBox().top > area.top + area.height)
			monster->m_is_fallen_off_screen = true;
	}

	for (size_t i = 0; i < m_monsters.size(); i++)
		if (sf::FloatRect area = utils::getViewArea(m_game_window); m_monsters[i]->isDestroyed() || m_monsters[i]->getCollisionBox().top > area.top + area.height)
			m_monsters.erase(m_monsters.begin() + i--);
}

Monster* Monsters::getMonsterDoodleWillJump(sf::FloatRect doodle_feet)
{
	Monster* res_monster{ nullptr };
	sf::FloatRect feet{ doodle_feet.left, doodle_feet.top + doodle_feet.height - 1, doodle_feet.width, 1 };
	for (const auto& monster : m_monsters) if (monster->getCollisionBox().intersects(feet)) res_monster = (monster->m_on_doodle_jump(doodle_feet) ? monster.get() : nullptr);
	return res_monster;
}

bool Monsters::willDoodleJump(sf::FloatRect doodle_feet)
{
	return getMonsterDoodleWillJump(doodle_feet);
}

bool Monsters::updateBullet(const Bullet& bullet)
{
	bool res = false;
	for (auto& monster : m_monsters) if (monster->getCollisionBox().intersects(bullet.getGlobalBounds())) res |= monster->getShooted(bullet);
	return res;
}

void Monsters::updateDoodleBump(Doodle* doodle)
{
	for (auto& monster : m_monsters)
		if (monster->getCollisionBox().intersects(doodle->getFeetCollisionBox())
			|| monster->getCollisionBox().intersects(doodle->getBodyCollisionBox()))
			monster->m_on_doodle_bump(doodle);
}

size_t Monsters::getMonstersCount()
{
	return m_monsters.size();
}

void Monsters::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (const auto& monster : m_monsters)
	{
		target.draw(*monster, states);
		/*sf::RectangleShape sh{ {monster->getCollisionBox().width, monster->getCollisionBox().height} };
		sh.setPosition(monster->getCollisionBox().left, monster->getCollisionBox().top);
		sh.setFillColor(sf::Color(255, 0, 0, 100));
		target.draw(sh, states);*/
	}
}
