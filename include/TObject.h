#ifndef TOBJECT_H
#define TOBJECT_H

extern "C" {

void tuner_debug_object_count (const char *msg);

}

class TObject;

typedef void (*TObjectCallback) (TObject *object, const char *msg, const void *data,
		void *user_data);

class TObject
{
private:
	unsigned int id_;
	const char *name_;
	TObjectCallback callback_func_;
	void *callback_func_user_data_;

protected:
	virtual void onMove (int x, int y);
	virtual void onResize (int w, int h);
	virtual void onLayer (int layer);
	virtual void onOpacity (int alpha);
	virtual void onShow (bool smooth);
	virtual void onHide (bool smooth);
	virtual void onUpdate ();

	bool debug_enable;
	Evas_Object *parent_eo_;
	bool visibility_;
	Evas_Coord_Rectangle rect_;
	int layer_;
	int opacity_;

public:
	explicit TObject (Evas_Object *parent_eo, const char *name);
	explicit TObject (const char *name);
	virtual ~TObject ();

	int getId ();
	const char *getName ();
	Evas_Object* getParentEvasObject (void);
	TObject& debug (bool enable);

	TObject& update ();

	TObject& rectWith (TObject *target);
	TObject& getGeometry (int *x, int *y, int *w, int *h)
	{
		if (x)
			*x = this->rect_.x;
		if (y)
			*y = this->rect_.y;
		if (w)
			*w = this->rect_.w;
		if (h)
			*h = this->rect_.h;

		return *this;
	}

	/*
	 * Visibility
	 */
	bool isVisible () const;
	virtual void show (bool smooth = true);
	virtual void hide (bool smooth = true);

	/*
	 * Size
	 */
	TObject& resizeTo (int w, int h);
	TObject& resizeBy (int w, int h);
	TObject& resizeWith (Evas_Object *eo);
	TObject& resizeWith (TObject *target);
	inline int width () const
	{
		return rect_.w;
	}
	inline int height () const
	{
		return rect_.h;
	}

	/*
	 * Position
	 */
	TObject& moveTo (int x, int y);
	TObject& moveXTo (int x);
	TObject& moveYTo (int y);
	TObject& moveBy (int x, int y);
	TObject& moveWith (Evas_Object *eo);
	TObject& moveToAngle (double angle, int r, int cx, int cy, bool center =
			true);
	TObject& moveToAngle (double angle, int r, TObject *base,
			bool center = true);
	inline int end_x () const
	{
		return rect_.x + rect_.w;
	}
	inline int end_y () const
	{
		return rect_.y + rect_.h;
	}
	inline int center_x () const
	{
		return rect_.x + rect_.w / 2;
	}
	inline int center_y () const
	{
		return rect_.y + rect_.h / 2;
	}
	inline int x () const
	{
		return rect_.x;
	}
	inline int y () const
	{
		return rect_.y;
	}

	/*
	 * Layer
	 */
	inline int getLayer ()
	{
		return layer_;
	}
	TObject& layerTo (int layer);
	TObject& layerBy (int layer);

	/*
	 * Opacity
	 */
	inline int getOpacity ()
	{
		return opacity_;
	}
	TObject& opacityTo (int alpha);

	TObject& setCallback (TObjectCallback func, void *user_data = NULL);
	TObject& emit (const char *msg, const void *data = NULL);
};

#endif
