#pragma once
#include <string>
#include <deque>
#include <functional>

#include <SFML/Graphics.hpp>
#include <Thor/Animations.hpp>

#include <common/Utils.hpp>

class Monsters;
class Doodle;
class Bullet;
class Monster : public sf::Sprite
{
public:
	using SpecUpdateFunctionType = std::function<void(sf::Time)>;
	using OnDoodleJumpFunctionType = std::function<bool(sf::FloatRect)>;
	using OnDoodleBumpFunctionType = std::function<void(Doodle*)>;
	
	Monster();
	Monster(sf::Texture* texture_ptr);
	Monster(const Monster&) = default;
	Monster(Monster&&) = default;
	Monster& operator=(const Monster&) = default;
	Monster& operator=(Monster&&) = default;

	sf::FloatRect getCollisionBox() const;
	sf::Vector2f getCollisionBoxSize() const;
	bool isDestroyed() const;
	virtual bool getShooted(const Bullet& bullet);
	bool isFallenOffScreen() const;
	bool isDead() const;
	virtual void update(sf::Time) = 0;
	void updatePhysics(sf::Time dt);
	void setSpecUpdate(SpecUpdateFunctionType spec_update);
	void setDoodleJumpCallback(OnDoodleJumpFunctionType on_doodle_jump);
	void setDoodleBumpCallback(OnDoodleBumpFunctionType on_doodle_bump);

	virtual ~Monster() = default;

protected:
	std::shared_ptr<thor::AnimationMap<sf::Sprite, std::string>> m_animations;
	thor::Animator<sf::Sprite, std::string> m_animator;
	sf::FloatRect m_collision_box{};
	sf::Vector2f m_collision_box_size{};
	float m_texture_scale{ 0.65 };
	float m_gravity{ 0 };
	float m_vert_velocity{ 0 };
	sf::Time m_existing_time{};
	sf::Vector2f m_curr_pos_offset{};
	SpecUpdateFunctionType m_spec_update{ [](sf::Time) {} };
	OnDoodleJumpFunctionType m_on_doodle_jump{ [this](sf::FloatRect) 
	{
			if (isDead()) return false;
			m_is_dead = true; 
			m_gravity = 1200; 
			m_vert_velocity = 400;
			return true; 
	} };
	OnDoodleBumpFunctionType m_on_doodle_bump;
	bool m_is_dead{ 0 };
	bool m_is_destroyed{ 0 };

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	bool m_is_fallen_off_screen{ 0 };

	friend class Monsters;
};

class BlueOneEyedMonster : public Monster
{
	float m_speed;
	float m_left{ 0 }, m_right{ -1 };
	sf::Time m_oscillation_time{ sf::seconds(0.7) };
	float m_oscillation_size{ 15 };
public:
	BlueOneEyedMonster(float speed);
	void update(sf::Time dt) override;
	void updateMovingLocation(sf::FloatRect area);
	void updateMovingLocation(float left, float right);
};

class CamronMonster : public Monster
{
	sf::Time m_oscillation_time{ sf::seconds(0.5) };
	float m_oscillation_size{ 15 };
public:
	CamronMonster();
	void update(sf::Time dt) override;
};

class PurpleSpiderMonster : public Monster
{
	sf::Time m_oscillation_time{ sf::seconds(0.5) };
	float m_oscillation_size{ 15 };
public:
	PurpleSpiderMonster();
	void update(sf::Time dt) override;
};

class LargeBlueMonster : public Monster
{
	sf::Time m_oscillation_time{ sf::seconds(1) };
	sf::Vector2f m_oscillation_size{ 15, 15 };
public:
	LargeBlueMonster();
	void update(sf::Time dt) override;
};

class UFO : public Monster
{
	sf::Time m_oscillation_time{ sf::seconds(4) };
	sf::Vector2f m_oscillation_size{ 40, 20 };
	float m_animation_speed = 1;
	sf::Vector2f m_UFO_collision_box_sizes[2]
	{
		sf::Vector2f{156, 36} * m_texture_scale,
		sf::Vector2f{76, 38} * m_texture_scale 
	};
	sf::Vector2f m_light_collision_box_size{ sf::Vector2f{120, 200} *m_texture_scale };
	sf::Vector2f m_collision_box_offset{ sf::Vector2f{-80, -34} *m_texture_scale };
	sf::Vector2f m_UFO_collision_box_offsets[2]
	{
		sf::Vector2f{-78, -2} * m_texture_scale,
		sf::Vector2f{-38, -34} * m_texture_scale
	};
	sf::Vector2f m_light_collision_box_offset{ sf::Vector2f{-60, 0} *m_texture_scale };
	sf::FloatRect m_UFO_collision_boxes[2];
	sf::FloatRect m_light_collision_box;
	sf::Sprite m_light;
public:
	UFO();
	void update(sf::Time dt) override;
	bool getShooted(const Bullet& bullet) override;

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

class BlackHole : public Monster
{
	sf::Vector2f m_collision_box_offset{ sf::Vector2f{-55, -52}*m_texture_scale };
	Doodle* m_doodle{ nullptr };
public:
	BlackHole();
	void update(sf::Time dt) override;
	bool getShooted(const Bullet& bullet) override;

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

class OvalGreenMonster: public Monster
{
	sf::Time m_oscillation_time{ sf::seconds(1) };
	sf::Vector2f m_oscillation_size{ 15, 15 };
	int m_hp = 2;

public:
	OvalGreenMonster();
	void update(sf::Time dt) override;
	bool getShooted(const Bullet& bullet) override;
};

class FlatGreenMonster: public Monster
{
	sf::Time m_oscillation_time{ sf::seconds(1) };
	float m_oscillation_size{ 15 };
	int m_hp = 3;

public:
	FlatGreenMonster();
	void update(sf::Time dt) override;
	bool getShooted(const Bullet& bullet) override;
};

class LargeGreenMonster : public Monster
{
	int m_hp = 2;

public:
	LargeGreenMonster();
	void update(sf::Time dt) override;
	bool getShooted(const Bullet& bullet) override;
};

class BlueWingedMonster: public Monster
{
	sf::Time m_oscillation_time{ sf::seconds(0.5) };
	float m_oscillation_size{ 15 };

public:
	BlueWingedMonster();
	void update(sf::Time dt) override;
};

class TheTerrifyingMonster : public Monster
{
	sf::Vector2f m_speed;
	float m_left{ 0 }, m_right{ -1 };

	int m_hp = 5;
public:
	TheTerrifyingMonster(sf::Vector2f speed);
	void update(sf::Time dt) override;
	bool getShooted(const Bullet& bullet) override;

	void updateMovingLocation(sf::FloatRect area);
	void updateMovingLocation(float left, float right);
};

class Level;
class LevelGenerator;
class Monsters : public sf::Drawable
{
private:
	std::deque<std::unique_ptr<Monster>> m_monsters;
	sf::RenderWindow& m_game_window;

public:
	Monsters(sf::RenderWindow& game_window);
	void update(sf::Time dt);
	Monster* getMonsterDoodleWillJump(sf::FloatRect doodle_feet);
	bool willDoodleJump(sf::FloatRect doodle_feet);
	bool updateBullet(const Bullet& bullet);
	void updateDoodleBump(Doodle* doodle);
	size_t getMonstersCount();

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	friend class Level;
	friend class LevelGenerator;

};