#ifndef TIMAGE_H
#define TIMAGE_H

#include <vector>
#include <TObject.h>
#include <TTransition.h>

class TImage;

class TTransform
{
private:
	TImage *target;
	Evas_Map *map;
	Evas_Coord_Rectangle show_rect;
	Evas_Coord_Rectangle src_rect;
	double x_angle, y_angle, z_angle;
	double x_scale, y_scale;

public:
	explicit TTransform (TImage *img);
	virtual ~TTransform ();

	Evas_Map* getMap ();
	TTransform& reset ();
	TTransform& move (int x, int y, bool relative = false);
	TTransform& moveX (int x, bool relative = false);
	TTransform& moveY (int y, bool relative = false);
	TTransform& moveToAngle (double angle, int r, int cx, int cy, bool center =
			true);

	TTransform& moveToAngle (double angle, int r, TObject *base, bool center =
			true)
	{
		return moveToAngle(angle, r, base->center_x(), base->center_y(), center);
	}

	TTransform& resize (int w, int h, bool relative = false);
	TTransform& scale (double ratio);
	TTransform& scale (double ratio, int cx, int cy);
	TTransform& scale (double x_ratio, double y_ratio);
	TTransform& scale (double x_ratio, double y_ratio, int cx, int cy);
	TTransform& rotate (double angle, bool relative = false);
	TTransform& rotate (double angle, TObject *base, bool relative = false);
	TTransform& rotate (double angle, int cx, int cy, bool relative = false);
	TTransform& rotateX (double angle, bool relative = false);
	TTransform& rotateX (double angle, int cx, int cy, int cz, bool relative);
	TTransform& rotateY (double angle, bool relative = false);
	TTransform& rotateY (double angle, int cx, int cy, int cz, bool relative);

	TTransform& selectSourcePos (int x, int y);
	TTransform& selectSourceSize (int w, int h);

	inline int getCX ()
	{
		return show_rect.x + show_rect.w / 2;
	}
	inline int getCY ()
	{
		return show_rect.y + show_rect.h / 2;
	}
	inline int getWidth ()
	{
		return show_rect.w;
	}
	inline int getHeight ()
	{
		return show_rect.h;
	}
	inline int getX ()
	{
		return show_rect.x;
	}
	inline int getY ()
	{
		return show_rect.y;
	}
	inline double getXScale()
	{
		return x_scale;
	}
	inline double getYScale()
	{
		return y_scale;
	}

	void apply (void);
};

class TImage: public TObject
{
private:
	int stride;
	TTransform *map;
	unsigned char *rawdata;
	Evas_Coord_Rectangle real_size;
	std::vector<Evas_Object *> items;
	std::vector<Evas_Object *>::iterator cursor;
	Evas_Object *cursor_object;

	int cur_frame;
	int max_frame;
	int animation_start_frame;
	Ecore_Timer *animation_timer;
	bool _stop_request;
	bool _repeat;
	int prev_blur;
	static Eina_Bool on_animation (void *user_data);

	virtual void onMove (int x, int y);
	virtual void onResize (int w, int h);
	virtual void onLayer (int layer);

public:
	explicit TImage (Evas_Object *parent);
	explicit TImage (Evas_Object *parent, const char *path);
	explicit TImage (Evas_Object *parent, Evas_Object *proxy_src,
			bool is_proxy = false);
	explicit TImage (Evas_Object *parent, void *data, int size, char *format);
	explicit TImage (Evas_Object *parent, int width, int height);
	explicit TImage (Evas_Object *parent, TImage *orig);
	virtual ~TImage ();

	TImage& replace (const char *path);
	TImage& replace (const char *path, int frame);
	TImage& append (const char *path);

	void show (bool smooth = true);
	void hide (bool smooth = true);
	TImage& moveToAngle (double angle, int r, bool center = true);

	int getRealWidth ();
	int getRealHeight ();

	TImage& copy(Evas_Object *src);
	TImage& setOpacity (int alpha, bool update_src_data = false);
	TImage& setBlur (int radius, bool with_alpha = true);
	TImage& fillTo (unsigned int rgba, bool circle = false);
	TImage& fillTo (int r, int g, int b, int a, bool circle = false);

	TTransform& getTransform ();

	bool isAnimationSupport () const;
	bool isAnimationStarted () const;
	int getFrameIndex ();
	int getFrameCount ();
	double getFrameDuration ();
	TImage& setFrame (int frame_index);
	TImage& nextFrame (bool rewind = true);
	TImage& nextAnimationFrame (bool rewind = true);
	TImage& setStartFrame (int frame_index);
	TImage& startAnimation (bool repeat_request = true);
	TImage& stopAnimation (Transition::position after_pos = Transition::START,
			bool rightnow = true);

	Evas_Object *getFrameSource ();

	friend class TTransform;
};

enum timage_state_types
{
	STATE_DEFAULT,
	STATE_FOCUSED,
	STATE_PRESSED,
	STATE_DISABLED,
	STATE_CHECKED_DEFAULT,
	STATE_CHECKED_FOCUS,
	STATE_CHECKED_PRESSED,
	STATE_CHECKED_DISABLED,
	STATE_MAX
};

class TStateImage: public TObject
{
private:
	TImage *items[STATE_MAX];
	enum timage_state_types cur_state;

protected:
	virtual void onMove (int x, int y);
	virtual void onResize (int w, int h);
	virtual void onLayer (int layer);
	virtual void onShow (bool smooth);
	virtual void onHide (bool smooth);

public:
	TStateImage ();
	virtual ~TStateImage ();

	void setImage (TImage *img, enum timage_state_types type);
	TImage *getImage (enum timage_state_types type);

	void setState (enum timage_state_types type);
	enum timage_state_types getState ();
	TImage *getStateImage ();

};

#endif
