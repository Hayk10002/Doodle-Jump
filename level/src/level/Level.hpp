#pragma once
#include <string>
#include <unordered_set>
#include <deque>

#include <SFML/Graphics.hpp>
#include <Thor/Resources.hpp>


//abstract class for all scrolling algorithms
class ViewScrolling
{
protected:

	static sf::View lerp(const sf::View& start, const sf::View& end, float alpha);
	sf::Time m_duration;
	std::string m_name = "None";
	ViewScrolling(sf::Time duration);
public:
	virtual sf::View getView(sf::Time passed_time, const sf::View& start_view, const sf::View& end_view) const = 0;
	std::string getName() const;
	sf::Time getDuration() const;
};

class LevelObject
{

	LevelObject(std::shared_ptr<const size_t*> level);

	std::shared_ptr<const size_t*> level;
	sf::Drawable* drawable_ptr{};
	std::function<void(sf::Time)> update{};
	size_t identifier{};
	inline static size_t identifier_counter{};
public:

	LevelObject(const LevelObject& other);
	LevelObject get_duplicate();
	bool operator==(LevelObject other) const;
	LevelObject& operator=(const LevelObject& other);
	friend class Level;
};

template<>
struct std::hash<LevelObject>
{
	size_t operator()(const LevelObject& obj) const
	{
		return _Hash_array_representation<char>((char*)&obj, sizeof(LevelObject));
	}
};

//class for creating scrolling environment for creating (for example) levels
class Level : public sf::Drawable
{
public:

	Level();
	Level(const Level&) = default;
	Level(Level&&) = default;

	~Level();

	void setWindow(sf::RenderWindow* window);
	sf::RenderWindow* getWindow() const;

	

	template<std::derived_from<sf::Drawable> T>
	LevelObject addObject(T& obj)
	{
		LevelObject object(std::make_shared<const size_t*>(&m_identifier));
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
	bool removeObject(LevelObject obj);

	bool moveObjectUpInUpdateOrder(LevelObject obj, int count = 1);
	bool moveObjectDownInUpdateOrder(LevelObject obj, int count = 1);
	bool updateFirst(LevelObject obj);
	bool updateLast(LevelObject obj);
	bool addToUpdateList(LevelObject obj, int position = -1);
	bool removeFromUpdateList(LevelObject obj);
	bool moveObjectUpInDrawOrder(LevelObject obj, int count = 1);
	bool moveObjectDownInDrawOrder(LevelObject obj, int count = 1);
	bool drawFirst(LevelObject obj);
	bool drawLast(LevelObject obj);
	bool addToDrawList(LevelObject obj, int position = -1);
	bool removeFromDrawList(LevelObject obj);

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

	void update(sf::Time dt);
	void updateScrolling();

	sf::View getCurrentView() const;

private:


	sf::View m_view{}, m_view_destination{};
	std::unique_ptr<ViewScrolling> m_scrolling_type_ptr{nullptr};
	std::unordered_set<LevelObject> m_objects{};
	std::deque<LevelObject> m_update_order, m_draw_order;
	sf::Clock m_scroll_timer;
	bool m_in_scroll{false};
	sf::RenderWindow* m_window_ptr{nullptr};

	inline static size_t identifier_counter{ 0 };
	const size_t m_identifier{ identifier_counter++ };
	bool isMyObject(LevelObject obj);

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

struct InstantScrolling : ViewScrolling
{
	InstantScrolling();
	sf::View getView(sf::Time passed_time, const sf::View& start_view, const sf::View& end_view) const override;
};

struct LinearScrolling : ViewScrolling
{
	LinearScrolling(sf::Time duration);
	sf::View getView(sf::Time passed_time, const sf::View& start_view, const sf::View& end_view) const override;
};

struct ExponentialScrolling : ViewScrolling
{
	ExponentialScrolling(sf::Time duration, float power);
	sf::View getView(sf::Time passed_time, const sf::View& start_view, const sf::View& end_view) const override;
private:
	float m_polynolial_power{ 3 }, m_exponent_base, m_turning_point, m_polynomial_scale;
};