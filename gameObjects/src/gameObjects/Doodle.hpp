#pragma once
#include <string>
#include <map>
#include <functional>

#include <SFML/Graphics.hpp>
#include <Selbaward.hpp>

#include <common/Resources.hpp>

#include <gameObjects/Items.hpp>

class Tiles;
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
	sf::FloatRect getArea();
	void setTexture(sf::Texture* texture_ptr);
	bool isJumping();
	bool isShooting();
	bool isFallenOutOfScreen();
	bool isTooHigh();

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


	enum BodyStatus
	{
		Right,
		Left,
		Up
	};


	sf::Vector2f m_velocity{0, 0};
	sf::Vector2f m_gravity{0, 1200};
	sf::Texture* m_texture{ &global_textures["doodle"] };
	mutable sw::GallerySprite m_body{ *m_texture };
	size_t m_body_normal_exhind, m_body_shooting_exhind;
	mutable sw::GallerySprite m_feet{ *m_texture };
	size_t m_feet_normal_exhind, m_feet_shooting_exhind;
	mutable sw::GallerySprite m_nose{ *m_texture };
	size_t m_nose_exhind;
	BodyStatus m_body_status{ Right };
	sf::Vector2f m_body_collision_box_size{ 300, 350 };
	sf::FloatRect m_body_collision_box{};
	bool m_is_jumping{ 0 };
	sf::Clock m_jumping_clock;
	sf::Time m_jumping_interval{ sf::seconds(0.3) };
	bool m_is_shooting{ 0 };
	sf::Clock m_shooting_clock;
	sf::Time m_shooting_interval{ sf::seconds(0.5) };
	float m_nose_angle{ 90 };
	float m_max_nose_angle_dev{ 70 };
	sf::Vector2f m_nose_rotation_center{ 0, -42 }, m_nose_not_shooting_offset{202, 0};
	float m_nose_distance_from_rotation_center{190};
	float m_jumping_speed{700};
	float m_moving_speed{500};
	float m_speed_change_rate{m_moving_speed * 2};
	float m_speed_decreasing_rate{ 10 };
	sf::Vector2f m_texture_scale{ 0.15, 0.15 };
	sf::Vector2f m_feet_offset{0, 195}, m_feet_jumping_offset{0, -25};
	sf::Vector2f m_feet_collision_box_size{ 270, 60 };
	sf::FloatRect m_feet_collision_box{};
	sf::FloatRect m_area{};
	bool m_is_fallen_out{ 0 }, m_is_too_high{ 0 };
	mutable bool m_is_updating_for_drawing_needed{ false };

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	//friend class DoodleWithStats;
	friend class Item::DoodleManipulator;
};