#ifndef HOST
#include <app.h>
#endif

#include "TLibrary.h"

#define PI     3.14159265358979323846
#define PI_180 (PI / 180)

EXPORT_API double get_scale()
{
#ifdef HOST
	return elm_config_scale_get();
#else
	return 1.0;
#endif
}

EXPORT_API gchar *get_img_path(const char *file)
{
#ifndef HOST
	char *res_path;
	char *result;

	res_path = app_get_resource_path();
	if (!res_path)
		return NULL;

	result = g_strdup_printf("%s%s", res_path, file);
	free(res_path);

	dbg("load resource: %s", result);

	return result;
#else
	return g_strdup_printf("%s/%s", "../res", file);
#endif
}

EXPORT_API double tuner_util_convert_angle_to_radian (double angle)
{
	double value = angle;

	if (value >= 0 && value <= 90)
		value += 270.0;
	else
		value -= 90.0;

	if (value < 0)
		value += 360.0;

	return value * PI_180;
}


EXPORT_API Evas_Object *
tuner_util_make_circle_image (Evas_Object *parent, const gchar *path, int w,
		int h)
{
	Evas_Object *src;
	cairo_t *cr;
	cairo_surface_t *sf;
	cairo_surface_t *img_sf;
	cairo_surface_t *outline_sf;
	cairo_surface_t *shadow_sf;
	unsigned char *pixels;
	double width_ratio, height_ratio;
	gchar *img_path;

	sf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cr = cairo_create(sf);

	img_sf = cairo_image_surface_create_from_png(path);

	img_path = get_img_path("people_dashboard_main_stroke.png");
	outline_sf = cairo_image_surface_create_from_png(img_path);
	g_free(img_path);

	img_path = get_img_path("people_dashboard_main_shadow.png");
	shadow_sf = cairo_image_surface_create_from_png(img_path);
	g_free(img_path);

	/* clipped image */
	cairo_save(cr);
	cairo_arc(cr, w / 2, h / 2 - 1 * get_scale(),
			w / 2 - 3 * get_scale(), 0, 2 * M_PI);
	cairo_clip(cr);
	cairo_save(cr);
	width_ratio = (double) (w)
			/ (double) (cairo_image_surface_get_width(img_sf));
	height_ratio = (double) (h)
			/ (double) (cairo_image_surface_get_height(img_sf));
	cairo_scale(cr, width_ratio, height_ratio);
	cairo_set_source_surface(cr, img_sf, 0, 0);
	cairo_paint(cr);
	cairo_restore(cr);
	cairo_restore(cr);
	cairo_surface_finish(img_sf);
	cairo_surface_destroy(img_sf);

	/* add outline, drop-shadow */
	cairo_save(cr);
	width_ratio = (double) (w)
			/ (double) (cairo_image_surface_get_width(outline_sf));
	height_ratio = (double) (h)
			/ (double) (cairo_image_surface_get_height(outline_sf));
	cairo_scale(cr, width_ratio, height_ratio);
	cairo_set_source_surface(cr, outline_sf, 0, 0);
	cairo_paint(cr);
	cairo_set_source_surface(cr, shadow_sf, 0, 0);
	cairo_paint(cr);
	cairo_restore(cr);
	cairo_surface_finish(outline_sf);
	cairo_surface_destroy(outline_sf);
	cairo_surface_finish(shadow_sf);
	cairo_surface_destroy(shadow_sf);

	cairo_destroy(cr);

	pixels = cairo_image_surface_get_data(sf);

	src = evas_object_image_filled_add(evas_object_evas_get(parent));
	evas_object_image_size_set(src, w, h);
	evas_object_image_colorspace_set(src, EVAS_COLORSPACE_ARGB8888);
	evas_object_resize(src, w, h);
	evas_object_repeat_events_set(src, EINA_FALSE);
	evas_object_image_alpha_set(src, EINA_TRUE);
	evas_object_image_data_copy_set(src, pixels);
	evas_object_image_data_update_add(src, 0, 0, w, h);
	//evas_object_show(src);

	cairo_surface_finish(sf);
	cairo_surface_destroy(sf);

	return src;
}


