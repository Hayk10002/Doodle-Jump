#pragma once
#include <functional>
#include <string>

#include <SFML/Graphics.hpp>
#include <Thor/Animations.hpp>
#include <Thor/Math.hpp>
#include <Selbaward.hpp>

class Doodle;
class Items;
class Tile;
class Item : public sf::Sprite
{
	struct DoodleManipulator
	{
		Doodle* m_doodle;
		DoodleManipulator(Doodle* doodle);
		void setDoodle(Doodle* doodle);
		bool hasDoodle() const;

		void jumpWithSpeed(float speed);
		float getRotation() const;
		void setRotation(float rotation);
		sf::Vector2f getPosition() const;
		sf::Vector2f getVelocity() const;
		void setVelocity(sf::Vector2f velocity);
		void move(sf::Vector2f offset);
		auto getBodyStatus() const;
		void setBodyStatus(int status);
		bool hasItem() const;
		void setItem(sf::Drawable* item);
		bool hasShoes() const;
		void setHasShoes(bool val);
		bool canShoot() const;
		void setCanShoot(bool val);
		bool drawFeet() const;
		void setDrawFeet(bool val);
		bool canJump() const;
		void setCanJump(bool val);
		//...
	};
public:
	Item();
	Item(sf::Texture* texture_ptr);
	Item(const Item&) = default;
	Item(Item&&) = default;
	Item& operator=(const Item&) = default;
	Item& operator=(Item&&) = default;

	sf::FloatRect getCollisionBox() const;
	sf::Vector2f getCollisionBoxSize() const;
	virtual bool isDestroyed() const;
	bool isReadyToBeDeleted() const;
	virtual void update(sf::Time) = 0;
	void setDoodleCollisionCallback(std::function<void(Doodle*)> on_doodle_collision);
	void setOffsetFromTile(float offset);

	virtual ~Item() = default;

protected:
	DoodleManipulator m_doodle_manip{nullptr};
	std::shared_ptr<thor::AnimationMap<sf::Sprite, std::string>> m_animations;
	thor::Animator<sf::Sprite, std::string> m_animator;
	sf::FloatRect m_collision_box{};
	float m_texture_scale{ 0.65 };
	sf::Vector2f m_collision_box_size;
	bool m_is_ready_to_be_deleted{ true };
	std::function<void(Doodle*)> m_on_doodle_collision{ [](Doodle*) {} };
	float m_velocity{}, m_gravity{ 1200 };
	Tile* m_tile{};
	sf::Vector2f m_tile_offset{0, 0};
	void updatePhysics(sf::Time dt);

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	friend class Items;
	friend class Doodle;
};

class Spring : public Item
{
	float m_jumping_speed{ 1200 };
public:
	Spring(Tile* tile);
	void update(sf::Time dt) override;
};

class Trampoline : public Item
{
	float m_jumping_speed{ 1500 };
	sf::Time m_existing_time{}, m_doodle_rotation_start{}, m_doodle_rotation_duration{sf::seconds(1)};
	int m_is_doodle_in_rotation = 0;
	int m_doodle_body_status{};
	float getDoodleRotation(float progress);

public:
	Trampoline(Tile* tile);
	void update(sf::Time dt) override;
};

class PropellerHat : public Item
{
	sf::Vector2f m_doodle_offset{ 0, -30 };
	sf::Time m_existing_time{}, m_use_start{}, m_use_duration{ sf::seconds(3) };
	float m_max_speed{ 1000 };
	bool m_is_used{ false };
	float m_after_use_rotation_speed{ 90 };
	float m_after_use_horizontal_speed{ 100 };
	float getDoodleSpeed(float progress);

public:
	PropellerHat(Tile* tile);
	PropellerHat(const PropellerHat&) = default;
	PropellerHat(PropellerHat&&) = default;
	PropellerHat& operator=(const PropellerHat&) = default;
	PropellerHat& operator=(PropellerHat&&) = default;
	void update(sf::Time dt) override;
	~PropellerHat();
};

class Jetpack : public Item
{
	sf::Sprite m_body;
	sf::Vector2f m_doodle_offset{ -29, 27 };
	sf::Vector2f m_body_doodle_offset{-28, 4};
	sf::Time m_existing_time{}, m_use_start{}, m_use_duration{ sf::seconds(4) };
	float m_max_speed{ 3000 };
	bool m_is_used{ false };
	float m_after_use_rotation_speed{ 90 };
	float m_after_use_horizontal_speed{ 100 };
	float getDoodleSpeed(float progress);

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

public:
	Jetpack(Tile* tile);
	Jetpack(const Jetpack&) = default;
	Jetpack(Jetpack&&) = default;
	Jetpack& operator=(const Jetpack&) = default;
	Jetpack& operator=(Jetpack&&) = default;
	void update(sf::Time dt) override;
	~Jetpack();
};

class Tiles;
class SpringShoes : public Item
{
	sw::GallerySprite m_shoes;
	sf::Vector2f m_doodle_offset{ 0, 34 };
	Tile* m_current_tile{ nullptr };
	size_t m_max_use_count, m_use_count{ 0 };
	Tiles* m_tiles;
	float m_jumping_speed{ 1200 };
	bool m_jumping{ false };
	float m_after_use_rotation_speed{ 90 };
	float m_after_use_horizontal_speed{ 100 };
	float getCurrentCompressionWithoutClamp();
	float getCurrentCompression();

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

public:
	SpringShoes(Tile* tile, size_t max_use_count, Tiles* tiles);
	SpringShoes(const SpringShoes&) = default;
	SpringShoes(SpringShoes&&) = default;
	SpringShoes& operator=(const SpringShoes&) = default;
	SpringShoes& operator=(SpringShoes&&) = default;
	void update(sf::Time dt) override;
	~SpringShoes();
};


class Items : public sf::Drawable
{
private:
	std::deque<std::unique_ptr<Item>> m_items;
	sf::RenderWindow& m_game_window;

public:
	Items(sf::RenderWindow& game_window);
	void update(sf::Time dt);
	void updateDoodleCollisions(Doodle* doodle);
	size_t getItemsCount();

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	friend class LevelGenerator;
};