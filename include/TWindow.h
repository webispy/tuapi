#ifndef INCLUDE_TWINDOW_H_
#define INCLUDE_TWINDOW_H_

#include <TObject.h>

class TWindow: public TObject
{
protected:
	Evas_Object *eo;
	Evas_Object *eo_bg;
	Evas_Object *eo_conform;
	Evas_Object *eo_gesture;
	Evas_Object *eo_rect;

	Ecore_Timer *etimer;
	Evas_Object *eo_clip;
	Evas_Object *eo_time;
	Evas_Object *eo_time_bg;
	gboolean is_time_show;
	gboolean is_time_bg_show;
	int prev_hour;
	int prev_min;

	virtual void onShow(bool smooth);
	virtual void onHide(bool smooth);
	virtual void onMove(int x, int y);
	virtual void onResize(int w, int h);
	virtual void onWheel(int dir);
	virtual void onTap(int x, int y, unsigned int n, unsigned int timestamp);
	virtual void onBack();
	virtual void onSeconds(time_t tt, struct tm *loc_time);

public:
	explicit TWindow(const char *object_name = "TWindow");
	virtual ~TWindow();

	void lower();
	void raise();

	void setBackgroundColor(int r, int g, int b, int a);
	void setTitle(const char *title);

	void setTimeBgColor(int r, int g, int b, int a = 200);
	void setTimeColor(int r, int g, int b, int a = 255);
	void setTimeVisible(bool time_visible, bool time_bg_visible = true);

	Evas_Object *getEvasObject();
	Evas_Object *getContainer();
};

#endif /* INCLUDE_TWINDOW_H_ */
