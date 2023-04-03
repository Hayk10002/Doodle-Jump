#include <iostream>

#include <SFML/Graphics.hpp>

#include <imgui.h>
#include <SFML/imgui-SFML.h>

#include <Thor/Input.hpp>
#include <Thor/Resources.hpp>
#include <Thor/Math.hpp>
#include <SelbaWard.hpp>

#include <DoodleJumpConfig.hpp>
#include <common/Resources.hpp>
#include <common/DebugImGui.hpp>
#include <common/Utils.hpp>
#include <drawables/ImageBackground.hpp>
#include <drawables/Scene.hpp>
#include <level/LevelGenerator.hpp>
#include <gameObjects/Doodle.hpp>
#include <gameObjects/Tiles.hpp>
#include <gameObjects/Items.hpp>
#include <gameObjects/Monsters.hpp>

enum class UserActions
{
	None,
	Close,
	Resize,
	Left, 
	Right,
	Shoot,
	Die,
	Ressurect,
	BreakPoint
};



int main()
{
	//setup the window
	constexpr size_t WINDOW_WIDTH = 500;
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, 800), "Doodle Jump");
	sf::Vector2u window_prev_size(window.getSize());
	window.setKeyRepeatEnabled(false);

	//setup Dear ImGui
	ImGui::SFML::Init(window);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

	//setup thor resource manager
	init_resources();

	//create the background
	ImageBackground ib(&global_textures["background"], &window);

	Doodle doodle({ 400, 400 });
	Tiles tiles(window);
	Items items(window);
	Monsters monsters(window);

	LevelGenerator level_generator(&window, &tiles, &items, &monsters);

	//create level scene
	Scene scene;
	scene.addObject(ib, []() {});
	scene.addObject(tiles);
	auto items_obj = scene.addObject(items);
	auto doodle_obj = scene.addObject(doodle);
	auto doodle_dupl_obj = doodle_obj.get_duplicate();
	doodle_dupl_obj.setUpdate([&doodle]() {doodle.updateForDrawing(); });
	scene.moveObjectUpInUpdateOrder(items_obj);
	scene.addObject(monsters);
	scene.addToUpdateList(doodle_dupl_obj);
	scene.setWindow(&window);
	scene.setScrollingType(InstantScrolling());

	sf::Font default_font;
	default_font.loadFromFile(RESOURCES_PATH"mistal.ttf");
	sf::Text points_text("", default_font);
	points_text.setFillColor(sf::Color::Black);
	points_text.setOutlineColor(sf::Color::White);
	points_text.setOutlineThickness(2);

	//setup the user actions
	thor::ActionMap<UserActions> action_map;
	action_map[UserActions::Close] = thor::Action(sf::Event::Closed);
	action_map[UserActions::Resize] = thor::Action(sf::Event::Resized);
	action_map[UserActions::Left] = thor::Action(sf::Keyboard::A) || thor::Action(sf::Keyboard::Left);
	action_map[UserActions::Right] = thor::Action(sf::Keyboard::D) || thor::Action(sf::Keyboard::Right);
	action_map[UserActions::Shoot] = thor::Action(sf::Mouse::Left, thor::Action::PressOnce);
	action_map[UserActions::Die] = thor::Action(sf::Keyboard::BackSpace, thor::Action::PressOnce);
	action_map[UserActions::Ressurect] = thor::Action(sf::Keyboard::Enter, thor::Action::PressOnce);
	action_map[UserActions::BreakPoint] = thor::Action(sf::Keyboard::LShift) && thor::Action(sf::Keyboard::Escape);

	//frame clock
	size_t frame_count = 0;
	sf::Clock deltaClock;
	sf::Time full_time, dt;
	float game_speed{ 1 };

	float dragging_speed = 100;

	while (window.isOpen())
	{
		//event handling
		action_map.clearEvents();
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(window, event);
			switch (event.type)
			{
			case sf::Event::KeyPressed:
			case sf::Event::KeyReleased:
			case sf::Event::TextEntered:
				if (!ImGui::GetIO().WantCaptureKeyboard) action_map.pushEvent(event);
				break;
			case sf::Event::MouseButtonPressed:
			case sf::Event::MouseButtonReleased:
			case sf::Event::MouseMoved:
			case sf::Event::MouseWheelScrolled:
				if (!ImGui::GetIO().WantCaptureMouse) action_map.pushEvent(event);
				break;
			default:
				action_map.pushEvent(event);
				break;
			}
		}

		//updating

		frame_count++;
		full_time += (dt = deltaClock.restart() * game_speed);

		
		ImGui::SFML::Update(window, dt);
		ImGui::Begin("Info");
		ImGui::Text("Frame count: %d", frame_count);
		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
		ImGui::DragFloat("Drag speed", &dragging_speed, 1, 0.f, 10000.f, "%.3f", ImGuiSliderFlags_Logarithmic);
		ImGui::DragFloat("Game speed", &game_speed, 1, 0.f, 10000.f, "%.3f", ImGuiSliderFlags_Logarithmic);
		sf::Vector2f position = doodle.getPosition();
		ImGui::DragFloat2("Position", (float*)&position, dragging_speed);
		doodle.setPosition(position);
		ImGui::Text("Area: {%f, %f, %f, %f}", doodle.getArea().left, doodle.getArea().top, doodle.getArea().width, doodle.getArea().height);
		ImGui::Text("Is doodle dead: %d", doodle.isDead());
		//if (ImGui::IsWindowFocused())  dt = sf::Time::Zero;
		ImGui::End();

		if (action_map.isActive(UserActions::Close))
			window.close();
		if (action_map.isActive(UserActions::Resize))
		{
			window.setSize({ WINDOW_WIDTH, window.getSize().y });
			sf::Vector2f scale = { (float)window_prev_size.x / window.getSize().x, (float)window_prev_size.y / window.getSize().y };
			window_prev_size = { window.getSize().x, window.getSize().y };
			scene.zoom(scale, true);
		}
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
		if (action_map.isActive(UserActions::BreakPoint))
		{
			int debug = 0;
		}

		level_generator.update();
		doodle.updateArea(utils::getViewArea(window));
		if(!doodle.isDead()){
			doodle.updateTiles(tiles);
			doodle.updateItems(items);
			doodle.updateMonsters(monsters);
		}
		scene.updateObjects(dt);
		if (doodle.isTooHigh()) scene.scrollUp(doodle.getArea().top - doodle.getPosition().y);
		scene.updateScrolling();
		ib.update();

		points_text.setPosition(window.mapPixelToCoords({ 10, 10 }));
		points_text.setString(std::to_string(int(-window.mapPixelToCoords({ 0, 0 }).y)));
	
		//drawing
		window.clear();
		window.draw(scene);
		window.draw(points_text);
		ImGui::SFML::Render(window);
		window.display();
	}



	ImGui::SFML::Shutdown();
	release_resources();
	return 0;
}