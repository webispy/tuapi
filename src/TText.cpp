#include <ft2build.h>
#include FT_FREETYPE_H

#include "TLibrary.h"

#include "TText.h"

static FT_Library ft_library = NULL;

EXPORT_API TText::TText () :
		color(0), useTrim(false), useDropShadow(false), drop_color(0), drop_off_x(
				0), drop_off_y(0)
{
	if (!ft_library) {
		FT_Error ft_error = FT_Init_FreeType(&ft_library);
		if (ft_error) {
			err("FT_Init_FreeType() failed: %d", ft_error);
			return;
		}
	}

	sf = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, NULL);
	cr = cairo_create(sf);

	FT_New_Face(ft_library, FONT_PATH, 0, &ft_face);
	font_face = cairo_ft_font_face_create_for_ft_face(ft_face, 0);

	font_opt = cairo_font_options_create();
	cairo_font_options_set_antialias(font_opt, CAIRO_ANTIALIAS_GRAY);
	cairo_font_options_set_hint_style(font_opt, CAIRO_HINT_STYLE_FULL);
	cairo_set_font_options(cr, font_opt);

	cairo_set_font_face(cr, font_face);

	total_height = 0;
	total_width = 0;
}

EXPORT_API TText::~TText ()
{
	if (font_opt)
		cairo_font_options_destroy(font_opt);

	if (ft_face)
		FT_Done_Face(ft_face);

	if (cr)
		cairo_destroy(cr);
	if (sf)
		cairo_surface_destroy(sf);
}

EXPORT_API TText& TText::setFont (const char *description)
{
#ifdef USE_PANGO

	if (desc)
		pango_font_description_free(desc);

	desc = pango_font_description_from_string(description);
	if (desc) {
		pango_layout_set_font_description(layout, desc);
	}
	else {
		err("invalid description('%s')", description)
	}

#endif

	return *this;
}

EXPORT_API TText& TText::setFont (const char *family, int size,
		PangoWeight weight, PangoStyle style)
{
	cairo_set_font_size(cr, size);
#ifdef USE_PANGO
	if (desc)
		pango_font_description_free(desc);

	desc = pango_font_description_new();
	pango_font_description_set_family(desc, family);
	pango_font_description_set_weight(desc, weight);
	pango_font_description_set_style(desc, style);
	pango_font_description_set_size(desc, size * PANGO_SCALE);

	pango_layout_set_font_description(layout, desc);
#endif
	return *this;
}

EXPORT_API TText& TText::setAlignment (PangoAlignment align)
{
#ifdef USE_PANGO
	pango_layout_set_alignment(layout, align);
#endif

	return *this;
}

EXPORT_API TText& TText::setDropShadow (unsigned int rgba, int off_x, int off_y)
{
	useDropShadow = true;

	drop_color = rgba;
	drop_off_x = off_x;
	drop_off_y = off_y;

	return *this;
}

EXPORT_API TText& TText::addMarkupLine (const char *str, int wrap_width,
		bool fill_width)
{
#ifdef USE_PANGO

	PangoRectangle ink_rect;
	PangoRectangle logical_rect;
	int w, h;
	int y;

	if (!str)
		return *this;

	y = total_height;

	if (wrap_width > 0) {
		pango_layout_set_width(layout, wrap_width * PANGO_SCALE);
		pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
	}

	pango_layout_set_markup(layout, str, -1);

	pango_layout_get_pixel_extents(layout, &ink_rect, &logical_rect);

	if (logical_rect.height > ink_rect.height)
		h = logical_rect.height;
	else
		h = ink_rect.height;

	if (logical_rect.width > ink_rect.width)
		w = logical_rect.width;
	else
		w = ink_rect.width;

#if 0
	int baseline;

	baseline = pango_layout_get_baseline(layout) / PANGO_SCALE;
	dbg("text: %s", str);

	dbg("ink : (%d,%d)~(%d,%d)", ink_rect.x, ink_rect.y,
			ink_rect.width, ink_rect.height);

	dbg("log : (%d,%d)~(%d,%d)", logical_rect.x, logical_rect.y,
			logical_rect.width, logical_rect.height);

	dbg("----: baseline:%d, h=%d, total_height=%d", baseline, h, total_height);
#endif

	if (useTrim) {
		y = -ink_rect.y;
		h = ink_rect.height;
#if 0
		dbg("trim: h=%d", h);
#endif
	}

	if (w > total_width)
		total_width = w;

	if (fill_width) {
		if (wrap_width > 0 && total_width < wrap_width)
			total_width = wrap_width;
	}

	total_height = total_height + h;

	if (useDropShadow) {
		cairo_set_source_rgba(cr, ((drop_color & 0xFF000000) >> 24) / 255.0,
				((drop_color & 0x00FF0000) >> 16) / 255.0,
				((drop_color & 0x0000FF00) >> 8) / 255.0,
				(drop_color & 0x000000FF) / 255.0);

		cairo_move_to(cr, drop_off_x, y + drop_off_y);
		pango_cairo_show_layout(cr, layout);

		setColor(color);
	}

	cairo_move_to(cr, 0, y);
	pango_cairo_show_layout(cr, layout);

	g_object_unref(layout);
	g_object_unref(context);

	context = pango_cairo_create_context(cr);
	pango_cairo_context_set_resolution(context, 72.0);

	layout = pango_layout_new(context);
	pango_layout_set_font_description(layout, desc);
#endif

	return *this;
}

