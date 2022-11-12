#include <functional>

#include <SFML/Graphics.hpp>

#include <imgui.h>
#include <SFML/imgui-SFML.h>

#include <Thor/Input.hpp>
#include <Thor/Resources.hpp>

#include <DoodleJumpConfig.hpp>
#include <drawables/ImageBackground.hpp>


enum class UserActions
{
	None,
	Close,
	Resize,
	Zoom,
	Move
};



int main()
{
	sf::RenderWindow window(sf::VideoMode(600, 800), "Doodle Jump");
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);

	thor::ResourceHolder <sf::Texture, std::string> textures_holder;
	textures_holder.acquire("background", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"background.png"));
	ImageBackground ib(textures_holder["background"], &window);

	
	thor::ActionMap<UserActions> action_map;
	action_map[UserActions::Close] = thor::Action(sf::Event::Closed);
	action_map[UserActions::Resize] = thor::Action(sf::Event::Resized);
	action_map[UserActions::Zoom] = thor::Action(sf::Event::MouseWheelScrolled);
	action_map[UserActions::Move] =
		thor::Action(sf::Keyboard::A) ||
		thor::Action(sf::Keyboard::D) ||
		thor::Action(sf::Keyboard::W) ||
		thor::Action(sf::Keyboard::S);

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

	thor::ActionMap<UserActions>::CallbackSystem callback_system;
	callback_system.connect(UserActions::Resize, onResize);
	callback_system.connect(UserActions::Zoom, onZoom);
	callback_system.connect0(UserActions::Move, onMove);

	sf::Clock deltaClock;

	while (window.isOpen())
	{
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

		if (action_map.isActive(UserActions::Close)) window.close();
		action_map.invokeCallbacks(callback_system, &window);

		ib.update();

		ImGui::SFML::Update(window, deltaClock.restart());

		ImGui::Begin("Info");
		ImGui::Text("Background Position %f, %f", ib.getPosition().x, ib.getPosition().y);
		ImGui::End();

		window.clear();
		window.draw(ib);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
	return 0;
}