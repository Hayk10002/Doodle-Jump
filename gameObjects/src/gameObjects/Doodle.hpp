#pragma once
#include <string>
#include <map>
#include <functional>
#include <deque>

#include <SFML/Graphics.hpp>
#include <Selbaward.hpp>

#include <common/Resources.hpp>

#include <gameObjects/Items.hpp>


class Bullet : public sf::Sprite
{
	sf::Vector2f m_velocity, m_gravity;
public:
	Bullet(const sf::Sprite& sprite);
	void setVelocity(sf::Vector2f velocity);
	void setGravity(sf::Vector2f gravity);
	sf::Vector2f getVelocity() const;
	sf::Vector2f getGravity() const;

	void update(sf::Time dt);

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

};

class Tiles;
class Monsters;
//class DoodleWithStats;
class Doodle : public sf::Drawable, public sf::Transformable
{
public:

	Doodle(sf::Vector2f starting_pos);

	void update(sf::Time dt);
	void left(sf::Time dt);
	void right(sf::Time dt);
	void shoot(float angle);
	void updateArea(sf::FloatRect area);
	void updateTiles(Tiles& tiles);
	void updateItems(Items& items);
	void updateMonsters(Monsters& monsters);
	void setTexture(sf::Texture* texture_ptr);

	void dieHeadBump();
	void dieShrink(sf::Vector2f shrinking_pos, sf::Time duration = sf::seconds(0.5), float rotation_speed = 0);
	void ressurrect(sf::Vector2f position);

	bool isJumping() const;
	bool isShooting() const;
	bool isFallenOutOfScreen() const;
	bool isTooHigh() const;
	bool hasItem() const;
	bool hasShoes() const;
	bool canShoot() const;
	bool canJump() const;
	bool drawFeet() const;
	bool hasShield() const;
	bool isDead() const;
	bool isCompletelyDead() const;


	sf::FloatRect getArea() const;
	sf::Vector2f getVelocity() const;
	sf::Vector2f getGravity() const;
	float getJumpingSpeed() const;
	sf::FloatRect getFeetCollisionBox() const;
	sf::FloatRect getBodyCollisionBox() const;
	void updateForDrawing() const;

private:

	void jump();
	void setVelocity(sf::Vector2f velocity);
	void setGravity(sf::Vector2f gravity);
	void setJumpingSpeed(float speed);
	void setItem(sf::Drawable* item);
	void setHasShoes(bool val);
	void setCanShoot(bool val);
	void setCanJump(bool val);
	void setDrawFeet(bool val);
	void setShield(Shield* shield);


	enum BodyStatus
	{
		Right,
		Left,
		Up
	};


	sf::Vector2f m_velocity{0, 0};
	sf::Vector2f m_gravity{0, 1200};
	sf::Time m_existing_time{};
	sf::Texture* m_texture{ &global_textures["doodle"] };
	mutable sw::GallerySprite m_body{ *m_texture };
	size_t m_body_normal_exhind, m_body_shooting_exhind;
	mutable sw::GallerySprite m_feet{ *m_texture };
	size_t m_feet_normal_exhind, m_feet_shooting_exhind;
	mutable sw::GallerySprite m_nose{ *m_texture };
	size_t m_nose_exhind;
	BodyStatus m_body_status{ Right };
	sf::Vector2f m_body_collision_box_size{ 60, 70 };
	sf::FloatRect m_body_collision_box{};
	bool m_is_jumping{ 0 };
	sf::Time m_jumping_interval{ sf::seconds(0.3) }, m_jumping_start{};
	bool m_is_shooting{ 0 };
	sf::Clock m_shooting_clock;
	sf::Time m_shooting_interval{ sf::seconds(0.5) }, m_shooting_start{};
	float m_nose_angle{ 90 };
	float m_max_nose_angle_dev{ 70 };
	sf::Vector2f m_nose_rotation_center{ 0, -8.4 }, m_nose_not_shooting_offset{40.4, 0};
	float m_nose_distance_from_rotation_center{38};
	float m_jumping_speed{700};
	float m_moving_speed{500};
	float m_speed_change_rate{m_moving_speed * 2};
	float m_speed_decreasing_rate{ 10 };
	sf::Vector2f m_texture_scale{ 0.75, 0.75 };
	sf::Vector2f m_current_texture_scale{ m_texture_scale };
	sf::Vector2f m_feet_offset{0, 39}, m_feet_jumping_offset{0, -5};
	sf::Vector2f m_feet_collision_box_size{ 54, 12 };
	sf::FloatRect m_feet_collision_box{};
	bool m_draw_feet{ true };
	sf::FloatRect m_area{};
	bool m_is_fallen_out{ false }, m_is_too_high{ false };
	bool m_can_shoot{ true }, m_can_jump{ true };
	sf::Drawable* m_item{ nullptr };
	bool m_has_shoes{ false };
	mutable bool m_is_updating_for_drawing_needed{ false };

	bool m_is_dead{ false };

	sf::Vector2f m_head_bump_stars_offset{0, -30};
	sf::Vector2f m_head_bump_stars_rotating_boundaries{ 48, 25 };
	sf::Time m_head_bump_stars_rotation_time{ sf::seconds(0.5) };
	mutable sf::Sprite m_head_bump_star{ *m_texture, {176, 76, 24, 24} };
	void animateHeadBumpStars(size_t index) const;

	bool m_is_shrinking{ false };
	sf::Vector2f m_shrinking_pos{}, m_shrinking_start_pos;
	sf::Time m_shrinking_duration{}, m_shrinking_start{};
	float m_shrinking_rotation_speed{};

	void die();

	std::deque<Bullet> m_bullets;
	sf::Sprite m_bullet_sprite{ *m_texture };
	float m_bullet_speed = 2000;

	Shield* m_shield{ nullptr };

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	//friend class DoodleWithStats;
	friend class Item::DoodleManipulator;
};

