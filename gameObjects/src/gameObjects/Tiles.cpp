#include "Tiles.hpp"

#include <Thor/Math.hpp>

#include <common/Resources.hpp>
#include <common/Utils.hpp>

Tile::Tile() : Tile(nullptr)
{}

Tile::Tile(sf::Texture* texture_ptr):
	m_animations(new thor::AnimationMap<sf::Sprite, std::string>()),
	m_animator(*m_animations)
{
	if (texture_ptr) setTexture(*texture_ptr);
}

sf::FloatRect Tile::getCollisionBox() const
{
	return m_collision_box;
}

bool Tile::isDestroyed() const
{
	return false;
}

void Tile::setSpecUpdate(std::function<void(sf::Time)> spec_update)
{
	m_spec_update = spec_update;
}

void Tile::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(sf::Sprite(*this), states);
	/*sf::RectangleShape sh({ m_collision_box.width, m_collision_box.height });
	sh.setOrigin(sf::Vector2f{ m_collision_box.width, m_collision_box.height } / 2.f);
	sh.setFillColor(sf::Color(255, 0, 0, 100));
	sh.setPosition(getPosition());
	target.draw(sh);*/
}


NormalTile::NormalTile() :
	Tile(&global_textures["tiles"])
{
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 0, 0, 128, 40 }, { 64, 20 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	m_animator.play() << thor::Playback::loop("default");
}

void NormalTile::update(sf::Time dt)
{
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_animator.update(dt);
	m_animator.animate(*this);
}

HorizontalSlidingTile::HorizontalSlidingTile(float speed):
	Tile(&global_textures["tiles"]),
	m_speed(speed)
{
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 0, 40, 128, 40 }, { 64, 20 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	m_animator.play() << thor::Playback::loop("default");
}

void HorizontalSlidingTile::update(sf::Time dt)
{
	if (m_collision_box == sf::FloatRect{}) m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_spec_update(dt);
	float step = m_speed * dt.asSeconds();
	if(m_right > m_left)
	{
		if (step >= 0 && m_collision_box.left + m_collision_box.width + step >= m_right) m_speed = -m_speed;
		if (step < 0 && m_collision_box.left + step <= m_left) m_speed = -m_speed;
	}
	move(m_speed * dt.asSeconds(), 0);
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_animator.update(dt);
	m_animator.animate(*this);
}

void HorizontalSlidingTile::updateMovingLocation(sf::FloatRect area)
{
	m_left = area.left;
	m_right = area.left + area.width;
}

void HorizontalSlidingTile::updateMovingLocation(float left, float right)
{
	m_left = left;
	m_right = right;
}

VerticalSlidingTile::VerticalSlidingTile(float speed):
	Tile(&global_textures["tiles"]),
	m_speed(speed)
{
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 0, 80, 128, 40 }, { 64, 20 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	m_animator.play() << thor::Playback::loop("default");
}

void VerticalSlidingTile::update(sf::Time dt)
{
	if (m_collision_box == sf::FloatRect{}) m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_spec_update(dt);
	float step = m_speed * dt.asSeconds();
	if (m_bottom > m_top)
	{
		if (step >= 0 && m_collision_box.top + m_collision_box.height + step >= m_bottom) m_speed = -m_speed;
		if (step < 0 && m_collision_box.top + step <= m_top) m_speed = -m_speed;
	}
	move(0, m_speed * dt.asSeconds());
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_animator.update(dt);
	m_animator.animate(*this);
}

void VerticalSlidingTile::updateMovingLocation(sf::FloatRect area)
{
	m_top = area.top;
	m_bottom = area.top + area.width;
}

void VerticalSlidingTile::updateMovingLocation(float top, float bottom)
{
	m_top = top;
	m_bottom = bottom;
}

Tiles::TileGenerator::~TileGenerator() 
{
	if (coro) coro.destroy();
}

Tiles::TileGenerator::TileGenerator(TileGenerator&& oth) noexcept : coro(oth.coro) 
{
	oth.coro = nullptr;
}

