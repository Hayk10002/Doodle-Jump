#pragma once
#include <string>
#include <deque>
#include <functional>

#include <SFML/Graphics.hpp>
#include <Thor/Animations.hpp>

class Tiles;
class Tile : public sf::Sprite
{
public:

	using SpecUpdateFunctionType = std::function<void(sf::Time)>;
	using OnDoodleJumpFunctionType = std::function<bool()>;

	Tile();
	Tile(sf::Texture* texture_ptr);
	Tile(const Tile&) = default;
	Tile(Tile&&) = default;
	Tile& operator=(const Tile&) = default;
	Tile& operator=(Tile&&) = default;

	sf::FloatRect getCollisionBox() const;
	sf::Vector2f getCollisionBoxSize() const;
	virtual bool isDestroyed() const;
	bool isReadyToBeDeleted() const;
	bool isFallenOffScreen() const;
	bool canCollide() const;
	void setReadyToBeDeleted(bool is_ready);
	virtual void update(sf::Time) = 0;
	void setSpecUpdate(SpecUpdateFunctionType spec_update);
	void setDoodleJumpCallback(OnDoodleJumpFunctionType on_doodle_jump);

	virtual ~Tile() = default;

protected:
	std::shared_ptr<thor::AnimationMap<sf::Sprite, std::string>> m_animations;
	thor::Animator<sf::Sprite, std::string> m_animator;
	sf::FloatRect m_collision_box{};
	sf::Vector2f m_collision_box_size{};
	float m_texture_scale{ 0.65 };
	SpecUpdateFunctionType m_spec_update{ [](sf::Time) {} };
	OnDoodleJumpFunctionType m_on_doodle_jump{ []() {return true; } };
	bool m_can_collide{ true };

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	bool m_is_ready_to_be_deleted{ 1 };
	bool m_is_fallen_off_screen{ 0 };

	friend class Tiles;
};

class NormalTile : public Tile
{
public:
	NormalTile();

	void update(sf::Time dt) override;
};

class HorizontalSlidingTile: public Tile
{
	float m_speed;
	float m_left{0}, m_right{-1};
public:
	HorizontalSlidingTile(float speed);

	void update(sf::Time dt) override;
	void updateMovingLocation(sf::FloatRect area);
	void updateMovingLocation(float left, float right);
	
};

class VerticalSlidingTile : public Tile
{
	float m_speed;
	float m_top{0}, m_bottom{-1};
	sf::FloatRect m_area{};
public:
	VerticalSlidingTile(float speed);

	void update(sf::Time dt) override;
	void updateMovingLocation(sf::FloatRect area);
	void updateMovingLocation(float top, float bottom);
};

class DecayedTile : public Tile
{
	float m_speed, m_vert_speed{0}, m_gravity{500};
	float m_left{ 0 }, m_right{ -1 };
	
	bool m_is_breaked{ 0 };
	
public:
	DecayedTile();
	DecayedTile(float speed);

	void update(sf::Time dt) override;
	void updateMovingLocation(sf::FloatRect area);
	void updateMovingLocation(float left, float right);
};

class BombTile : public Tile
{
	bool m_is_started_exploding{ 0 }, m_is_exploded{ 0 }, m_is_gone{ 0 };
	float m_exploding_height, m_current_height{};

public:
	BombTile(float exploding_height);
	 
	void update(sf::Time dt) override;
	void updateHeight(float current_height);
	bool isDestroyed() const override;
private:
	void startExploding();

};

class OneTimeTile : public Tile
{
	bool m_is_gone{ 0 };

public:
	OneTimeTile();

	void update(sf::Time dt) override;
	bool isDestroyed() const override;
};

class TeleportTile : public Tile
{
	
	std::deque<sf::Vector2f> m_offsets{ {0, 0} };
	size_t m_current_offset_index{ 0 };

public:
	TeleportTile();

	void update(sf::Time dt) override;
	bool isDestroyed() const override;

	void addNewPosition(sf::Vector2f offset_from_first_position);

private:
	void next();
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

class ClusterTile : public Tile
{
	
	std::deque<sf::Vector2f> m_offsets{ {0, 0} };
	size_t m_current_offset_index{ 0 };
	sf::Time m_existing_time{}, m_transition_start{}, m_transition_duration{sf::seconds(0.2)}, m_oscillating_duration{sf::seconds(0.3)};
	sf::Vector2f m_oscillation_offset{ 10 * m_texture_scale, 0 };
	bool m_is_in_transition{ 0 };
public:

	class Id
	{
		size_t m_id;
		std::shared_ptr<std::deque<ClusterTile*>> m_tiles;
		void addTile(ClusterTile* tile);
		void removeTile(ClusterTile* tile);
		inline static size_t counter{ 0 };
	public:
		Id();

		friend class ClusterTile;
	};
private:
	Id m_id;

public:
	ClusterTile(Id id);
	ClusterTile(const ClusterTile& oth);
	ClusterTile(ClusterTile&& oth) noexcept;
	ClusterTile& operator=(const ClusterTile& oth);
	ClusterTile& operator=(ClusterTile&& oth) noexcept;

	void setId(Id id);
	void update(sf::Time dt) override;
	void addNewPosition(sf::Vector2f offset_from_first_position);
	~ClusterTile();

private:
	static sf::Vector2f m_interpolation(sf::Vector2f start, sf::Vector2f end, float progress);
	sf::Vector2f getCurrentOffset() const;
	void next(bool recursive = 1);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

};

class LevelGenerator;
class Tiles : public sf::Drawable
{
private:
	std::deque<std::unique_ptr<Tile>> m_tiles;
	sf::RenderWindow& m_game_window;

public:
	Tiles(sf::RenderWindow& game_window);
	void update(sf::Time dt);
	Tile* getTileDoodleWillJump(sf::FloatRect doodle_feet);
	bool willDoodleJump(sf::FloatRect doodle_feet);
	size_t getTilesCount();

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	friend class LevelGenerator;

};