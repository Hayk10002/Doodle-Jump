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

	sf::FloatRect getCollisionBox() const;
	virtual bool isDestroyed() const;
	virtual void update(sf::Time) = 0;
	void setSpecUpdate(std::function<void(sf::Time)> spec_update);


protected:
	std::shared_ptr<thor::AnimationMap<sf::Sprite, std::string>> m_animations;
	thor::Animator<sf::Sprite, std::string> m_animator;
	sf::FloatRect m_collision_box{};
	std::function<void(sf::Time)> m_spec_update{ [](sf::Time) {} };

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
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
	sf::Vector2f m_collision_box_size{ sf::Vector2f{114, 30} *m_texture_scale };
	float m_speed;
	float m_top{0}, m_bottom{-1};
	sf::FloatRect m_area{};
public:
	VerticalSlidingTile(float speed);

	void update(sf::Time dt) override;
	void updateMovingLocation(sf::FloatRect area);
	void updateMovingLocation(float top, float bottom);
};




class Tiles : public sf::Drawable
{
private:

	struct TileGenerator {

		struct promise_type;
		using handle_type = std::coroutine_handle<promise_type>;

		TileGenerator(handle_type h) : coro(h) {}                         // (3)
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
			promise_type(promise_type&& oth) noexcept;
			promise_type& operator=(const promise_type&) = delete;
			promise_type& operator=(promise_type&& oth) noexcept;
			~promise_type() = default;

			auto initial_suspend();
			auto final_suspend() noexcept;
			auto get_return_object();
			auto return_void() {};

			auto yield_value(Tile* value);
			void unhandled_exception();
			Tile* current_value{nullptr};
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

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;


};