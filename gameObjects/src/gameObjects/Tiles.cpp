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

void Tile::setDoodleJumpCallback(std::function<bool()> on_doodle_jump)
{
	m_on_doodle_jump = on_doodle_jump;
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

DecayedTile::DecayedTile() : DecayedTile(0)
{}

DecayedTile::DecayedTile(float speed) :
	Tile(&global_textures["tiles"]),
	m_speed(speed)
{
	setDoodleJumpCallback([this]() 
	{
		m_is_breaked = 1;
		m_animator.play() << "break";
		return false; 
	});
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 0, 120, 128, 40 }, { 64, 20 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	thor::FrameAnimation breaking_animation;
	breaking_animation.addFrame(1, { 0, 120, 128, 40 }, { 64, 20 });
	breaking_animation.addFrame(1, { 0, 160, 128, 40 }, { 64, 20 });
	breaking_animation.addFrame(1, { 0, 200, 128, 60 }, { 64, 20 });
	breaking_animation.addFrame(1, { 0, 260, 128, 60 }, { 64, 20 });
	m_animations->addAnimation("break", breaking_animation, sf::seconds(0.1));

	m_animator.play() << thor::Playback::loop("default");
}

void DecayedTile::update(sf::Time dt)
{
	if (m_collision_box == sf::FloatRect{}) m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_spec_update(dt);
	float step = m_speed * dt.asSeconds();
	if (m_right > m_left)
	{
		if (step >= 0 && m_collision_box.left + m_collision_box.width + step >= m_right) m_speed = -m_speed;
		if (step < 0 && m_collision_box.left + step <= m_left) m_speed = -m_speed;
	}
	if (m_is_breaked) m_vert_speed += m_gravity * dt.asSeconds();
	move(m_speed * dt.asSeconds(), m_vert_speed * dt.asSeconds());
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_animator.update(dt);
	m_animator.animate(*this);
}

void DecayedTile::updateMovingLocation(sf::FloatRect area)
{
	m_left = area.left;
	m_right = area.left + area.width;
}

void DecayedTile::updateMovingLocation(float left, float right)
{
	m_left = left;
	m_right = right;
}

BombTile::BombTile(float exploding_height):
	Tile(&global_textures["tiles"]),
	m_exploding_height(exploding_height)
{
	setDoodleJumpCallback([this]() 
	{
		if (!m_is_started_exploding)
		{
			startExploding();
			return true;
		}
		return !m_is_exploded;
	});
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 0, 360, 128, 40 }, { 64, 20 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	thor::FrameAnimation preparing_animation;
	preparing_animation.addFrame(1, { 0, 360, 128, 40 }, { 64, 20 });
	preparing_animation.addFrame(1, { 0, 400, 128, 40 }, { 64, 20 });
	preparing_animation.addFrame(1, { 0, 440, 128, 40 }, { 64, 20 });
	preparing_animation.addFrame(1, { 0, 480, 128, 40 }, { 64, 20 });
	preparing_animation.addFrame(10, { 0, 520, 128, 40 }, { 64, 20 });
	m_animations->addAnimation("prepare", preparing_animation, sf::seconds(0.8));
	thor::FrameAnimation exploding_animation;
	exploding_animation.addFrame(1, { 0, 560, 128, 40 }, { 64, 20 });
	exploding_animation.addFrame(1, { 0, 600, 128, 60 }, { 64, 30 });
	exploding_animation.addFrame(1, { 0, 660, 128, 60 }, { 64, 30 });
	m_animations->addAnimation("explode", exploding_animation, sf::seconds(0.2));

	m_animator.play() << thor::Playback::loop("default");
}

void BombTile::update(sf::Time dt)
{
	m_collision_box = sf::FloatRect{ getPosition() - m_collision_box_size / 2.f, m_collision_box_size };
	m_spec_update(dt);
	if (m_current_height > m_exploding_height && !m_is_started_exploding) startExploding();
	m_animator.update(dt);
	m_animator.animate(*this);
}

void BombTile::updateHeight(float current_height)
{
	m_current_height = current_height;
}

bool BombTile::isDestroyed() const
{
	return m_is_gone;
}

void BombTile::startExploding()
{
	m_animator.play()
		<< thor::Playback::notify([this]() {m_is_started_exploding = true; })
		<< "prepare"
		<< thor::Playback::notify([this]() {m_is_exploded = true; })
		<< "explode"
		<< thor::Playback::notify([this]() {m_is_gone = true; });
}

OneTimeTile::OneTimeTile():
	Tile(&global_textures["tiles"])
{
	setDoodleJumpCallback([this]()
	{
		m_animator.play() 
			<< "disappear"
			<< thor::Playback::notify([this]() {m_is_gone = true; });
		return true;
	});
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, { 0, 720, 128, 40 }, { 64, 20 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	m_animations->addAnimation("disappear", [](sf::Sprite& sp, float progress)
	{
		sf::Color col = sp.getColor();
		col.a = 255 - progress * 255;
		sp.setColor(col);
	}, sf::seconds(0.1));

	m_animator.play() << thor::Playback::loop("default");
}

void OneTimeTile::update(sf::Time dt)
{
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f, m_collision_box_size);
	m_animator.update(dt);
	m_animator.animate(*this);
}

bool OneTimeTile::isDestroyed() const
{
	return m_is_gone;
}

TeleportTile::TeleportTile() :
	Tile(&global_textures["tiles"])
{
	setDoodleJumpCallback([this]()
	{
		if (m_current_offset_index < m_offsets.size()) next();
		return true;
	});
	setScale(m_texture_scale, m_texture_scale);
	size_t size = thor::Distributions::uniform(10, 15)();
	auto dur_dist = thor::Distributions::uniform(1, 3);
	thor::FrameAnimation default_animation;
	for (size_t i = 0; i < size; i++) default_animation.addFrame(dur_dist(), { 0, (i % 2) ? 800 : 760, 128, 40 }, {64, 20});
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	m_animations->addAnimation("disappear", [](sf::Sprite& sp, float progress)
	{
		sf::Color col = sp.getColor();
		col.a = 255 - progress * 255;
		sp.setColor(col);
	}, sf::seconds(0.1));
	m_animations->addAnimation("appear", [](sf::Sprite& sp, float progress)
	{
		sf::Color col = sp.getColor();
		col.a = progress * 255;
		sp.setColor(col);
	}, sf::seconds(0.1));

	m_animator.play() << thor::Playback::loop("default");
}

void TeleportTile::update(sf::Time dt)
{
	sf::Vector2f offset = m_offsets[std::min(m_current_offset_index, m_offsets.size() - 1)];
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f + offset, m_collision_box_size);
	m_animator.update(dt);
	m_animator.animate(*this);
}

