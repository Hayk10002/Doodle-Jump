#include <functional>

#include <SFML/Graphics.hpp>

#include <imgui.h>
#include <SFML/imgui-SFML.h>

#include <Thor/Input.hpp>
#include <Thor/Resources.hpp>
#include <SelbaWard.hpp>

#include <DoodleJumpConfig.hpp>
#include <common/Resources.hpp>
#include <drawables/ImageBackground.hpp>


enum class UserActions
{
	None,
	Close,
	Resize,
	Zoom,
	Move,
	Rotate
};



int main()
{
	//setup the window
	sf::RenderWindow window(sf::VideoMode(600, 800), "Doodle Jump");
	window.setFramerateLimit(60);

	//setup Dear ImGui
	ImGui::SFML::Init(window);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

	//setup thor resource manager
	init_resources();
	
	//create the background
	ImageBackground ib(&global_textures["background"], &window);
	
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

	//callback functions for user actions

	std::function<void(thor::ActionContext<UserActions>)> onResize = [&window](thor::ActionContext<UserActions> context)
	{
		sf::View v(sf::FloatRect(0, 0, context.event->size.width, context.event->size.height));
		window.setView(v);
	};

	std::function<void(thor::ActionContext<UserActions>)> onZoom = [&window](thor::ActionContext<UserActions> context)
	{
		sf::View v = window.getView();
		v.zoom(std::pow(1.3f, context.event->mouseWheelScroll.delta));
		window.setView(v);
	};

	std::function<void()> onMove = [&ib]()
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) ib.move(20, 0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) ib.move(-20, 0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) ib.move(0, 20);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) ib.move(0, -20);
		/*
		sf::View v = window.getView();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) v.move(-20, 0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) v.move(20, 0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) v.move(0, -20);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) v.move(0, 20);
		window.setView(v);
		*/
	};

	std::function<void()> onRotate = [&window]()
	{
		/*if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) ib.rotate(1);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) ib.rotate(-1);*/
		
		sf::View v = window.getView();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) v.rotate(-1);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) v.rotate(1);
		window.setView(v);
		
	};

	//bind callbacks to actions
	thor::ActionMap<UserActions>::CallbackSystem callback_system;
	callback_system.connect(UserActions::Resize, onResize);
	callback_system.connect(UserActions::Zoom, onZoom);
	callback_system.connect0(UserActions::Move, onMove);
	callback_system.connect0(UserActions::Rotate, onRotate);

	//frame clock
	sf::Clock deltaClock;

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

		ib.update();

		ImGui::SFML::Update(window, deltaClock.restart());

		ImGui::Begin("Info");
		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
		ImGui::Text("Background Position %f, %f", ib.getPosition().x, ib.getPosition().y);
		ImGui::Text("Zoom x%f", window.getView().getSize().x / window.getSize().x);
		ImGui::End();

		//drawing
		window.clear();
		window.draw(ib);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
	release_resources();
	return 0;
}