#pragma once
#include <string>

#include <SFML/Graphics.hpp>
#include <Thor/Resources.hpp>

#include <drawables/ImageBackground.hpp>

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

//class for creating scrolling environment for creating (for example) levels
class Level : public sf::Drawable
{
public:
	Level();
	void setWindow(sf::RenderWindow* window);
	sf::RenderWindow* getWindow() const;
	void setBackground(const ImageBackground &background);
	const ImageBackground& getBackground() const;
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
	void setScrollingType(T type)
	{
		m_scrolling_type_ptr = std::make_unique<T>(type);
	}
	std::string getScrollingTypeName() const;
	void update();
	void updateScrolling();
	sf::View getCurrentView() const;
	
	sf::View m_view{}, m_view_destination{};
	std::unique_ptr<ViewScrolling> m_scrolling_type_ptr{nullptr};
	sf::Clock m_scroll_timer;
	bool m_in_scroll{false};
	sf::RenderWindow* m_window_ptr{nullptr};
	ImageBackground m_background{nullptr, sf::FloatRect()};
	thor::ResourceHolder<sf::Texture, std::string> m_texture_holder;

private:
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