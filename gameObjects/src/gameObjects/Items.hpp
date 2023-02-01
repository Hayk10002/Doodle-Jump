#pragma once
#include <functional>
#include <string>

#include <SFML/Graphics.hpp>
#include <Thor/Animations.hpp>
#include <Thor/Math.hpp>

class Doodle;
class Items;
class Item : public sf::Sprite
{
	struct DoodleManipulator
	{
		Doodle* m_doodle;
		DoodleManipulator(Doodle* doodle);
		void setDoodle(Doodle* doodle);

		void jumpWithSpeed(float speed);
		void setRotation(float rotation);
		auto getBodyStatus();
		void setBodyStatus(int status);
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
	void updatePhysics(sf::Time dt);

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	friend class Items;
	friend class Doodle;
};

class Tile;
class Spring : public Item
{
	Tile* m_tile;
	sf::Vector2f m_offset{ 0, -14 };
	float m_jumping_speed{ 1200 };
public:
	Spring(Tile* tile);
	void setOffsetFromTile(float offset);
	void update(sf::Time dt) override;
};

class Trampoline : public Item
{
	Tile* m_tile;
	sf::Vector2f m_offset{ 0, -16 };
	float m_jumping_speed{ 1500 };
	sf::Time m_existing_time{}, m_doodle_rotation_start{}, m_doodle_rotation_duration{sf::seconds(1)};
	int m_is_doodle_in_rotation = 0;
	int m_doodle_body_status{};
	float getDoodleRotation(float progress);

public:
	Trampoline(Tile* tile);
	void setOffsetFromTile(float offset);
	void update(sf::Time dt) override;
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