EXPORT_API TText& TText::addLine (const char *str, int wrap_width,
		bool fill_width)
{
	gchar *escaped;

	if (!str)
		return *this;

	escaped = g_markup_escape_text(str, -1);
#ifdef USE_PANGO
	pango_layout_set_markup(layout, escaped, -1);
#endif

	addMarkupLine(escaped, wrap_width, fill_width);

	g_free(escaped);

	return *this;
}

EXPORT_API TText& TText::setColor (int r, int g, int b, int a)
{
	color = r << 24 | g << 16 | b << 8 | a;

	cairo_set_source_rgba(cr, (double) r / 255.0, (double) g / 255.0,
			(double) b / 255.0, (double) a / 255.0);

	return *this;
}

EXPORT_API TText& TText::setColor (unsigned int rgba)
{
	color = rgba;
	cairo_set_source_rgba(cr, ((rgba & 0xFF000000) >> 24) / 255.0,
			((rgba & 0x00FF0000) >> 16) / 255.0,
			((rgba & 0x0000FF00) >> 8) / 255.0, (rgba & 0x000000FF) / 255.0);

	return *this;
}

EXPORT_API cairo_surface_t* TText::makeSurface (unsigned int rgba_bg,
		int crop_x, int crop_y, int crop_width, int crop_height)
{
	cairo_surface_t *new_sf;
	cairo_t *new_cr;

	if (crop_width == -1)
		crop_width = total_width;
	if (crop_height == -1)
		crop_height = total_height;

//	dbg("w=%d, h=%d, x=%d, y=%d, rgba_bg=%x", crop_width, crop_height, crop_x,
//			crop_y, rgba_bg);

	new_sf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, crop_width,
			crop_height);

	new_cr = cairo_create(new_sf);
	cairo_set_source_rgba(new_cr, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator(new_cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(new_cr);

	cairo_set_operator(new_cr, CAIRO_OPERATOR_OVER);

	if (rgba_bg) {
		cairo_set_source_rgba(new_cr, ((rgba_bg & 0xFF000000) >> 24) / 255.0,
				((rgba_bg & 0x00FF0000) >> 16) / 255.0,
				((rgba_bg & 0x0000FF00) >> 8) / 255.0,
				(rgba_bg & 0x000000FF) / 255.0);
		cairo_paint(new_cr);
	}

	cairo_set_source_surface(new_cr, sf, crop_x, crop_y);
	cairo_paint(new_cr);
	cairo_destroy(new_cr);

	return new_sf;
}

EXPORT_API Evas_Object* TText::makeImage (Evas_Object *parent,
		unsigned int rgba_bg, int crop_x, int crop_y, int crop_width,
		int crop_height)
{
	Evas_Object *src;
	unsigned char *pixels;
	cairo_surface_t *new_sf = makeSurface(rgba_bg, crop_x, crop_y, crop_width,
			crop_height);

	if (crop_width == -1)
		crop_width = total_width;
	if (crop_height == -1)
		crop_height = total_height;

	pixels = cairo_image_surface_get_data(new_sf);

	src = evas_object_image_filled_add(evas_object_evas_get(parent));
	evas_object_image_size_set(src, crop_width, crop_height);
	evas_object_image_colorspace_set(src, EVAS_COLORSPACE_ARGB8888);
	evas_object_resize(src, crop_width, crop_height);
	evas_object_repeat_events_set(src, EINA_FALSE);
	evas_object_image_alpha_set(src, EINA_TRUE);
	evas_object_name_set(src, "TText");

	evas_object_image_data_copy_set(src, pixels);
	evas_object_image_data_update_add(src, 0, 0, crop_width, crop_height);

	cairo_surface_destroy(new_sf);

	return src;
}
