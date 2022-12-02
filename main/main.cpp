#include <functional>

#include <SFML/Graphics.hpp>

#include <imgui.h>
#include <SFML/imgui-SFML.h>

#include <Thor/Input.hpp>
#include <Thor/Resources.hpp>
#include <SelbaWard.hpp>

#include <DoodleJumpConfig.hpp>
#include <common/Resources.hpp>
#include <common/DebugImGui.hpp>
#include <drawables/ImageBackground.hpp>
#include <level/level.hpp>


enum class UserActions
{
	None,
	Close,
	Resize,
	Zoom,
	Move,
	Rotate,
	ChangeScrollingType
};



int main()
{
	//setup the window
	sf::RenderWindow window(sf::VideoMode(600, 800), "Doodle Jump");
	sf::Vector2u window_prev_size(window.getSize());

	//setup Dear ImGui
	ImGui::SFML::Init(window);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

	//setup thor resource manager
	init_resources();
	
	//create the background
	ImageBackground ib(&global_textures["background"], &window);

	//create level
	Level level;
	level.setBackground(ib);
	level.setWindow(&window);
	level.setScrollingType(ExponentialScrolling(sf::seconds(2), 10));

	//setup the user actions
	thor::ActionMap<UserActions> action_map;
	action_map[UserActions::Close] = thor::Action(sf::Event::Closed);
	action_map[UserActions::Resize] = thor::Action(sf::Event::Resized);
	action_map[UserActions::Zoom] = thor::Action(sf::Event::MouseWheelScrolled);
	action_map[UserActions::Move] =
		thor::Action(sf::Keyboard::A) ||
		thor::Action(sf::Keyboard::D) ||
		thor::Action(sf::Keyboard::W) ||
		thor::Action(sf::Keyboard::S);
	action_map[UserActions::Rotate] =
		thor::Action(sf::Keyboard::Q) ||
		thor::Action(sf::Keyboard::E);
	action_map[UserActions::ChangeScrollingType] =
		thor::Action(sf::Keyboard::I) ||
		thor::Action(sf::Keyboard::L) ||
		thor::Action(sf::Keyboard::X);

	//frame clock
	sf::Clock deltaClock;
	sf::Time full_time, dt;

	//callback functions for user actions

	std::function<void(thor::ActionContext<UserActions>)> onResize = [&level, &window_prev_size](thor::ActionContext<UserActions> context)
	{
		sf::Vector2f scale = { (float)window_prev_size.x / context.event->size.width, (float)window_prev_size.y / context.event->size.height };
		window_prev_size = { context.event->size.width, context.event->size.height };
		level.zoom(scale, true);
	};

	std::function<void(thor::ActionContext<UserActions>)> onZoom = [&level](thor::ActionContext<UserActions> context)
	{
		level.zoom(std::pow(1.3f, context.event->mouseWheelScroll.delta));
	};

	std::function<void()> onMove = [&level, &dt]()
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) level.scrollLeft(dt.asSeconds() * 1000);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) level.scrollRight(dt.asSeconds() * 1000);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) level.scrollUp(dt.asSeconds() * 1000);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) level.scrollDown(dt.asSeconds() * 1000);
	};

	std::function<void()> onRotate = [&level, &dt]()
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) level.rotate(dt.asSeconds() * -50);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) level.rotate(dt.asSeconds() * 50);
	};

	std::function<void()> onChangeScrollingType = [&level]()
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::I)) level.setScrollingType(InstantScrolling());
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) level.setScrollingType(LinearScrolling(sf::seconds(2)));
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) level.setScrollingType(ExponentialScrolling(sf::seconds(2), 10));
	};

	//bind callbacks to actions
	thor::ActionMap<UserActions>::CallbackSystem callback_system;
	callback_system.connect(UserActions::Resize, onResize);
	callback_system.connect(UserActions::Zoom, onZoom);
	callback_system.connect0(UserActions::Move, onMove);
	callback_system.connect0(UserActions::Rotate, onRotate);
	callback_system.connect0(UserActions::ChangeScrollingType, onChangeScrollingType);

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
		if (action_map.isActive(UserActions::Close)) 
			window.close();
		action_map.invokeCallbacks(callback_system, &window);

		level.update();

		full_time += (dt = deltaClock.restart());
		ImGui::SFML::Update(window, dt);

		ImGui::Begin("Info");
		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
		ImGui::Text("Background Position: %f, %f", ib.getPosition().x, ib.getPosition().y);
		ImGui::Text("Level current view pos: %f, %f", level.m_view.getCenter().x, level.m_view.getCenter().y);
		ImGui::Text("Level view destination pos: %f, %f", level.m_view_destination.getCenter().x, level.m_view_destination.getCenter().y);
		ImGui::Text("Level in scroll?: %i", level.m_in_scroll);
		ImGui::Text("Zoom: x%f", window.getView().getSize().x / window.getSize().x);
		ImGui::End();

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