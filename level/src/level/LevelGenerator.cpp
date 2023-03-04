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

LevelGenerator::LevelGenerator(sf::RenderWindow* window, Tiles* tiles, Items* items, Monsters* monsters) :
	m_window(window),
	m_tiles(tiles),
	m_items(items),
	m_monsters(monsters),
	m_generator(getGenerator()),
	m_generated_height(window->mapPixelToCoords(sf::Vector2i(window->getSize())).y)
{}

void LevelGenerator::update()
{
	while(true)
	{
		m_generating_area = getGeneratingArea();
		if (m_generating_area.height >= 0) m_generator.resume();
		else break;
	}
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
		Tile* tile = generateNormalTile(m_generated_height, m_generating_area.left, m_generating_area.left + m_generating_area.width);
		if (!thor::Distributions::uniform(0, 4)())
		{
			SpringShoes* item = new SpringShoes(tile, thor::Distributions::uniform(3, 6)(), m_tiles, m_monsters);
			item->setOffsetFromTile(thor::Distributions::uniform(-tile->getCollisionBoxSize().x / 2 + item->getCollisionBoxSize().x / 2, tile->getCollisionBoxSize().x / 2 - item->getCollisionBoxSize().x / 2)());
			m_items->m_items.push_back(std::unique_ptr<SpringShoes>(item));
		}
		if (!thor::Distributions::uniform(0, 10)())
		{
			Shield* item = new Shield(tile);
			item->setOffsetFromTile(thor::Distributions::uniform(-tile->getCollisionBoxSize().x / 2 + item->getCollisionBoxSize().x / 2, tile->getCollisionBoxSize().x / 2 - item->getCollisionBoxSize().x / 2)());
			m_items->m_items.push_back(std::unique_ptr<Shield>(item));
		}
		if (!thor::Distributions::uniform(0, 50)())
		{
			HorizontalMovingMonster* monster = new HorizontalMovingMonster(thor::Distributions::uniform(150, 200)());
			monster->setPosition(thor::Distributions::uniform(m_generating_area.left + monster->getCollisionBoxSize().x / 2, m_generating_area.left + m_generating_area.width - monster->getCollisionBoxSize().x / 2)(), m_generating_area.top + m_generating_area.height - 100);
			monster->setSpecUpdate([&monster, this](sf::Time)
				{
					monster->updateMovingLocation(utils::getViewArea(*m_window));
				});
			m_monsters->m_monsters.push_back(std::unique_ptr<HorizontalMovingMonster>(monster));
		}
		if (!thor::Distributions::uniform(0, 50)())
		{
			SpiderMonster* monster = new SpiderMonster();
			monster->setPosition(thor::Distributions::uniform(m_generating_area.left + monster->getCollisionBoxSize().x / 2, m_generating_area.left + m_generating_area.width - monster->getCollisionBoxSize().x / 2)(), m_generating_area.top + m_generating_area.height - 100);
			m_monsters->m_monsters.push_back(std::unique_ptr<SpiderMonster>(monster));
		}
		if (!thor::Distributions::uniform(0, 50)())
		{
			BigBlueMonster* monster = new BigBlueMonster();
			monster->setPosition(thor::Distributions::uniform(m_generating_area.left + monster->getCollisionBoxSize().x / 2, m_generating_area.left + m_generating_area.width - monster->getCollisionBoxSize().x / 2)(), m_generating_area.top + m_generating_area.height - 100);
			m_monsters->m_monsters.push_back(std::unique_ptr<BigBlueMonster>(monster));
		}
		if (!thor::Distributions::uniform(0, 50)())
		{
			VirusMonster* monster = new VirusMonster();
			monster->setPosition(thor::Distributions::uniform(m_generating_area.left + monster->getCollisionBoxSize().x / 2, m_generating_area.left + m_generating_area.width - monster->getCollisionBoxSize().x / 2)(), m_generating_area.top + m_generating_area.height - 100);
			m_monsters->m_monsters.push_back(std::unique_ptr<VirusMonster>(monster));
		}
		if (!thor::Distributions::uniform(0, 50)())
		{
			UFOMonster* monster = new UFOMonster();
			monster->setPosition(thor::Distributions::uniform(m_generating_area.left + monster->getCollisionBoxSize().x / 2, m_generating_area.left + m_generating_area.width - monster->getCollisionBoxSize().x / 2)(), m_generating_area.top + m_generating_area.height - 100);
			m_monsters->m_monsters.push_back(std::unique_ptr<UFOMonster>(monster));
		}
		if (!thor::Distributions::uniform(0, 50)())
		{
			BlackHoleMonster* monster = new BlackHoleMonster();
			monster->setPosition(thor::Distributions::uniform(m_generating_area.left + monster->getCollisionBoxSize().x / 2, m_generating_area.left + m_generating_area.width - monster->getCollisionBoxSize().x / 2)(), m_generating_area.top + m_generating_area.height - 100);
			m_monsters->m_monsters.push_back(std::unique_ptr<BlackHoleMonster>(monster));
		}
		if (!thor::Distributions::uniform(0, 50)())
		{
			GreenOvalMonster* monster = new GreenOvalMonster();
			monster->setPosition(thor::Distributions::uniform(m_generating_area.left + monster->getCollisionBoxSize().x / 2, m_generating_area.left + m_generating_area.width - monster->getCollisionBoxSize().x / 2)(), m_generating_area.top + m_generating_area.height - 100);
			m_monsters->m_monsters.push_back(std::unique_ptr<GreenOvalMonster>(monster));
		}
		if (!thor::Distributions::uniform(0, 50)())
		{
			FlatGreenMonster* monster = new FlatGreenMonster();
			monster->setPosition(thor::Distributions::uniform(m_generating_area.left + monster->getCollisionBoxSize().x / 2, m_generating_area.left + m_generating_area.width - monster->getCollisionBoxSize().x / 2)(), m_generating_area.top + m_generating_area.height - 100);
			m_monsters->m_monsters.push_back(std::unique_ptr<FlatGreenMonster>(monster));
		}
		if (!thor::Distributions::uniform(0, 50)())
		{
			BigGreenMonster* monster = new BigGreenMonster();
			monster->setPosition(thor::Distributions::uniform(m_generating_area.left + monster->getCollisionBoxSize().x / 2, m_generating_area.left + m_generating_area.width - monster->getCollisionBoxSize().x / 2)(), m_generating_area.top + m_generating_area.height - 100);
			m_monsters->m_monsters.push_back(std::unique_ptr<BigGreenMonster>(monster));
		}
		if (!thor::Distributions::uniform(0, 50)())
		{
			BlueWingedMonster* monster = new BlueWingedMonster();
			monster->setPosition(thor::Distributions::uniform(m_generating_area.left + monster->getCollisionBoxSize().x / 2, m_generating_area.left + m_generating_area.width - monster->getCollisionBoxSize().x / 2)(), m_generating_area.top + m_generating_area.height - 100);
			m_monsters->m_monsters.push_back(std::unique_ptr<BlueWingedMonster>(monster));
		}
		if (!thor::Distributions::uniform(0, 50)())
		{
			TheTerrifyMonster* monster = new TheTerrifyMonster({ thor::Distributions::uniform(150.f, 200.f)(), -thor::Distributions::uniform(150.f, 200.f)() });
			monster->setPosition(thor::Distributions::uniform(m_generating_area.left + monster->getCollisionBoxSize().x / 2, m_generating_area.left + m_generating_area.width - monster->getCollisionBoxSize().x / 2)(), m_generating_area.top + m_generating_area.height - 100);
			monster->setSpecUpdate([&monster, this](sf::Time)
				{
					monster->updateMovingLocation(utils::getViewArea(*m_window));
				});
			m_monsters->m_monsters.push_back(std::unique_ptr<TheTerrifyMonster>(monster));
		}
		co_await std::suspend_always{};
	}
}

Tile* LevelGenerator::generateNormalTile(float& generated_height, float left, float right)
{
	NormalTile* tile = new NormalTile{};
	tile->setPosition(thor::Distributions::uniform(left + tile->getCollisionBoxSize().x / 2, right - tile->getCollisionBoxSize().x / 2)(), generated_height - 100);
	m_tiles->m_tiles.push_back(std::unique_ptr<NormalTile>(tile));
	generated_height -= 105;

	return tile;
}