bool TeleportTile::isDestroyed() const
{
	return m_current_offset_index >= m_offsets.size();
}

void TeleportTile::addNewPosition(sf::Vector2f offset_from_first_position)
{
	m_offsets.push_back(offset_from_first_position);
}

void TeleportTile::next()
{
	m_animator.play() << "disappear";
		

	if (m_current_offset_index < m_offsets.size()) m_animator.queue() << thor::Playback::notify([this]() {m_current_offset_index++; });
	if (m_current_offset_index < m_offsets.size() - 1) 
		m_animator.queue() 
		<< "appear"
		<< thor::Playback::loop("default");
}

void TeleportTile::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	sf::Sprite copy{ *this };
	sf::Vector2f offset = m_offsets[std::min(m_current_offset_index, m_offsets.size() - 1)];
	copy.setPosition(getPosition() + offset);
	target.draw(copy, states);
}

void ClusterTile::Id::addTile(ClusterTile* tile)
{
	m_tiles->push_back(tile);
}

void ClusterTile::Id::removeTile(ClusterTile* tile)
{
	auto itr = std::find(m_tiles->begin(), m_tiles->end(), tile);
	if (itr != m_tiles->end()) m_tiles->erase(itr);
}

ClusterTile::Id::Id():
	m_id(counter++),
	m_tiles(new std::deque<ClusterTile*>{})
{}

ClusterTile::ClusterTile(Id id):
	Tile(&global_textures["tiles"]),
	m_id(id)
{
	setDoodleJumpCallback([this]()
	{
		next();
		return true;
	});	
	setScale(m_texture_scale, m_texture_scale);
	thor::FrameAnimation default_animation;
	default_animation.addFrame(1, {0, 320, 128, 40 }, { 64, 20 });
	m_animations->addAnimation("default", default_animation, sf::seconds(1));
	m_animator.play() << thor::Playback::loop("default");
	m_id.addTile(this);
}