Tiles::TileGenerator& Tiles::TileGenerator::operator=(TileGenerator&& oth) noexcept
{
	coro = std::exchange(oth.coro, nullptr);
	return *this;
}

std::unique_ptr<Tile> Tiles::TileGenerator::getValue() 
{
	Tile* val = coro.promise().current_value;
	coro.promise().current_value = nullptr;
	return std::unique_ptr<Tile>(val);
}

bool Tiles::TileGenerator::next()
{
	if(!coro.done()) coro.resume();
	return !coro.done();
}

std::unique_ptr<Tile> Tiles::TileGenerator::getNextValue()
{
	if (next()) return getValue();
	return std::unique_ptr<Tile>(nullptr);
}

Tiles::TileGenerator::promise_type::promise_type(promise_type&& oth) noexcept:
	current_value(oth.current_value)
{
	oth.current_value = nullptr;
}

Tiles::TileGenerator::promise_type& Tiles::TileGenerator::promise_type::operator=(promise_type&& oth) noexcept
{
	current_value = std::exchange(oth.current_value, nullptr);
	return *this;
}

auto Tiles::TileGenerator::promise_type::initial_suspend()
{
	return std::suspend_always{};
}

auto Tiles::TileGenerator::promise_type::final_suspend() noexcept 
{
	return std::suspend_always{};
}

auto Tiles::TileGenerator::promise_type::get_return_object() 
{
	return TileGenerator{ handle_type::from_promise(*this) };
}

auto Tiles::TileGenerator::promise_type::yield_value(Tile* value) 
{
	current_value = value;
	return std::suspend_always{};
}

void Tiles::TileGenerator::promise_type::unhandled_exception() 
{
	throw std::current_exception();
}

Tiles::TileGenerator Tiles::get_tile_generator()
{
	while (1)
	{
		m_more_tiles = 0;
		sf::FloatRect spawn_area = utils::getViewArea(m_game_window);
		auto m_tile_position_distribition = thor::Distributions::rect({ spawn_area.left + spawn_area.width / 2.f, spawn_area.top + spawn_area.height / 2.f }, {spawn_area.width / 2.f, spawn_area.height / 2.f});
	
		switch (thor::Distributions::uniform(1, 3)()) 
		{
		case 1:
			{
				NormalTile* tile = new NormalTile();
				tile->setPosition(m_tile_position_distribition());
				co_yield tile;
				break;
			}
		case 2:
			{
				HorizontalSlidingTile* tile = new HorizontalSlidingTile(thor::Distributions::uniform(-300, 300)());
				tile->setPosition(m_tile_position_distribition());
				tile->setSpecUpdate([this, tile](sf::Time)
				{
					tile->updateMovingLocation(utils::getViewArea(m_game_window));
				});
				co_yield tile;
				break;
			}
		case 3:
			{
				VerticalSlidingTile* tile = new VerticalSlidingTile(thor::Distributions::uniform(-300, 300)());
				tile->setPosition(m_tile_position_distribition());
				tile->setSpecUpdate([this, tile](sf::Time)
				{
					tile->updateMovingLocation(utils::getViewArea(m_game_window));
				});
				co_yield tile;
				break;
			}
		}

	}
}

Tiles::Tiles(sf::RenderWindow& game_window):
	m_game_window(game_window),
	m_tile_generator(get_tile_generator())
{
}

void Tiles::update(sf::Time dt)
{
	while (m_more_tiles && m_tile_generator.next()) if (auto val = m_tile_generator.getValue(); val) m_tiles.push_back(std::move(val));
	m_more_tiles = 1;

	for (auto& tile : m_tiles) tile->update(dt);

	for (size_t i = 0; i < m_tiles.size(); i++)
		if (m_tiles[i]->isDestroyed() || !m_tiles[i]->getCollisionBox().intersects(utils::getViewArea(m_game_window)))
			m_tiles.erase(m_tiles.begin() + i--);
}

void Tiles::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (const auto& tile : m_tiles) target.draw(*tile, states);
}
