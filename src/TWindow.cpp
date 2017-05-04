#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "TLibrary.h"

//#define FEATURE_TOUCH
#define TIME_BG_HEIGHT      70

EXPORT_API TWindow::TWindow(const char *object_name) :
		TObject(object_name)
{
	eo = elm_win_add(NULL, object_name, ELM_WIN_BASIC);
	elm_win_autodel_set(eo, EINA_TRUE);
	elm_win_title_set(eo, object_name);
	elm_win_indicator_mode_set(eo, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(eo, ELM_WIN_INDICATOR_OPAQUE);

	eo_bg = elm_bg_add(eo);
	evas_object_name_set(eo_bg, "window_bg");
	evas_object_color_set(eo_bg, 0, 0, 0, 255);
	evas_object_size_hint_weight_set(eo_bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(eo, eo_bg);
	evas_object_show(eo_bg);

	eo_conform = elm_conformant_add(eo);
	evas_object_name_set(eo_bg, "window_conform");
	evas_object_size_hint_weight_set(eo_conform, EVAS_HINT_EXPAND,
	EVAS_HINT_EXPAND);
	elm_win_resize_object_add(eo, eo_conform);
	evas_object_show(eo_conform);

#ifdef HOST
	dbg("host mode");
	eo_clip = elm_image_add(eo);
	evas_object_name_set(eo_clip, "32767:clip");
	gchar *path = get_img_path("black_mask.png");
	elm_image_file_set(eo_clip, path, NULL);
	g_free(path);
	evas_object_show(eo_clip);
	evas_object_layer_set(eo_clip, TUNER_LAYER_CLIP);
#endif

	eo_time = evas_object_text_add(evas_object_evas_get(eo));
	evas_object_name_set(eo_time, "32766:time");
	evas_object_text_style_set(eo_time, EVAS_TEXT_STYLE_PLAIN);
#ifdef HOST
	evas_object_text_font_set(eo_time, FONT_NAME, 18);
#else
	evas_object_text_font_set(eo_time, FONT_NAME, 22);
#endif
	evas_object_layer_set(eo_time, TUNER_LAYER_TIME);

	eo_time_bg = evas_object_image_filled_add(
			evas_object_evas_get(eo));
	evas_object_image_alpha_set(eo_time_bg, EINA_TRUE);
	evas_object_name_set(eo_time_bg, "32765:time_bg");
	evas_object_image_size_set(eo_time_bg, DEFAULT_WIDTH, TIME_BG_HEIGHT);
	evas_object_repeat_events_set(eo_time_bg, EINA_FALSE);
	evas_object_move(eo_time_bg, 0, 0);
	evas_object_layer_set(eo_time_bg, TUNER_LAYER_TIME_BG);

	eo_rect = evas_object_rectangle_add(evas_object_evas_get(eo));
	evas_object_repeat_events_set(eo_rect, EINA_TRUE);
	evas_object_move(eo_rect, 0, 0);
	evas_object_color_set(eo_rect, 0, 0, 0, 0);
	evas_object_layer_set(eo_rect, EVAS_LAYER_MAX);
	evas_object_show(eo_rect);

#ifndef HOST
	eext_rotary_object_event_activated_set(eo, EINA_TRUE);
	eext_rotary_object_event_callback_add(eo,
			[](void *data, Evas_Object *obj, Eext_Rotary_Event_Info *info) -> Eina_Bool {
				if (info->direction == EEXT_ROTARY_DIRECTION_CLOCKWISE) {
					((TWindow *)data)->onWheel(1);
				}
				else if (info->direction == EEXT_ROTARY_DIRECTION_COUNTER_CLOCKWISE) {
					((TWindow *)data)->onWheel(-1);
				}
				dbg("dir=%d", info->direction);

				return ECORE_CALLBACK_PASS_ON;
			}, this);

	eext_object_event_callback_add(eo, EEXT_CALLBACK_BACK,
			[](void *data, Evas_Object *obj, void *event_info) {
				((TWindow *)data)->onBack();
			}, this);
#else
	evas_object_event_callback_add(eo_rect,
			EVAS_CALLBACK_MOUSE_WHEEL,
			[](void *data, Evas *evas, Evas_Object *obj,
					void *event_info) {
				Evas_Event_Mouse_Wheel *ev = (Evas_Event_Mouse_Wheel *) event_info;

				if (ev->direction == 0) {
					if (ev->z > 0) {
						/* Down */
						((TWindow *)data)->onWheel(1);
					}
					else if (ev->z < 0) {
						/* Up */
						((TWindow *)data)->onWheel(-1);
					}
				}
			}
			, this);
#endif

	etimer = ecore_timer_add(1.0, [](void *data) -> Eina_Bool {
		struct tm loc_time;
		time_t tt;

		time(&tt);
		localtime_r(&tt, &loc_time);

		((TWindow *)data)->onSeconds(tt, &loc_time);

		return EINA_TRUE;
	}, this);

	eo_gesture = elm_gesture_layer_add(eo);
	elm_gesture_layer_attach(eo_gesture, eo_rect);
	elm_gesture_layer_cb_set(eo_gesture, ELM_GESTURE_N_TAPS,
			ELM_GESTURE_STATE_END,
			[](void *data, void *event_info) -> Evas_Event_Flags {
				Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
				((TWindow *)data)->onTap(p->x, p->y, p->n, p->timestamp);
				return EVAS_EVENT_FLAG_ON_HOLD;
			}, this);

#ifdef HOST
	elm_gesture_layer_cb_set(eo_gesture, ELM_GESTURE_N_DOUBLE_TAPS,
			ELM_GESTURE_STATE_END,
			[](void *data, void *event_info) -> Evas_Event_Flags {
				dbg("doubletap -> back");
				((TWindow *)data)->onBack();
				return EVAS_EVENT_FLAG_ON_HOLD;
			}, this);
#endif

#ifdef FEATURE_TOUCH
	/* Flick */
	elm_gesture_layer_cb_set(eo_gesture, ELM_GESTURE_N_FLICKS,
			ELM_GESTURE_STATE_START,
			[](void *data, void *event_info) -> Evas_Event_Flags {
				return EVAS_EVENT_FLAG_ON_HOLD;
			}, this);
	elm_gesture_layer_cb_set(eo_gesture, ELM_GESTURE_N_FLICKS,
			ELM_GESTURE_STATE_ABORT,
			[](void *data, void *event_info) -> Evas_Event_Flags {
				return EVAS_EVENT_FLAG_ON_HOLD;
			}, this);
	elm_gesture_layer_cb_set(eo_gesture, ELM_GESTURE_N_FLICKS,
			ELM_GESTURE_STATE_END,
			[](void *data, void *event_info) -> Evas_Event_Flags {
				Elm_Gesture_Line_Info *p = (Elm_Gesture_Line_Info *) event_info;
				dbg("flick   : mx:%d, my:%d, angle:%f", p->momentum.mx, p->momentum.my, p->angle);

				return EVAS_EVENT_FLAG_ON_HOLD;
			}, this);

	/* Momentum */
	elm_gesture_layer_cb_set(eo_gesture, ELM_GESTURE_MOMENTUM,
			ELM_GESTURE_STATE_START,
			[](void *data, void *event_info) -> Evas_Event_Flags {
				Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) event_info;
				dbg("moment-s: mx:%d, my:%d  (tx=%d, ty=%d)", p->mx, p->my, p->tx, p->ty);

				return EVAS_EVENT_FLAG_ON_HOLD;
			}, this);
	elm_gesture_layer_cb_set(eo_gesture, ELM_GESTURE_MOMENTUM,
			ELM_GESTURE_STATE_ABORT,
			[](void *data, void *event_info) -> Evas_Event_Flags {
				Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) event_info;
				dbg("moment-a: mx:%d, my:%d", p->mx, p->my);
				return EVAS_EVENT_FLAG_ON_HOLD;
			}, this);
	elm_gesture_layer_cb_set(eo_gesture, ELM_GESTURE_MOMENTUM,
			ELM_GESTURE_STATE_MOVE,
			[](void *data, void *event_info) -> Evas_Event_Flags {
				Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) event_info;
				dbg("moment-m: mx:%d, my:%d, diff-y:%d", p->mx, p->my, p->y2-p->y1);
				if (p->my > 0) {
					((TWindow *)data)->onWheel(-1);
				}
				else if (p->my < 0) {
					((TWindow *)data)->onWheel(1);
				}
				return EVAS_EVENT_FLAG_ON_HOLD;
			}, this);
	elm_gesture_layer_cb_set(eo_gesture, ELM_GESTURE_MOMENTUM,
			ELM_GESTURE_STATE_END,
			[](void *data, void *event_info) -> Evas_Event_Flags {
				Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) event_info;
				dbg("moment-e: mx:%d, my:%d (tx=%d, ty=%d)", p->mx, p->my, p->tx, p->ty);

				return EVAS_EVENT_FLAG_ON_HOLD;
			}, this);
