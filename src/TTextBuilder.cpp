#include "TLibrary.h"
#include "TTextBuilder.h"

EXPORT_API TTextBuilder::TTextBuilder(Evas_Object *parent_win)
{
	Evas_Coord w, h;

	pwin = parent_win;
	win = elm_win_add(pwin, "inlined",
			ELM_WIN_INLINED_IMAGE);
	evas_object_move(win, 0, 0);
	evas_object_geometry_get(win, NULL, NULL, &w, &h);
	evas_object_show(win);

	tb = evas_object_textblock_add(evas_object_evas_get(win));
	w = DEFAULT_WIDTH;
	evas_object_resize(tb, w, h);
	evas_object_show(tb);

	srcs = g_list_append(NULL, tb);

	font_style.color = g_strdup("color=#000");
	font_style.bgcolor = g_strdup("");
	font_style.align = g_strdup("");
	font_style.valign = 0.0;
	font_style.weight = g_strdup("");
	font_style.name = g_strdup_printf("font=%s", FONT_NAME);
	//font_style.name = g_strdup("font=Sans");
	//font_style.name = g_strdup("font=BreezeSans");
	//font_style.name = g_strdup("font=Tizen:style=Regular");
	font_style.size = 30;

	saved_text = NULL;
	total_height = 0;
	total_width = 0;
}

EXPORT_API TTextBuilder::~TTextBuilder()
{
	if (srcs) {
		g_list_foreach(srcs, [](void *data, void *user_data) {
			evas_object_del((Evas_Object *)data);
		}, NULL);
	}

	g_free(saved_text);
	g_free((void *) font_style.color);
	g_free((void *) font_style.bgcolor);
	g_free((void *) font_style.align);
	g_free((void *) font_style.name);

	if (win)
		evas_object_del(win);
}

EXPORT_API TTextBuilder& TTextBuilder::setSize(int size)
{
	font_style.size = size;

	return *this;
}

EXPORT_API TTextBuilder& TTextBuilder::setColor(unsigned int rgba)
{
	g_free((void *) font_style.color);

	font_style.color = g_strdup_printf("color=#%X", rgba & 0xFFFFFFFF);

	return *this;
}

EXPORT_API TTextBuilder& TTextBuilder::setColor(const char *color)
{
	if (!color)
		return *this;

	g_free((void *) font_style.color);

	font_style.color = g_strdup_printf("color=%s", color);

	return *this;
}

EXPORT_API TTextBuilder& TTextBuilder::setBgColor(unsigned int rgba)
{
	g_free((void *) font_style.bgcolor);

	font_style.bgcolor = g_strdup_printf("backing=on backing_color=#%X",
			rgba & 0xFFFFFFFF);

	return *this;
}

EXPORT_API TTextBuilder& TTextBuilder::setBgColor(const char *color)
{
	if (!color)
		return *this;

	g_free((void *) font_style.bgcolor);

	font_style.bgcolor = g_strdup_printf("backing=on backing_color=%s", color);

	return *this;
}

EXPORT_API TTextBuilder& TTextBuilder::setAlign(enum TTextBuilderAlign align)
{
	const char *str;

	switch (align) {
	case TTEXT_BUILDER_ALIGN_LEFT:
		str = "left";
		break;
	case TTEXT_BUILDER_ALIGN_CENTER:
		str = "center";
		break;
	case TTEXT_BUILDER_ALIGN_RIGHT:
		str = "right";
		break;
	default:
		return *this;
	}

	g_free((void *) font_style.align);

	font_style.align = g_strdup_printf("align=%s", str);

	return *this;
}

EXPORT_API TTextBuilder& TTextBuilder::setVAlign(enum TTextBuilderVAlign valign)
{
	switch (valign) {
	case TTEXT_BUILDER_VALIGN_TOP:
		font_style.valign = 0.0;
		break;
	case TTEXT_BUILDER_VALIGN_CENTER:
		font_style.valign = 0.5;
		break;
	case TTEXT_BUILDER_VALIGN_BOTTOM:
		font_style.valign = 1.0;
		break;
	case TTEXT_BUILDER_VALIGN_BASELINE:
		font_style.valign = -1.0;
		break;
	default:
		return *this;
	}

	return *this;
}

EXPORT_API TTextBuilder& TTextBuilder::setText(const char *str)
{
	g_free((void *) saved_text);

	saved_text = g_strdup(str);

	return *this;
}

EXPORT_API TTextBuilder& TTextBuilder::setFont(const char *str)
{
	g_free((void *) font_style.name);

	font_style.name = g_strdup_printf("font=%s", str);

	return *this;
}

