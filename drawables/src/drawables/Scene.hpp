#pragma once
#include <string>
#include <unordered_set>
#include <deque>

#include <SFML/Graphics.hpp>
#include <Thor/Resources.hpp>


class SimpleView
{
	using Vector2f = sf::Vector2f;
	using FloatRect = sf::FloatRect;

	Vector2f m_center{};
	Vector2f m_size{};
	float m_rotation{0};
	FloatRect m_viewport{ {0, 0}, {1, 1} };

public:
	SimpleView();
	SimpleView(const sf::View& view);
	explicit SimpleView(const FloatRect& rectangle);
	SimpleView(const Vector2f& center, const Vector2f& size);
	void setCenter(float x, float y);
	void setCenter(const Vector2f& center);
	void setSize(float width, float height);
	void setSize(const Vector2f& size);
	void setRotation(float angle);
	void setViewport(const FloatRect& viewport);
	void reset(const FloatRect& rectangle);
	const Vector2f& getCenter() const;
	const Vector2f& getSize() const;
	float getRotation() const;
	const FloatRect& getViewport() const;
	void move(float offsetX, float offsetY);
	void move(const Vector2f& offset);
	void rotate(float angle);
	void zoom(float factor);
	sf::View asSFMLView() const;
};

//abstract class for all scrolling algorithms
class ViewScrolling
{
protected:

	static SimpleView lerp(const SimpleView& start, const SimpleView& end, float alpha);
	sf::Time m_duration;
	std::string m_name = "None";
	ViewScrolling(sf::Time duration);
public:
	virtual SimpleView getView(sf::Time passed_time, const SimpleView& start_view, const SimpleView& end_view) const = 0;
	std::string getName() const;
	sf::Time getDuration() const;
};

//class for creating scrolling environment for creating scenes
class Scene : public sf::Drawable
{
public:

	Scene();
	Scene(const Scene&) = default;
	Scene(Scene&&) = default;

	~Scene();

	void setWindow(sf::RenderWindow* window);
	sf::RenderWindow* getWindow() const;

	class Object
	{

		Object(std::shared_ptr<const size_t*> scene_identifier);

		std::shared_ptr<const size_t*> scene_identifier;
		sf::Drawable* drawable_ptr{};
		std::function<void(sf::Time)> update{};
		size_t identifier{};
		inline static size_t identifier_counter{};

		struct Hasher
		{
			size_t operator()(const Object& obj) const;
		};

	public:

		Object(const Object& other);
		Object get_duplicate() const;
		template<class F>
			requires std::invocable<F> || std::invocable<F, sf::Time>
		void setUpdate(F&& update_function)
		{
			if constexpr (requires { update_function(sf::Time::Zero); })
				update = [&update_function](sf::Time t) {update_function(t); };
			else if constexpr (requires { update_function(); })
				update = [&update_function](sf::Time) {update_function(); };
			else update = [](sf::Time) {};
		}
		bool operator==(Object other) const;
		friend class Scene;
	};


	template<std::derived_from<sf::Drawable> T>
	Object addObject(T& obj)
	{
		Object object(std::make_shared<const size_t*>(&m_identifier));
		object.drawable_ptr = &obj;
		if constexpr (requires { obj.update(sf::Time::Zero); })
			object.update = [&obj](sf::Time t) {obj.update(t); };
		else if constexpr (requires { obj.update(); })
			object.update = [&obj](sf::Time) {obj.update(); };
		else object.update = [](sf::Time) {};

		m_objects.insert(object);
		addToUpdateList(object);
		addToDrawList(object);

		return object;
	}

	template<std::derived_from<sf::Drawable> T, class F>
		requires std::invocable<F> || std::invocable<F, sf::Time>
	Object addObject(T& obj, F&& update)
	{
		Object object(std::make_shared<const size_t*>(&m_identifier));
		object.drawable_ptr = &obj;
		if constexpr (std::invocable<F, sf::Time>)
			object.update = [&update](sf::Time t) { update(t); };
		else if constexpr (std::invocable<F>)
			object.update = [&update](sf::Time) { update(); };
		else object.update = [](sf::Time) {};

		m_objects.insert(object);
		addToUpdateList(object);
		addToDrawList(object);

		return object;
	}
	bool removeObject(Object obj);

	bool moveObjectUpInUpdateOrder(Object obj, int count = 1);
	bool moveObjectDownInUpdateOrder(Object obj, int count = 1);
	bool updateFirst(Object obj);
	bool updateLast(Object obj);
	bool addToUpdateList(Object obj, int position = -1);
	bool isInUpdateList(Object obj);
	bool removeFromUpdateList(Object obj);
	bool moveObjectUpInDrawOrder(Object obj, int count = 1);
	bool moveObjectDownInDrawOrder(Object obj, int count = 1);
	bool drawFirst(Object obj);
	bool drawLast(Object obj);
	bool addToDrawList(Object obj, int position = -1);
	bool isInDrawList(Object obj);
	bool removeFromDrawList(Object obj);

	void scrollUp(float offset, bool instant = false);
	void scrollDown(float offset, bool instant = false);
	void scrollLeft(float offset, bool instant = false);
	void scrollRight(float offset, bool instant = false);
	void scroll(float offset_x, float offset_y, bool instant = false);
	void scroll(sf::Vector2f offset, bool instant = false);
	void zoomX(float scale, bool instant = false);
	void zoomY(float scale, bool instant = false);
	void zoom(float scale, bool instant = false);
	void zoom(float scale_x, float scale_y, bool instant = false);
	void zoom(sf::Vector2f scale, bool instant = false);
	void rotate(float angle, bool instant = false);

	template<class T> requires std::derived_from<T, ViewScrolling>
	void setScrollingType(const T& type)
	{
		m_scrolling_type_ptr = std::make_unique<T>(type);
	}
	std::string getScrollingTypeName() const;

	void updateObjects(sf::Time dt);
	void updateScrolling();

	SimpleView getCurrentView() const;

private:

	std::deque<Object>::iterator getObjectsItrForUpdateList(Object obj);
	std::deque<Object>::iterator getObjectsItrForDrawList(Object obj);
	std::deque<Object>::const_iterator getObjectsItrForUpdateList(Object obj) const;
	std::deque<Object>::const_iterator getObjectsItrForDrawList(Object obj) const;

	SimpleView m_view{}, m_view_destination{};
	std::unique_ptr<ViewScrolling> m_scrolling_type_ptr{nullptr};
	std::unordered_set<Object, Object::Hasher> m_objects{};
	std::deque<Object> m_update_order;
	mutable std::deque<Object> m_draw_order;
	sf::Clock m_scroll_timer;
	bool m_in_scroll{false};
	sf::RenderWindow* m_window_ptr{nullptr};

	inline static size_t identifier_counter{ 0 };
	const size_t m_identifier{ identifier_counter++ };
	bool isMyObject(Object obj) const;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

struct InstantScrolling : ViewScrolling
{
	InstantScrolling();
	SimpleView getView(sf::Time passed_time, const SimpleView& start_view, const SimpleView& end_view) const override;
};

struct LinearScrolling : ViewScrolling
{
	LinearScrolling(sf::Time duration);
	SimpleView getView(sf::Time passed_time, const SimpleView& start_view, const SimpleView& end_view) const override;
};

struct ExponentialScrolling : ViewScrolling
{
	ExponentialScrolling(sf::Time duration, float power);
	SimpleView getView(sf::Time passed_time, const SimpleView& start_view, const SimpleView& end_view) const override;
private:
	float m_polynolial_power{ 3 }, m_exponent_base, m_turning_point, m_polynomial_scale;
};