#endif

#ifndef HOST
	if (elm_win_wm_rotation_supported_get(eo)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(eo,
				(const int *) (&rots), 4);
	}
#endif

	evas_object_smart_callback_add(eo, "delete,request",
			[](void *data, Evas_Object *obj, void *event_info) {
				elm_exit();
			}, eo);

	parent_eo_ = eo;

	resizeTo(DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

EXPORT_API TWindow::~TWindow()
{
	if (etimer)
		ecore_timer_del(etimer);
	evas_object_del(eo_time_bg);
	evas_object_del(eo_time);
#ifdef HOST
	evas_object_del(eo_clip);
#endif
	evas_object_del(eo_rect);
	evas_object_del(eo_conform);
	evas_object_del(eo_bg);
	evas_object_del(eo);
}

EXPORT_API void TWindow::lower()
{
	elm_win_lower(eo);
}

EXPORT_API void TWindow::raise()
{
	elm_win_raise(eo);
}

EXPORT_API void TWindow::setBackgroundColor(int r, int g, int b, int a)
{
	if (a == 255) {
		elm_win_alpha_set(eo, EINA_FALSE);

	}
	else {
		elm_win_alpha_set(eo, EINA_TRUE);
	}

	evas_object_color_set(eo_bg, r, g, b, a);
}

EXPORT_API void TWindow::setTimeBgColor(int r, int g, int b, int a)
{
	cairo_surface_t *sf;
	cairo_t *cr;
	cairo_pattern_t *pat;

	sf = cairo_image_surface_create_for_data(
			(unsigned char *) evas_object_image_data_get(eo_time_bg,
			FALSE), CAIRO_FORMAT_ARGB32, DEFAULT_WIDTH, TIME_BG_HEIGHT,
			evas_object_image_stride_get(eo_time_bg));
	cr = cairo_create(sf);

	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	pat = cairo_pattern_create_linear(0.0, 0.0, 0.0, TIME_BG_HEIGHT);
	cairo_pattern_add_color_stop_rgba(pat, 0.0, (double) r / 255.0,
			(double) g / 255.0, (double) b / 255.0, (double) a / 255.0);
	cairo_pattern_add_color_stop_rgba(pat, 0.69, (double) r / 255.0,
			(double) g / 255.0, (double) b / 255.0, (double) a / 255.0 - 0.05);
	cairo_pattern_add_color_stop_rgba(pat, 0.7, (double) r / 255.0,
			(double) g / 255.0, (double) b / 255.0, (double) a / 255.0 - 0.2);
	cairo_pattern_add_color_stop_rgba(pat, 1.0, (double) r / 255.0,
			(double) g / 255.0, (double) b / 255.0, 0.0);
	cairo_set_source(cr, pat);
	cairo_paint(cr);
	cairo_pattern_destroy(pat);

	cairo_destroy(cr);

	evas_object_image_data_set(eo_time_bg,
			(unsigned char *) cairo_image_surface_get_data(sf));
	evas_object_image_data_update_add(eo_time_bg, 0, 0, rect_.w,
	TIME_BG_HEIGHT);
}

EXPORT_API void TWindow::setTimeColor(int r, int g, int b, int a)
{
	evas_object_color_set(eo_time, r, g, b, a);
}

EXPORT_API void TWindow::setTimeVisible(bool time_visible, bool time_bg_visible)
{
	if (is_time_show == time_visible
			&& is_time_bg_show == time_bg_visible) {
		return;
	}

	is_time_show = time_visible;
	is_time_bg_show = time_bg_visible;
	//dbg("time-show:%d, time-bg-show:%d", time_flag, time_bg_flag);

	if (time_visible) {
		prev_hour = 0;
		prev_min = 0;
		evas_object_show(eo_time);
	}
	else {
		evas_object_hide(eo_time);
	}

	if (time_bg_visible)
		evas_object_show(eo_time_bg);
	else
		evas_object_hide(eo_time_bg);
}

EXPORT_API void TWindow::setTitle(const char *title)
{
	elm_win_title_set(eo, title);
}

EXPORT_API Evas_Object* TWindow::getContainer()
{
	return eo_conform;
}

EXPORT_API Evas_Object* TWindow::getEvasObject()
{
	return eo;
}

EXPORT_API void TWindow::onShow(bool UNUSED(smooth))
{
	evas_object_show(eo);
}

EXPORT_API void TWindow::onHide(bool UNUSED(smooth))
{
	evas_object_hide(eo);
}

EXPORT_API void TWindow::onMove(int x, int y)
{
	evas_object_move(eo, x, y);
}

EXPORT_API void TWindow::onResize(int w, int h)
{
	evas_object_resize(eo_time_bg, w, TIME_BG_HEIGHT);
#ifdef HOST
	evas_object_resize(eo_clip, w, h);
#endif
	evas_object_resize(eo_rect, w, h);
	evas_object_resize(eo, w, h);
}

EXPORT_API void TWindow::onWheel(int dir)
{
}

EXPORT_API void TWindow::onBack()
{
}

EXPORT_API void TWindow::onTap(int x, int y, unsigned int n, unsigned int timestamp)
{
}

EXPORT_API void TWindow::onSeconds(time_t tt, struct tm *loc_time)
{
	char buf[255];
	int cx;
	int h, m;
	const char *ampm;

	m = loc_time->tm_min;
	h = loc_time->tm_hour;
	if (h > 12) {
		h -= 12;
		ampm = "PM";
	}
	else {
		ampm = "AM";
	}

	if (h != prev_hour || m != prev_min) {
		prev_hour = h;
		prev_min = m;

		snprintf(buf, 255, "%d:%02d %s", h, m, ampm);
		evas_object_text_text_set(eo_time, buf);
		evas_object_geometry_get(eo_time, NULL, NULL, &cx, NULL);
		cx = rect_.w / 2 - cx / 2;
		evas_object_move(eo_time, cx, 18);
		evas_object_show(eo_time);
	}
}
