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
#include <drawables/ImageBackground.hpp>
#include <level/level.hpp>
#include <gameObjects/Doodle.hpp>


enum class UserActions
{
	None,
	Close,
	Resize,
	Jump, 
	Left, 
	Right,
	Shoot
};

//class DoodleWithStats : public Doodle
//{
//public:
//	using Doodle::Doodle;
//	using Doodle::m_velocity;
//	using Doodle::m_gravity;
//	using Doodle::m_texture;
//	using Doodle::m_body;
//	using Doodle::m_body_normal_exhind;
//	using Doodle::m_body_shooting_exhind;
//	using Doodle::m_feet;
//	using Doodle::m_feet_normal_exhind;
//	using Doodle::m_feet_shooting_exhind;
//	using Doodle::m_nose;
//	using Doodle::m_nose_exhind;
//	using Doodle::m_body_status;
//	using Doodle::m_is_jumping;
//	using Doodle::m_jumping_clock;
//	using Doodle::m_jumping_interval;
//	using Doodle::m_is_shooting;
//	using Doodle::m_shooting_clock;
//	using Doodle::m_shooting_interval;
//	using Doodle::m_nose_angle;
//	using Doodle::m_max_nose_angle_dev;
//	using Doodle::m_nose_rotation_center;
//	using Doodle::m_nose_not_shooting_offset;
//	using Doodle::m_nose_distance_from_rotation_center;
//	using Doodle::m_jumping_speed;
//	using Doodle::m_moving_speed;
//	using Doodle::m_speed_change_rate;
//	using Doodle::m_speed_decreasing_rate;
//	using Doodle::m_texture_scale;
//	using Doodle::m_feet_offset;
//	using Doodle::m_feet_jumping_offset;
//	using Doodle::m_area;
//	using Doodle::m_is_fallen_out;
//	using Doodle::m_is_too_high;
//	using Doodle::jump;
//	using Doodle::draw;
//};

int main()
{
	//setup the window
	sf::RenderWindow window(sf::VideoMode(800, 800), "Doodle Jump");
	sf::Vector2u window_prev_size(window.getSize());

	//setup Dear ImGui
	ImGui::SFML::Init(window);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

	//setup thor resource manager
	init_resources();

	//create the background
	ImageBackground ib(&global_textures["background"], &window);
	
	Doodle doodle({ 400, 400 });

	//create level
	Level level;
	Level::Object ib_obj = level.addObject(ib);
	Level::Object doodle_obj = level.addObject(doodle);
	level.setWindow(&window);
	level.setScrollingType(InstantScrolling());

	//setup the user actions
	thor::ActionMap<UserActions> action_map;
	action_map[UserActions::Close] = thor::Action(sf::Event::Closed);
	action_map[UserActions::Resize] = thor::Action(sf::Event::Resized);
	action_map[UserActions::Jump] = thor::Action(sf::Keyboard::W, thor::Action::PressOnce);
	action_map[UserActions::Left] = thor::Action(sf::Keyboard::A, thor::Action::Hold);
	action_map[UserActions::Right] = thor::Action(sf::Keyboard::D, thor::Action::Hold);
	action_map[UserActions::Shoot] = thor::Action(sf::Mouse::Left, thor::Action::PressOnce);

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
		sf::Vector2f position = doodle.getPosition();
		ImGui::DragFloat2("Position", (float*)&position);
		doodle.setPosition(position);
		ImGui::Text("Is jumping: %b", doodle.isJumping());
		ImGui::Text("Is shooting: %b", doodle.isShooting());
		ImGui::Text("Area: {%f, %f, %f, %f}", doodle.getArea().left, doodle.getArea().top, doodle.getArea().width, doodle.getArea().height);
		ImGui::Text("Is fallen out: %b", doodle.isFallenOutOfScreen());
		ImGui::Text("Is too high: %b", doodle.isTooHigh());
		if (ImGui::IsWindowFocused())  dt = sf::Time::Zero;
		ImGui::End();

		if (action_map.isActive(UserActions::Close))
			window.close();
		if (action_map.isActive(UserActions::Resize))
		{
			sf::Vector2f scale = { (float)window_prev_size.x / window.getSize().x, (float)window_prev_size.y / window.getSize().y };
			window_prev_size = { window.getSize().x, window.getSize().y };
			level.zoom(scale, true);
		}
		if (action_map.isActive(UserActions::Jump))
			doodle.jump();
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

		doodle.updateArea(sf::FloatRect{ window.mapPixelToCoords({0, 0}), window.mapPixelToCoords(sf::Vector2i{window.getSize()}) - window.mapPixelToCoords({0, 0}) });
		if (doodle.isFallenOutOfScreen()) doodle.setPosition(window.mapPixelToCoords({ 400, 400 }));
		if (doodle.isTooHigh()) level.scrollUp(doodle.getArea().top - doodle.getPosition().y);
		level.update(dt);


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