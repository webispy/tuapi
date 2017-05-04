#ifndef TSCALEROTATIONMENU_H
#define TSCALEROTATIONMENU_H

#include <TMenu.h>

enum icon_vertical_align
{
	ALIGN_TOP, ALIGN_MIDDLE, ALIGN_BOTTOM
};

class TScaleRotationMenuItem: public TMenuItem
{
public:
	struct moving_state angle;
	struct moving_state scale;
	struct moving_state text_y;
	struct moving_state opacity;
	struct moving_state opacity_icon;
	int radius;

	explicit TScaleRotationMenuItem (TMenu *parent_menu, const char *title,
			const char *object_name = "TScaleRotationMenuItem");
	virtual ~TScaleRotationMenuItem ();

	void hide (bool smooth = true);
	void update ();

	TScaleRotationMenuItem& focus ();
	TScaleRotationMenuItem& unfocus ();

	TScaleRotationMenuItem& stop ();
};

class TScaleRotationMenu: public TMenu
{
protected:
	struct moving_state _angle;
	struct moving_state _scale;
	TBezierCubicEaseOut motion;
	Ecore_Animator *move_anim;
	Ecore_Animator *showhide_anim;
	enum icon_vertical_align valign;
	static Eina_Bool on_move_anim (void *user_data, double pos);
	static Eina_Bool on_show_anim (void *user_data, double pos);
	static Eina_Bool on_hide_anim (void *user_data, double pos);

public:
	explicit TScaleRotationMenu (Evas_Object *parent, const char *object_name =
			"TScaleRotationMenu");
	virtual ~TScaleRotationMenu ();
	virtual TScaleRotationMenu& update ();
	virtual TScaleRotationMenu& setCursor (int index,
			enum rotation_direction dir = UNKNOWN);

	virtual void show (bool smooth = true);
	virtual void hide (bool smooth = true);

	inline void setIconMinimizeScale (double scale)
	{
		_scale.start = scale;
	}
	inline enum icon_vertical_align getVAlign ()
	{
		return valign;
	}
	inline void setVAlign (enum icon_vertical_align va)
	{
		valign = va;
	}
};

#endif