EXPORT_API Evas_Object *
tuner_util_make_text_image (Evas_Object *parent, const gchar *icon_path,
		int space, const gchar *text, int size, unsigned int rgba,
		unsigned int rgba_bg, cairo_font_weight_t weight, int *result_w,
		int *result_h)
{
	Evas_Object *src;
	cairo_t *cr;
	cairo_surface_t *sf;
	cairo_surface_t *img_sf = NULL;
	unsigned char *pixels;
	cairo_text_extents_t extents;
	cairo_font_extents_t max_extents;
	int text_x = space, w = space, h = 0;
	double height_ratio = 1.0;
	int icon_y = 0;

	sf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 0, 0);
	cr = cairo_create(sf);
	cairo_select_font_face(cr, FONT_NAME, CAIRO_FONT_SLANT_NORMAL, weight);
	cairo_set_font_size(cr, size);
	cairo_font_extents(cr, &max_extents);
	cairo_text_extents(cr, text, &extents);
	cairo_destroy(cr);
	cairo_surface_finish(sf);
	cairo_surface_destroy(sf);

	h = ceil(max_extents.ascent + max_extents.descent);
	if (h < extents.height)
		h = extents.height;

#if 0
	dbg("f.a: %.1f, f.d: %.1f, f.h: %.1f, f.y_advance: %.1f " ":: e.h: %.1f, e.y_advance: %.1f e.y_bearing: %.1f => h:%d",
			max_extents.ascent, max_extents.descent, max_extents.height,
			max_extents.max_y_advance, extents.height, extents.y_advance,
			extents.y_bearing, h);
#endif

	w += extents.x_advance;

	if (icon_path) {
		double icon_w = 0, icon_h = 0;

		img_sf = cairo_image_surface_create_from_png(icon_path);
		icon_h = cairo_image_surface_get_height(img_sf);
		if (icon_h > h) {
			height_ratio = (double) (h) / (double) (icon_h);
			icon_y = 2;
		}
		else {
			icon_y = (int) ((double) ((h - icon_h) / 2) + 0.5);
		}

		icon_w = (double) cairo_image_surface_get_width(img_sf) * height_ratio;
		text_x += (icon_w);
		w += (icon_w);
	}

	h += ceil(max_extents.ascent + extents.y_bearing) + 1;

	sf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cr = cairo_create(sf);
	cairo_select_font_face(cr, FONT_NAME, CAIRO_FONT_SLANT_NORMAL, weight);

	cairo_set_font_size(cr, size);

	if (rgba_bg) {
		cairo_set_source_rgba(cr, ((rgba_bg & 0xFF000000) >> 24) / 255.0,
				((rgba_bg & 0x00FF0000) >> 16) / 255.0,
				((rgba_bg & 0x0000FF00) >> 8) / 255.0,
				(rgba_bg & 0x000000FF) / 255.0);
		cairo_paint(cr);
	}

	if (icon_path) {
		cairo_save(cr);
		cairo_translate(cr, 0, icon_y);
		cairo_scale(cr, height_ratio, height_ratio);
		cairo_set_source_surface(cr, img_sf, 0, 0);
		cairo_paint(cr);
		cairo_restore(cr);
		cairo_surface_finish(img_sf);
		cairo_surface_destroy(img_sf);
	}

	cairo_set_source_rgba(cr, 0, 0, 0, 255.0 * 0.75);
	cairo_move_to(cr, text_x, max_extents.ascent + 1);
	cairo_show_text(cr, text);

	cairo_set_source_rgba(cr, ((rgba & 0xFF000000) >> 24) / 255.0,
			((rgba & 0x00FF0000) >> 16) / 255.0,
			((rgba & 0x0000FF00) >> 8) / 255.0, (rgba & 0x000000FF) / 255.0);
	cairo_move_to(cr, text_x, max_extents.ascent);
	cairo_show_text(cr, text);

	cairo_destroy(cr);

	pixels = cairo_image_surface_get_data(sf);

	src = evas_object_image_filled_add(evas_object_evas_get(parent));
	evas_object_image_size_set(src, w, h);
	evas_object_image_colorspace_set(src, EVAS_COLORSPACE_ARGB8888);
	evas_object_resize(src, w, h);
	evas_object_repeat_events_set(src, EINA_FALSE);
	evas_object_image_alpha_set(src, EINA_TRUE);

	evas_object_image_data_copy_set(src, pixels);
	evas_object_image_data_update_add(src, 0, 0, w, h);

	cairo_surface_finish(sf);
	cairo_surface_destroy(sf);

	if (result_w)
		*result_w = w;
	if (result_h)
		*result_h = h;

	return src;
}


