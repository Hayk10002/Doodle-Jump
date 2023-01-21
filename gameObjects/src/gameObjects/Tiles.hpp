#pragma once
#include <string>
#include <deque>
#include <coroutine>
#include <functional>

#include <SFML/Graphics.hpp>
#include <Thor/Animations.hpp>

class Tiles;
class Tile : public sf::Sprite
{
public:
	Tile();
	Tile(sf::Texture* texture_ptr);
	Tile(const Tile&) = default;
	Tile(Tile&&) = default;
	Tile& operator=(const Tile&) = default;
	Tile& operator=(Tile&&) = default;

	sf::FloatRect getCollisionBox() const;
	virtual bool isDestroyed() const;
	virtual void update(sf::Time) = 0;
	void setSpecUpdate(std::function<void(sf::Time)> spec_update);
	void setDoodleJumpCallback(std::function<bool()> on_doodle_jump);

	virtual ~Tile() = default;

protected:
	std::shared_ptr<thor::AnimationMap<sf::Sprite, std::string>> m_animations;
	thor::Animator<sf::Sprite, std::string> m_animator;
	sf::FloatRect m_collision_box{};
	std::function<void(sf::Time)> m_spec_update{ [](sf::Time) {} };
	std::function<bool()> m_on_doodle_jump{ []() {return true; } };

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	friend class Tiles;
};

class NormalTile : public Tile
{
	float m_texture_scale{ 0.65 };
	sf::Vector2f m_collision_box_size{ sf::Vector2f{114, 30} * m_texture_scale };
public:
	NormalTile();

	void update(sf::Time dt) override;
};

class HorizontalSlidingTile: public Tile
{
	float m_texture_scale{ 0.65 };
	sf::Vector2f m_collision_box_size{ sf::Vector2f{114, 30} * m_texture_scale };
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
	float m_texture_scale{ 0.65 };
	sf::Vector2f m_collision_box_size{ sf::Vector2f{114, 30} * m_texture_scale };
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
	float m_texture_scale{ 0.65 };
	sf::Vector2f m_collision_box_size{ sf::Vector2f{120, 30} * m_texture_scale };
	float m_speed, m_vert_speed{0}, m_gravity{1200};
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
	float m_texture_scale{ 0.65 };
	sf::Vector2f m_collision_box_size{ sf::Vector2f{114, 30} * m_texture_scale };
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
	float m_texture_scale{ 0.65 };
	sf::Vector2f m_collision_box_size{ sf::Vector2f{114, 30} * m_texture_scale };
	bool m_is_gone{ 0 };

public:
	OneTimeTile();

	void update(sf::Time dt) override;
	bool isDestroyed() const override;
};

class TeleportTile : public Tile
{
	float m_texture_scale{ 0.65 };
	sf::Vector2f m_collision_box_size{ sf::Vector2f{114, 30} *m_texture_scale };

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
	float m_texture_scale{ 0.65 };
	sf::Vector2f m_collision_box_size{ sf::Vector2f{114, 30} *m_texture_scale };

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


class Tiles : public sf::Drawable
{
private:

	struct TileGenerator {

		struct promise_type;
		using handle_type = std::coroutine_handle<promise_type>;

		TileGenerator(handle_type h) : coro(h) {} 
		handle_type coro;


		~TileGenerator();
		TileGenerator(const TileGenerator&) = delete;
		TileGenerator& operator = (const TileGenerator&) = delete;
		TileGenerator(TileGenerator&& oth) noexcept;
		TileGenerator& operator=(TileGenerator&& oth) noexcept;
		std::unique_ptr<Tile> getValue();
		bool next();

		std::unique_ptr<Tile> getNextValue();
		struct promise_type {
			promise_type() = default;
			promise_type(const promise_type&) = delete;
			promise_type(promise_type&& oth) = default;
			promise_type& operator=(const promise_type&) = delete;
			promise_type& operator=(promise_type&& oth) = default;
			~promise_type() = default;

			auto initial_suspend();
			auto final_suspend() noexcept;
			auto get_return_object();
			auto return_void() {};

			auto yield_value(Tile* value);
			void unhandled_exception();
			std::unique_ptr<Tile> current_value{nullptr};
		};

	};


	std::deque<std::unique_ptr<Tile>> m_tiles;
	TileGenerator m_tile_generator;
	sf::RenderWindow& m_game_window;
	bool m_more_tiles{ 1 };
	TileGenerator get_tile_generator();

public:
	Tiles(sf::RenderWindow& game_window);
	void update(sf::Time dt);
	bool willDoodleJump(sf::FloatRect doodle_feet);
	size_t getTilesCount();

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;


};