ClusterTile::ClusterTile(const ClusterTile& oth) :
	Tile(oth),
	m_texture_scale(oth.m_texture_scale),
	m_collision_box_size(oth.m_collision_box_size),
	m_offsets(oth.m_offsets),
	m_current_offset_index(oth.m_current_offset_index),
	m_existing_time(oth.m_existing_time),
	m_transition_start(oth.m_transition_start),
	m_transition_duration(oth.m_transition_duration),
	m_oscillating_duration(oth.m_oscillating_duration),
	m_oscillation_offset(oth.m_oscillation_offset),
	m_is_in_transition(oth.m_is_in_transition),
	m_id(oth.m_id)
{
	m_id.addTile(this);
}

ClusterTile::ClusterTile(ClusterTile&& oth) noexcept:
	Tile(std::move(oth)),
	m_texture_scale(std::move(oth.m_texture_scale)),
	m_collision_box_size(std::move(oth.m_collision_box_size)),
	m_offsets(std::move(oth.m_offsets)),
	m_current_offset_index(std::move(oth.m_current_offset_index)),
	m_existing_time(std::move(oth.m_existing_time)),
	m_transition_start(std::move(oth.m_transition_start)),
	m_transition_duration(std::move(oth.m_transition_duration)),
	m_oscillating_duration(std::move(oth.m_oscillating_duration)),
	m_oscillation_offset(std::move(oth.m_oscillation_offset)),
	m_is_in_transition(std::move(oth.m_is_in_transition)),
	m_id(std::move(oth.m_id))
{
	m_id.removeTile(&oth);
	m_id.addTile(this);
}

ClusterTile& ClusterTile::operator=(const ClusterTile& oth)
{
	m_id.removeTile(this);
	this->Tile::operator=(oth);
	m_texture_scale = oth.m_texture_scale;
	m_collision_box_size = oth.m_collision_box_size;
	m_offsets = oth.m_offsets;
	m_current_offset_index = oth.m_current_offset_index;
	m_existing_time = oth.m_existing_time;
	m_transition_start = oth.m_transition_start;
	m_transition_duration = oth.m_transition_duration;
	m_oscillating_duration = oth.m_oscillating_duration;
	m_oscillation_offset = oth.m_oscillation_offset;
	m_is_in_transition = oth.m_is_in_transition;
	m_id = oth.m_id;
	m_id.addTile(this);
	return *this;
}

ClusterTile& ClusterTile::operator=(ClusterTile&& oth) noexcept
{
	m_id.removeTile(this);
	this->Tile::operator=(std::move(oth));
	m_texture_scale = std::move(oth.m_texture_scale);
	m_collision_box_size = std::move(oth.m_collision_box_size);
	m_offsets = std::move(oth.m_offsets);
	m_current_offset_index = std::move(oth.m_current_offset_index);
	m_existing_time = std::move(oth.m_existing_time);
	m_transition_start = std::move(oth.m_transition_start);
	m_transition_duration = std::move(oth.m_transition_duration);
	m_oscillating_duration = std::move(oth.m_oscillating_duration);
	m_oscillation_offset = std::move(oth.m_oscillation_offset);
	m_is_in_transition = std::move(oth.m_is_in_transition);
	m_id = std::move(oth.m_id);
	m_id.removeTile(&oth);
	m_id.addTile(this);
	return *this;
}

void ClusterTile::setId(Id id)
{
	m_id.removeTile(this);
	m_id = id;
	m_id.addTile(this);
}

void ClusterTile::update(sf::Time dt)
{
	m_existing_time += dt;
	if (m_is_in_transition && m_existing_time - m_transition_start > m_transition_duration) m_is_in_transition = 0;
	m_collision_box = sf::FloatRect(getPosition() - m_collision_box_size / 2.f + getCurrentOffset(), m_collision_box_size);
	m_animator.update(dt);
	m_animator.animate(*this);
}

void ClusterTile::addNewPosition(sf::Vector2f offset_from_first_position)
{
	m_offsets.push_back(offset_from_first_position);
}

ClusterTile::~ClusterTile()
{
	m_id.removeTile(this);
}

sf::Vector2f ClusterTile::m_interpolation(sf::Vector2f start, sf::Vector2f end, float progress)
{
	return (1 - progress) * start + progress * end;
}