static Evas_Object *make_round (Evas_Object *parent, int w, int h,
		Evas_Object *text, unsigned int rgba_bg, gboolean use_bg_pattern)
{
	Evas_Object *src;
	cairo_surface_t *sf;
	cairo_t *cr;
	unsigned char *pixels;
	double radius = 15;
	double a = 0, b = w, c = 0, d = h;
	int text_w, text_h;

	evas_object_image_size_get(text, &text_w, &text_h);

	src = evas_object_image_filled_add(evas_object_evas_get(parent));
	evas_object_image_alpha_set(src, EINA_TRUE);
	evas_object_image_size_set(src, w, h);
	evas_object_resize(src, w, h);
	evas_object_name_set(src, "TBubbleBG");
	//evas_object_image_content_hint_set(src, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);

	sf = cairo_image_surface_create_for_data(
			(unsigned char *) evas_object_image_data_get(src, FALSE),
			CAIRO_FORMAT_ARGB32, w, h, evas_object_image_stride_get(src));
	cr = cairo_create(sf);

	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

#if 1
	if (use_bg_pattern) {
		cairo_pattern_t *pat;
		pat = cairo_pattern_create_linear(0.0, 0.0, 0.0, h);
		cairo_pattern_add_color_stop_rgba(pat, 0.0, 1.0, 1.0, 1.0, 0.9);
		cairo_pattern_add_color_stop_rgba(pat, 0.69, 1.0, 1.0, 1.0, 0.8);
		cairo_pattern_add_color_stop_rgba(pat, 0.7, 1.0, 1.0, 1.0, 0.7);
		cairo_pattern_add_color_stop_rgba(pat, 1.0, 1.0, 1.0, 1.0, 0.0);
		cairo_set_source(cr, pat);
		cairo_paint(cr);
		cairo_pattern_destroy(pat);
	}
#endif

	/* round rect */
	double margin_w = 15, margin_h = 3;
	a = w / 2 - text_w / 2 - margin_w;
	b = a + text_w + margin_w * 2;
	c = h / 2 - text_h / 2 - margin_h;
	d = c + text_h + margin_h * 2;

	cairo_arc(cr, a + radius, c + radius, radius, 2 * (M_PI / 2),
			3 * (M_PI / 2));
	cairo_arc(cr, b - radius, c + radius, radius, 3 * (M_PI / 2),
			4 * (M_PI / 2));
	cairo_arc(cr, b - radius, d - radius, radius, 0 * (M_PI / 2),
			1 * (M_PI / 2));
	cairo_arc(cr, a + radius, d - radius, radius, 1 * (M_PI / 2),
			2 * (M_PI / 2));
	cairo_close_path(cr);

	cairo_set_source_rgba(cr, ((rgba_bg & 0xFF000000) >> 24) / 255.0,
			((rgba_bg & 0x00FF0000) >> 16) / 255.0,
			((rgba_bg & 0x0000FF00) >> 8) / 255.0,
			(rgba_bg & 0x000000FF) / 255.0);
	cairo_fill(cr);

	{
		cairo_surface_t *txt_sf;

		txt_sf = cairo_image_surface_create_for_data(
				(unsigned char *) evas_object_image_data_get(text, FALSE),
				CAIRO_FORMAT_ARGB32, text_w, text_h,
				evas_object_image_stride_get(text));
		cairo_set_source_surface(cr, txt_sf, w / 2 - text_w / 2,
				h / 2 - text_h / 2);
		cairo_paint(cr);
		cairo_surface_destroy(txt_sf);
	}
	cairo_destroy(cr);

	pixels = cairo_image_surface_get_data(sf);
	evas_object_image_data_set(src, pixels);

	return src;
}

EXPORT_API Evas_Object *tuner_util_round_title (Evas_Object *parent,
		const char *title, unsigned int rgba, unsigned int rgba_bg,
		gboolean use_bg_pattern)
{
	Evas_Object *txt = NULL;
	TText *tmp = new TText();
	Evas_Object *target;

	tmp->setFont(FONT_NAME, 26, PANGO_WEIGHT_BOLD).setColor(rgba).setAlignment(
			PANGO_ALIGN_LEFT).addMarkupLine(title);
	txt = tmp->makeImage(parent);
	delete tmp;

	target = make_round(parent, DEFAULT_WIDTH, 120, txt, rgba_bg,
			use_bg_pattern);

	return target;
}
