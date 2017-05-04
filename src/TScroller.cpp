#include "TLibrary.h"

#include "TScroller.h"

EXPORT_API TScroller::TScroller (TObject *target) :
		target(target), target_max_height(0), bg(0), bar(0), bar_size(0)
{
	cairo_surface_t *sf;
	cairo_t *cr;
	int w = target->width(), h = target->height();
	void *pixels;

	bg = new TImage(target->getParentEvasObject(), w, h);
	bg->hide();

	bar = new TImage(target->getParentEvasObject(), w, h);
	bar->hide();

	sf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cr = cairo_create(sf);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_paint(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_line_width(cr, 6);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.2);
	cairo_arc(cr, w / 2, h / 2, w / 2 - 3,
			tuner_util_convert_angle_to_radian(45),
			tuner_util_convert_angle_to_radian(135));
	cairo_stroke(cr);

	cairo_destroy(cr);
	pixels = cairo_image_surface_get_data(sf);
	evas_object_image_data_copy_set(bg->getFrameSource(), pixels);
	evas_object_image_data_update_add(bg->getFrameSource(), 0, 0, w, h);

	evas_object_name_set(bg->getFrameSource(), "TScroller::bg");

	cairo_surface_destroy(sf);
}

EXPORT_API TScroller::~TScroller ()
{
	if (bg)
		delete bg;

	if (bar)
		delete bar;
}

EXPORT_API void TScroller::update (int max_height, int last_item_size)
{
	cairo_surface_t *sf;
	cairo_t *cr;
	void *pixels;
	int w, h;
	int tmp;

	evas_object_image_size_get(bar->getFrameSource(), &w, &h);

	if (last_item_size)
		tmp = max_height + (h - last_item_size);
	else
		tmp = max_height;

	if (tmp == target_max_height)
		return;

	target_max_height = tmp;

	if (target_max_height <= h)
		bar_size = 0.0;
	else
		bar_size = 90.0 * h / target_max_height;

	//dbg("bar_size: %f  (h=%d, max=%d)", bar_size, h, max_height);

	sf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cr = cairo_create(sf);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_paint(cr);

	if (bar_size > 0) {
		cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
		cairo_set_line_width(cr, 6);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
		cairo_set_source_rgba(cr, 129.0 / 255.0, 129.0 / 255.0, 129.0 / 255.0,
				1.0);
		cairo_arc(cr, w / 2, h / 2, w / 2 - 3,
				tuner_util_convert_angle_to_radian(45.0),
				tuner_util_convert_angle_to_radian(45.0 + bar_size));
		cairo_stroke(cr);
	}

	cairo_destroy(cr);
	pixels = cairo_image_surface_get_data(sf);
	evas_object_image_data_copy_set(bar->getFrameSource(), pixels);
	evas_object_image_data_update_add(bar->getFrameSource(), 0, 0, w, h);

	evas_object_name_set(bar->getFrameSource(), "TScroller::bar");

	cairo_surface_destroy(sf);
}

EXPORT_API void TScroller::set (int position)
{
	double r;

	if (position < 0)
		r = 0;
	else
		r = 90.0 * position / target_max_height;

	if (r > 90.0 - bar_size)
		r = 90.0 - bar_size;

	//dbg("position:%d, max:%d, r=%f", position, target_max_height, r);

	bar->getTransform().rotate(r).apply();
}

EXPORT_API void TScroller::show ()
{
	bg->show();
	bar->show();
}

EXPORT_API void TScroller::hide ()
{
	bg->hide();
	bar->hide();
}

EXPORT_API void TScroller::setLayer (int layer)
{
	bg->layerTo(layer);
	bar->layerTo(layer + 1);
}