EXPORT_API TTextBuilder& TTextBuilder::setWeight(enum TTextBuilderWeight weight)
{
	const char *str;

	switch (weight) {
	case TTEXT_BUILDER_WEIGHT_NORMAL:
		str = "normal";
		break;
	case TTEXT_BUILDER_WEIGHT_THIN:
		str = "thin";
		break;
	case TTEXT_BUILDER_WEIGHT_ULTRALIGHT:
		str = "ultralight";
		break;
	case TTEXT_BUILDER_WEIGHT_LIGHT:
		str = "light";
		break;
	case TTEXT_BUILDER_WEIGHT_BOOK:
		str = "book";
		break;
	case TTEXT_BUILDER_WEIGHT_MEDIUM:
		str = "medium";
		break;
	case TTEXT_BUILDER_WEIGHT_SEMIBOLD:
		str = "semibold";
		break;
	case TTEXT_BUILDER_WEIGHT_BOLD:
		str = "bold";
		break;
	case TTEXT_BUILDER_WEIGHT_ULTRABOLD:
		str = "ultrabold";
		break;
	case TTEXT_BUILDER_WEIGHT_BLACK:
		str = "black";
		break;
	case TTEXT_BUILDER_WEIGHT_EXTRABLACK:
		str = "extrablack";
		break;
	default:
		return *this;
	}

	g_free((void *) font_style.weight);

	font_style.weight = g_strdup_printf("font_weight=%s", str);

	return *this;
}

EXPORT_API TTextBuilder& TTextBuilder::done(int width, int height)
{
	char *buf;
	Evas_Coord x, y, w, h;
	Evas_Textblock_Style *st;

	st = evas_textblock_style_new();
	buf = g_strdup_printf("DEFAULT='wrap=char %s %s %s %s %s font_size=%d'",
			font_style.name, font_style.weight, font_style.color,
			font_style.bgcolor,
			font_style.align, font_style.size);
	evas_textblock_style_set(st, buf);
	g_free(buf);
	evas_object_textblock_style_set(tb, st);
	evas_textblock_style_free(st);

	evas_object_textblock_valign_set(tb, font_style.valign);
	evas_object_textblock_text_markup_set(tb, saved_text);
	evas_render(evas_object_evas_get(win));

	evas_object_textblock_size_formatted_get(tb, &w, &h);
	if (width != -1)
		w = width;
	if (height != -1)
		h = height;

	if (w == 0)
		w = 100;

	evas_object_resize(tb, w, h);
	evas_object_geometry_get(tb, &x, &y, NULL, NULL);

	if (x + w > total_width)
		total_width = x + w;
	if (y + h > total_height)
		total_height = y + h;

	evas_object_resize(win, total_width, total_height);

	srcs = g_list_append(srcs, tb);
	tb = evas_object_textblock_add(evas_object_evas_get(win));
	evas_object_move(tb, 0, total_height);
	evas_object_resize(tb, total_width, 1);
	evas_object_show(tb);

	return *this;
}

EXPORT_API cairo_surface_t* TTextBuilder::makeSurface(unsigned int rgba_bg,
		int crop_x, int crop_y, int crop_width, int crop_height)
{
	Evas_Object *bimg;
	cairo_surface_t *tmp_sf;
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

	bimg = elm_win_inlined_image_object_get(win);
	evas_object_move(bimg, 0, 0);
	evas_object_resize(bimg, total_width, total_height);
	evas_object_show(bimg);

	evas_render(evas_object_evas_get(win));

	tmp_sf = cairo_image_surface_create_for_data(
			(unsigned char *) evas_object_image_data_get(bimg, FALSE),
			CAIRO_FORMAT_ARGB32, total_width, total_height,
			evas_object_image_stride_get(bimg));

	cairo_set_source_surface(new_cr, tmp_sf, crop_x, crop_y);
	cairo_paint(new_cr);
	cairo_surface_destroy(tmp_sf);
	cairo_destroy(new_cr);

	return new_sf;
}

EXPORT_API Evas_Object* TTextBuilder::makeImage(
		unsigned int rgba_bg, int crop_x, int crop_y, int crop_width,
		int crop_height)
{
	Evas_Object *src;
	unsigned char *pixels;
	cairo_surface_t *new_sf;

	if (crop_width == -1)
		crop_width = total_width;
	if (crop_height == -1)
		crop_height = total_height;

	src = evas_object_image_filled_add(evas_object_evas_get(pwin));
	evas_object_image_size_set(src, crop_width, crop_height);
	evas_object_image_colorspace_set(src, EVAS_COLORSPACE_ARGB8888);
	evas_object_resize(src, crop_width, crop_height);
	evas_object_repeat_events_set(src, EINA_FALSE);
	evas_object_image_alpha_set(src, EINA_TRUE);
	evas_object_name_set(src, "TText");

	new_sf = makeSurface(rgba_bg, crop_x, crop_y, crop_width,
			crop_height);
	pixels = cairo_image_surface_get_data(new_sf);
	evas_object_image_data_copy_set(src, pixels);
	evas_object_image_data_update_add(src, 0, 0, crop_width, crop_height);

	cairo_surface_destroy(new_sf);

	return src;
}