sf::Vector2f ClusterTile::getCurrentOffset() const
{
	sf::Vector2f offset;
	if (!m_is_in_transition) offset = m_offsets[m_current_offset_index];
	else
	{
		sf::Vector2f m_prev_offset = m_offsets[(m_current_offset_index - 1 + m_offsets.size()) % m_offsets.size()];
		sf::Vector2f m_curr_offset = m_offsets[m_current_offset_index];
		offset = m_interpolation(m_prev_offset, m_curr_offset, (m_existing_time - m_transition_start) / m_transition_duration);
	}

	return offset + m_oscillation_offset * std::sin(m_existing_time / m_oscillating_duration * thor::TrigonometricTraits<float>::pi() * 2);
}

void ClusterTile::next(bool recursive)
{
	++m_current_offset_index %= m_offsets.size();
	m_is_in_transition = true;
	m_transition_start = m_existing_time;
	if(recursive) for (auto tile : (*m_id.m_tiles)) if(tile != this) tile->next(0);
}

void ClusterTile::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	sf::Sprite copy{ *this };
	copy.setPosition(getPosition() + getCurrentOffset());
	target.draw(copy, states);
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
	return std::move(coro.promise().current_value);
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
	current_value = std::unique_ptr<Tile>(value);
	return std::suspend_always{};
}

void Tiles::TileGenerator::promise_type::unhandled_exception() 
{
	throw std::current_exception();
}

Tiles::TileGenerator Tiles::get_tile_generator()
{
	ClusterTile::Id ids[2] = { ClusterTile::Id(), ClusterTile::Id() };

	for(size_t i = 0; i < 15; i++)
	{
		m_more_tiles = 0;
		sf::FloatRect spawn_area = utils::getViewArea(m_game_window);
		auto m_tile_position_distribition = thor::Distributions::rect({ spawn_area.left + spawn_area.width / 2.f, spawn_area.top + spawn_area.height / 2.f }, {spawn_area.width / 2.f, spawn_area.height / 2.f});

		switch (8) 
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
		case 4:
			{
				DecayedTile* tile = new DecayedTile((thor::Distributions::uniform(0, 1)()) ? 0 : thor::Distributions::uniform(-300, 300)());
				tile->setPosition(m_tile_position_distribition());
				tile->setSpecUpdate([this, tile](sf::Time)
				{
					tile->updateMovingLocation(utils::getViewArea(m_game_window));
				});
				co_yield tile;
				break;
			}
		case 5:
			{
				BombTile* tile = new BombTile(thor::Distributions::uniform(-(float)m_game_window.getSize().y / 2.f, (float)m_game_window.getSize().y / 2.f)());
				tile->setPosition(m_tile_position_distribition());
				tile->setSpecUpdate([this, tile](sf::Time)
				{
					tile->updateHeight(m_game_window.mapCoordsToPixel(tile->getPosition()).y - m_game_window.getSize().y / 2.f);
				});
				co_yield tile;
				break;
			}
		case 6:
			{
				OneTimeTile* tile = new OneTimeTile();
				tile->setPosition(m_tile_position_distribition());
				co_yield tile;
				break;
			}
		case 7:
			{
				TeleportTile* tile = new TeleportTile();
				tile->setPosition(m_tile_position_distribition());
				size_t count = thor::Distributions::uniform(3, 6)();
				auto offset_dist = thor::Distributions::rect({0, 0}, { 200, 200 });
				for (size_t i = 0; i < count; i++) tile->addNewPosition(offset_dist());
				co_yield tile;
				break;
			}
		case 8:
			{
				ClusterTile* tile = new ClusterTile(ids[thor::Distributions::uniform(0, 1)()]);
				tile->setPosition(m_tile_position_distribition());
				size_t count = thor::Distributions::uniform(3, 6)();
				auto offset_dist = thor::Distributions::rect({ 0, 0 }, { 200, 200 });
				for (size_t i = 0; i < count; i++) tile->addNewPosition({offset_dist().x, 0});
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

bool Tiles::willDoodleJump(sf::FloatRect doodle_feet)
{
	bool res{0};
	for (const auto& tile : m_tiles) if (tile->getCollisionBox().intersects(doodle_feet)) res = tile->m_on_doodle_jump();
	return res;
}

size_t Tiles::getTilesCount()
{
	return m_tiles.size();
}

void Tiles::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (const auto& tile : m_tiles)
	{
		target.draw(*tile, states);/*
		sf::RectangleShape sh{ {tile->getCollisionBox().width, tile->getCollisionBox().height} };
		sh.setPosition(tile->getCollisionBox().left, tile->getCollisionBox().top);
		sh.setFillColor(sf::Color(255, 0, 0, 100));
		target.draw(sh, states);*/
	}
}

