#include "Level.hpp"

Level::Level(sf::Vector2f doodle_pos, sf::RenderWindow& window):
	window(window),
	ib(&global_textures["background"], &window),
	doodle(doodle_pos),
	tiles(window),
	items(window),
	monsters(window),
	level_generator(&window, &tiles, &items, &monsters)
{
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
