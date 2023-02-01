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
#include <level/Level.hpp>
#include <level/LevelGenerator.hpp>
#include <gameObjects/Doodle.hpp>
#include <gameObjects/Tiles.hpp>


enum class UserActions
{
	None,
	Close,
	Resize,
	Left, 
	Right,
	Shoot,
	BreakPoint
};



int main()
{
	//setup the window
	sf::RenderWindow window(sf::VideoMode(528, 800), "Doodle Jump");
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

	LevelGenerator level_generator(&window, &tiles, &items);

	//create level
	Level level;
	level.addObject(ib);
	level.addObject(tiles);
	auto items_obj = level.addObject(items);
	auto doodle_obj = level.addObject(doodle);
	auto doodle_dupl_obj = doodle_obj.get_duplicate();
	doodle_dupl_obj.setUpdate([&doodle]() {doodle.updateForDrawing(); });
	level.moveObjectUpInUpdateOrder(items_obj);
	level.addToUpdateList(doodle_dupl_obj);
	level.setWindow(&window);
	level.setScrollingType(InstantScrolling());

	//setup the user actions
	thor::ActionMap<UserActions> action_map;
	action_map[UserActions::Close] = thor::Action(sf::Event::Closed);
	action_map[UserActions::Resize] = thor::Action(sf::Event::Resized);
	action_map[UserActions::Left] = thor::Action(sf::Keyboard::A, thor::Action::Hold);
	action_map[UserActions::Right] = thor::Action(sf::Keyboard::D, thor::Action::Hold);
	action_map[UserActions::Shoot] = thor::Action(sf::Mouse::Left, thor::Action::PressOnce);
	action_map[UserActions::BreakPoint] = thor::Action(sf::Keyboard::LShift) && thor::Action(sf::Keyboard::Escape);

	//frame clock
	sf::Clock deltaClock;
	sf::Time full_time, dt;

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

		full_time += (dt = deltaClock.restart());

		
		ImGui::SFML::Update(window, dt);
		ImGui::Begin("Info");
		static float dragging_speed = 100;
		ImGui::DragFloat("Drag speed", &dragging_speed, 1, 0.f, 10000.f, "%.3f", ImGuiSliderFlags_Logarithmic);
		sf::Vector2f position = doodle.getPosition();
		ImGui::DragFloat2("Position", (float*)&position, dragging_speed);
		doodle.setPosition(position);
		ImGui::Text("Area: {%f, %f, %f, %f}", doodle.getArea().left, doodle.getArea().top, doodle.getArea().width, doodle.getArea().height);
		//if (ImGui::IsWindowFocused())  dt = sf::Time::Zero;
		ImGui::End();

		if (action_map.isActive(UserActions::Close))
			window.close();
		if (action_map.isActive(UserActions::Resize))
		{
			sf::Vector2f scale = { (float)window_prev_size.x / window.getSize().x, (float)window_prev_size.y / window.getSize().y };
			window_prev_size = { window.getSize().x, window.getSize().y };
			level.zoom(scale, true);
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
		if (action_map.isActive(UserActions::BreakPoint))
		{
			int debug = 0;
		}

		level_generator.update();
		doodle.updateArea(utils::getViewArea(window));
		doodle.updateTiles(tiles);
		doodle.updateItems(items);
		if (doodle.isFallenOutOfScreen()) doodle.setPosition(doodle.getPosition().x, window.mapPixelToCoords(sf::Vector2i{ window.getSize() } / 2).y);
		level.updateObjects(dt);
		if (doodle.isTooHigh()) level.scrollUp(doodle.getArea().top - doodle.getPosition().y);
		level.updateScrolling();
	
		//drawing
		window.clear();
		window.draw(level);
		ImGui::SFML::Render(window);
		window.display();
	}



	ImGui::SFML::Shutdown();
	release_resources();
	return 0;
}