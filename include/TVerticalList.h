#ifndef TVERTICAL_LIST_H
#define TVERTICAL_LIST_H

#include <string>
#include <TContainer.h>
#include <TImage.h>

class TVerticalList;

class TVerticalListItem: public TObject
{
protected:
	char *_title;
	TImage *_icon;
	TImage *_background;
	TImage *_text;
	TVerticalList *_parent_menu;
	TImage *_content;
	virtual void update_size ();
	virtual void onShow (bool smooth);
	virtual void onHide (bool smooth);
	virtual void onUpdate ();

public:
	struct moving_state y_pos;
	struct moving_state y_pos_scroll;
	struct moving_state scale;
	struct moving_state opacity;
	double scroll_max;

	explicit TVerticalListItem (TVerticalList *parent_menu, const char *title,
			const char *subtitle = NULL, const char *object_name =
					"TVerticalListItem");
	virtual ~TVerticalListItem ();

	virtual TVerticalListItem& setTitle (const char *title, unsigned int rgba =
			0xFFFFFFFF, int size = 34 * elm_config_scale_get());

	virtual TVerticalListItem& setText (const char *title,
			const char *subtitle = NULL, unsigned int rgba = 0xFFFFFFFF,
			unsigned int sub_rgba = 0xE0E0E0FF);

	virtual TVerticalListItem& setIcon (const char *path);
	virtual TVerticalListItem& setIcon (TImage *icon);

	inline const char *getTitle ()
	{
		return _title;
	}
	inline TImage *getIcon ()
	{
		return _icon;
	}
	inline TImage *getText ()
	{
		return _text;
	}
};

class TVerticalList: public TContainer
{
protected:
	TImage *bg_;
	TImage *cursor_bg_;
	struct moving_state bg_sy, bg_ey;
	TTransition *tr_bounce_top;
	TTransition *tr_bounce_bottom;
	bool is_cursor_pos_middle_;
	int cursor_pos_offset_;
	int item_min_height;
	int cursor_mode;
	TImage *title_area;
	virtual void onUpdate ();
	virtual void onShow (bool smooth);
	virtual void onHide (bool smooth);
	virtual void onAdded (TObject *item);
	virtual void onResize (int w, int h);
	virtual void onCursorChanged (GList *prev);
	void item_scroll ();

protected:
	Ecore_Animator *move_anim;
	static Eina_Bool on_move_anim (void *user_data, double pos);
	static Eina_Bool on_item_scroll_anim (void *user_data, double pos);
	static Eina_Bool __anim_bounce_top (TTransition *t, TObject *target,
			double pos, double frame, void *user_data);
	static Eina_Bool __anim_bounce_bottom (TTransition *t, TObject *target,
			double pos, double frame, void *user_data);

public:
	int padsize;
	TImage *_more_top_bg;
	TImage *_more_bottom_bg;
	struct moving_state scroll_y;

	explicit TVerticalList (Evas_Object *parent);
	virtual ~TVerticalList ();
	virtual TVerticalList& next (bool circular = false);
	virtual TVerticalList& prev (bool circular = false);
	TVerticalList& refresh ();

	void setAlign (bool isAlignMiddle, int offset = 0);
	void setPadsize (int padsize = 30 * elm_config_scale_get());
	void setItemMinHeight (int min_height = 0);
	void setBackgroundColor (int r, int g, int b, int a);
	void setBackground (const char *path);
	void setBackground (TImage *img);
	inline TImage *getBackground ()
	{
		return bg_;
	}
	void setCursorColor (int r, int g, int b, int a);

	void show_cursor_bg ();
	void hide_cursor_bg ();

	void setTitle (const char *title, unsigned int rgba = 0xFFFFFFFF,
			unsigned int rgba_bg = 0x7DD535FF, gboolean use_bg_pattern = TRUE);
};

#endif
