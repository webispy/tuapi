#ifndef TMENU_H
#define TMENU_H

// #include <string>
#include <TObject.h>
#include <TImage.h>

class TMenu;

class TMenuItem: public TObject
{
private:
	bool _enabled;
	bool _focused;
	bool _checked;

protected:
	char *_title;
	TStateImage _icon;
	TImage *_background;
	TImage *_text;
	TMenu *_parent_menu;
	virtual void onMove (int x, int y);
	virtual void onResize (int w, int h);
	virtual void onLayer (int layer);

public:
	explicit TMenuItem (TMenu *parent_menu, const char *title,
			const char *object_name = "TMenuItem");
	explicit TMenuItem (TMenu *parent_menu, const char *object_name =
			"TMenuItem");
	virtual ~TMenuItem ();

	virtual TMenuItem& setIcon (const char *path, enum timage_state_types type =
			STATE_DEFAULT);
	virtual TMenuItem& setIcon (TImage *icon, enum timage_state_types type =
			STATE_DEFAULT);
	inline TStateImage *getIcon ()
	{
		return &_icon;
	}

	virtual void show (bool smooth = true);
	virtual void hide (bool smooth = true);
	virtual void update ();

	virtual TMenuItem& setTitle (const char *title, unsigned int rgba =
			0x53FFC6FF, int size = 28 * elm_config_scale_get()); const char *getTitle ();
	inline TImage *getText ()
	{
		return _text;
	}

	virtual TMenuItem& enable ();
	virtual TMenuItem& disable ();
	bool isEnabled () const;

	virtual TMenuItem& focus ();
	virtual TMenuItem& unfocus ();
	bool isFocused () const;

	virtual TMenuItem& check ();
	virtual TMenuItem& uncheck ();
	bool isChecked () const;

	bool isCursor ();
};

class TMenu: public TObject
{
public:
	enum rotation_direction
	{
		CW = 0, COUNTER_CW = 1, UNKNOWN = 2
	};

protected:
	std::vector<TMenuItem *> items;
	std::vector<TMenuItem *>::iterator prev_cursor;
	std::vector<TMenuItem *>::iterator cursor;
	TImage *bg;

	double base_angle;
	double step_angle;
	double radius;
	int icon_size;
	int default_bg_opacity;
	enum rotation_direction dir;

public:
	explicit TMenu (Evas_Object *parent, const char *object_name = "TMenu");
	virtual ~TMenu ();

	virtual TMenu& append (TMenuItem *item, bool to_bottom = TRUE);
	virtual TMenu& update ();

	virtual void show (bool smooth = true);
	virtual void hide (bool smooth = true);

	TMenuItem* getCursor ();
	int getCursorIndex ();
	int getItemCount ();

	virtual TMenu& setCursor (int index, enum rotation_direction dir = UNKNOWN);
	virtual TMenu& next (bool circular = false);
	virtual TMenu& prev (bool circular = false);
	virtual TMenu& refresh ();

	TMenu& setBackground (const char *path, int default_opacity = 64);
	TMenu& setBackground (TImage *img, int default_opacity = 64);
	TMenu& setBackgroundColor (int r, int g, int b, int a);
	TImage* getBackground ();

	inline double getStepAngle ()
	{
		return step_angle;
	}
	inline void setStepAngle (double angle)
	{
		step_angle = angle;
	}
	inline void setIconSize (int size)
	{
		icon_size = size;
	}
	inline void setRadius (double r)
	{
		radius = r;
	}
	inline double getRadius ()
	{
		return radius;
	}
	inline int getIconSize ()
	{
		return icon_size;
	}
	inline enum TMenu::rotation_direction getRotationDirection ()
	{
		return dir;
	}
};

#endif
