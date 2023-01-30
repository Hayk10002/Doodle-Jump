#include "LevelGenerator.hpp"

#include <Thor/Math.hpp>
#include <common/Utils.hpp>

LevelGenerator::Generator::~Generator()
{
	if (coro) coro.destroy();
}

LevelGenerator::Generator::Generator(Generator&& oth) noexcept
{
	oth.coro = nullptr;
}

LevelGenerator::Generator& LevelGenerator::Generator::operator=(Generator&& oth) noexcept
{
	coro = std::exchange(oth.coro, nullptr);
	return *this;
}

bool LevelGenerator::Generator::resume()
{
	if (!coro.done()) coro.resume();
	return !coro.done();
}

LevelGenerator::Generator LevelGenerator::Generator::promise_type::get_return_object()
{
	return Generator{ handle_type::from_promise(*this) };
}

LevelGenerator::LevelGenerator(sf::RenderWindow* window, Tiles* tiles) :
	m_window(window),
	m_tiles(tiles),
	m_generator(getGenerator()),
	m_generated_height(window->mapPixelToCoords(sf::Vector2i(window->getSize())).y)
{}

void LevelGenerator::update()
{
	m_generating_area = getGeneratingArea();
	if(m_generating_area.height >= 0) m_generator.resume();
}

sf::FloatRect LevelGenerator::getGeneratingArea()
{
	sf::FloatRect area{ utils::getViewArea(*m_window)};
	if (m_generated_height > area.top + area.height) m_generated_height = area.top + area.height;
	area.top -= area.height / 2;
	area.height = m_generated_height - area.top;
	return area;
}

LevelGenerator::Generator LevelGenerator::getGenerator()
{
	while (true)
	{
		NormalTile* tile = new NormalTile{};
		tile->setPosition(thor::Distributions::uniform(m_generating_area.left, m_generating_area.left + m_generating_area.width)(), m_generating_area.top + m_generating_area.height - 100);
		m_tiles->m_tiles.push_back(std::unique_ptr<NormalTile>(tile));
		m_generated_height -= 100;
		co_await std::suspend_always{};
	}
}
