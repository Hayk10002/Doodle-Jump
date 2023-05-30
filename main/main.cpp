#include <iostream>

#include <SFML/Graphics.hpp>

#include <imgui.h>
#include <SFML/imgui-SFML.h>

#include <Thor/Input.hpp>
#include <Thor/Resources.hpp>
#include <Thor/Math.hpp>
#include <SelbaWard.hpp>
#include <nlohmann/json.hpp>

#include <DoodleJumpConfig.hpp>
#include <common/Resources.hpp>
#include <common/DebugImGui.hpp>
#include <common/Utils.hpp>
#include <common/GameStuff.hpp>
#include <drawables/ImageBackground.hpp>
#include <drawables/Scene.hpp>
#include <level/Level.hpp>
#include <level/LevelGenerator.hpp>
#include <gameObjects/Doodle.hpp>
#include <gameObjects/Tiles.hpp>
#include <gameObjects/Items.hpp>
#include <gameObjects/Monsters.hpp>





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

	//create the level
	Level level(window);
	size_t current_level = 0;
	
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
		sf::Vector2f position = level.doodle.getPosition();
		ImGui::DragFloat2("Position", (float*)&position, dragging_speed);
		level.doodle.setPosition(position);
		ImGui::Text("Area: {%f, %f, %f, %f}", level.doodle.getArea().left, level.doodle.getArea().top, level.doodle.getArea().width, level.doodle.getArea().height);
		ImGui::Text("Is doodle dead: %d", level.doodle.isDead());
		ImGui::Text("Is doodle completely dead: %d", level.doodle.isCompletelyDead());

		ImGui::Text("Level no.");
		ImGui::SameLine();
		ImGui::InputScalar("##Level no", ImGuiDataType_U64, &current_level);
		ImGui::SameLine();
		if (ImGui::SmallButton("Play"))
		{
			level.loadFromFile(std::format(RESOURCES_PATH"Levels/level{}.json", current_level));
			level.refresh();
		}
		
		//if (ImGui::IsWindowFocused())  dt = sf::Time::Zero;
		ImGui::End();

		if (action_map.isActive(UserActions::Close))
			window.close();
		if (action_map.isActive(UserActions::Resize))
		{
			window.setSize({ WINDOW_WIDTH, window.getSize().y });
			sf::Vector2f scale = { (float)window_prev_size.x / window.getSize().x, (float)window_prev_size.y / window.getSize().y };
			window_prev_size = { window.getSize().x, window.getSize().y };
			level.scene.zoom(scale, true);
		}
		if (action_map.isActive(UserActions::BreakPoint))
		{
			int debug = 0;
		}

		level.handleGameEvents(action_map, dt);
		level.update(dt);

		points_text.setPosition(window.mapPixelToCoords({ 10, 10 }));
		points_text.setString(std::to_string(int(-window.mapPixelToCoords({ 0, 0 }).y)));
	
		//drawing
		window.clear();
		window.draw(level.scene);
		window.draw(points_text);
		ImGui::SFML::Render(window);
		window.display();
	}



	ImGui::SFML::Shutdown();
	release_resources();

	return 0;
}