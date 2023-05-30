#include "Level.hpp"

#include <fstream>
#include <common/Resources.hpp>
#include <common/Utils.hpp>
#include <common/Previews.hpp>
#include <DoodleJumpConfig.hpp>

Level::Level(sf::RenderWindow& window) :
	window(window),
	ib(&global_textures[ib_texture_name], &window),
	doodle(doodle_pos),
	tiles(window),
	items(window),
	monsters(window)
{
	Previews::window = &window;

	//create level scene
	scene.addObject(ib, []() {});
	scene.addObject(tiles);
	auto items_obj = scene.addObject(items);
	auto doodle_obj = scene.addObject(doodle);
	auto doodle_dupl_obj = doodle_obj.get_duplicate();
	doodle_dupl_obj.setUpdate([this]() {doodle.updateForDrawing(); });
	scene.moveObjectUpInUpdateOrder(items_obj);
	scene.addObject(monsters);
	scene.addToUpdateList(doodle_dupl_obj);
	scene.setWindow(&window);
	scene.setScrollingType(InstantScrolling());

	level_generator.setLevelForGeneration(this);
}

void Level::handleGameEvents(thor::ActionMap<UserActions>& action_map, sf::Time dt)
{
	if (action_map.isActive(UserActions::Left))
		doodle.left(dt);
	if (action_map.isActive(UserActions::Right))
		doodle.right(dt);
	if (action_map.isActive(UserActions::Shoot))
	{
		sf::Vector2f mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		sf::Vector2f doodle_pos = doodle.getPosition();
		sf::Vector2f shoot_vec = mouse_pos - doodle_pos;
		float angle = thor::TrigonometricTraits<float>::arcTan2(shoot_vec.y, shoot_vec.x);
		if (angle > 90) angle -= 360;
		doodle.shoot(angle + 90);
	}
	if (action_map.isActive(UserActions::Die)) doodle.dieShrink(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
	if (action_map.isActive(UserActions::Ressurect)) doodle.ressurrect({ window.mapPixelToCoords(sf::Mouse::getPosition(window)).x, window.mapPixelToCoords(sf::Vector2i{ window.getSize() } / 2).y });
}

void Level::update(sf::Time dt)
{
	level_generator.update();
	doodle.updateArea(utils::getViewArea(window));
	if (!doodle.isDead()) {
		doodle.updateTiles(tiles);
		doodle.updateItems(items);
		doodle.updateMonsters(monsters);
	}
	scene.updateObjects(dt);
	if (doodle.isTooHigh()) scene.scrollUp(doodle.getArea().top - doodle.getPosition().y);
	scene.updateScrolling();
	ib.update();
}

void Level::addTile(Tile* tile)
{
	tiles.m_tiles.emplace_back(tile);
}

void Level::addItem(Item* item)
{
	items.m_items.emplace_back(item);
}

void Level::addMonster(Monster* monster)
{
	monsters.m_monsters.emplace_back(monster);
}

void Level::saveToFile(std::string path)
{
	std::ofstream fout(path);
	fout << std::setw(4) << nl::json(*this);
}

void Level::loadFromFile(std::string path)
{
	std::ifstream fin(path);
	if (!fin) return;
	nl::json j;
	fin >> j;
	j.get_to(*this);
}

void Level::refresh()
{
	doodle.ressurrect(sf::Vector2f{ window.getSize() / 2u });
	doodle.updateArea(sf::FloatRect{ {0, 0}, sf::Vector2f(window.getSize()) });

	tiles.m_tiles.clear();
	items.m_items.clear();
	monsters.m_monsters.clear();
	scene.scroll(sf::Vector2f(window.getSize() / 2u) - window.getView().getCenter(), true);
	scene.updateScrolling();
	level_generator.reset();
}

void to_json(nl::json& j, const Level& level)
{
	j["ib_texture_name"] = level.ib_texture_name;
	j["doodle_position"] = level.doodle_pos;
	j["scene"] = level.scene;
	j["level_generator"] = level.level_generator;
}

void from_json(const nl::json& j, Level& level)
{
	if (j.contains("ib_texture_name"))
	{
		j.at("ib_texture_name").get_to(level.ib_texture_name);
		level.ib.setTexture(&global_textures[level.ib_texture_name]);
	}
	if (j.contains("doodle_position"))
	{
		j.at("doodle_position").get_to(level.doodle_pos);
		level.doodle.setPosition(level.doodle_pos);
	}
	if (j.contains("scene")) j.at("scene").get_to(level.scene);
	if (j.contains("level_generator")) j.at("level_generator").get_to(level.level_generator);
}


