#include <typeinfo>
#include "TLibrary.h"

#define MAX_ID 99999

static int g_count = 0;
static int g_id = 0;

extern "C"
{

EXPORT_API void tuner_debug_object_count (const char *msg)
{
	info("TObject count: %04d   (%s)", g_count, msg);
}

}

EXPORT_API TObject::TObject (Evas_Object *parent_eo, const char *object_name) :
		debug_enable(false), parent_eo_(parent_eo), visibility_(false), layer_(
				0), opacity_(255)
{
	rect_.x = 0;
	rect_.y = 0;
	rect_.w = 0;
	rect_.h = 0;

	callback_func_ = NULL;
	callback_func_user_data_ = NULL;

	name_ = object_name;
	id_ = g_id;
	g_id++;
	if (g_id > MAX_ID)
		g_id = 0;

	g_count++;

	if (debug_enable) {
		info("created:<%05d-%s> count=%d (eo=%p)", id_, name_, g_count,
				parent_eo_);
	}
}

EXPORT_API TObject::TObject (const char *object_name) :
		debug_enable(false), parent_eo_(NULL), visibility_(false), layer_(0), opacity_(
				255)
{
	rect_.x = 0;
	rect_.y = 0;
	rect_.w = 0;
	rect_.h = 0;

	callback_func_ = NULL;
	callback_func_user_data_ = NULL;

	name_ = object_name;
	id_ = g_id;
	g_id++;
	if (g_id > MAX_ID)
		g_id = 0;

	g_count++;

	if (debug_enable) {
		info("created:<%05d-%s> count=%d (eo=%p)", id_, name_, g_count,
				parent_eo_);
	}
}

EXPORT_API TObject::~TObject ()
{
	g_count--;

	//info("removed:<%05d-%s> remain_count=%d", id, name, g_count);
}

EXPORT_API TObject& TObject::debug (bool enable)
{
	debug_enable = enable;

	return *this;
}

EXPORT_API int TObject::getId ()
{
	return id_;
}

EXPORT_API const char* TObject::getName ()
{
	return name_;
}

EXPORT_API Evas_Object* TObject::getParentEvasObject (void)
{
	return parent_eo_;
}

TObject& TObject::rectWith (TObject *target)
{
	if (target->x() == rect_.x && target->y() == rect_.y
			&& target->width() == rect_.w && target->height() == rect_.h)
		return *this;

	rect_.x = target->x();
	rect_.y = target->y();
	rect_.w = target->width();
	rect_.h = target->height();

	onMove(rect_.x, rect_.y);
	onResize(rect_.w, rect_.h);

	return *this;
}

EXPORT_API TObject& TObject::moveTo (int x, int y)
{
	if (rect_.x == x && rect_.y == y)
		return *this;

	rect_.x = x;
	rect_.y = y;

	onMove(rect_.x, rect_.y);

	return *this;
}

EXPORT_API TObject& TObject::moveXTo (int x)
{
	if (rect_.x == x)
		return *this;

	rect_.x = x;
	onMove(rect_.x, rect_.y);

	return *this;
}

EXPORT_API TObject& TObject::moveYTo (int y)
{
	if (rect_.y == y)
		return *this;

	rect_.y = y;
	onMove(rect_.x, rect_.y);

	return *this;
}

EXPORT_API TObject& TObject::moveBy (int x, int y)
{
	if (x == 0 && y == 0)
		return *this;

	rect_.x += x;
	rect_.y += y;

	onMove(rect_.x, rect_.y);

	return *this;
}

EXPORT_API TObject& TObject::moveWith (Evas_Object *eo)
{
	Evas_Coord x = 0, y = 0;
	evas_object_geometry_get(eo, &x, &y, NULL, NULL);

	if (rect_.x == x && rect_.y == y)
		return *this;

	rect_.x = x;
	rect_.y = y;

	onMove(rect_.x, rect_.y);

	return *this;
}

EXPORT_API TObject& TObject::resizeTo (int w, int h)
{
	if (rect_.w == w && rect_.h == h)
		return *this;

	rect_.w = w;
	rect_.h = h;

	onResize(rect_.w, rect_.h);

	return *this;
}

EXPORT_API TObject& TObject::resizeBy (int w, int h)
{
	if (w == 0 && h == 0)
		return *this;

	rect_.w += w;
	rect_.h += h;

	onResize(rect_.w, rect_.h);

	return *this;
}

EXPORT_API TObject& TObject::resizeWith (Evas_Object *eo)
{
	Evas_Coord w = 0, h = 0;
	evas_object_geometry_get(eo, NULL, NULL, &w, &h);

	if (rect_.w == w && rect_.h == h)
		return *this;

	rect_.w = w;
	rect_.h = h;

	onResize(rect_.w, rect_.h);

	return *this;
}

EXPORT_API TObject& TObject::resizeWith (TObject *target)
{
	rect_.w = target->width();
	rect_.h = target->height();

	onResize(rect_.w, rect_.h);
	return *this;
}

EXPORT_API bool TObject::isVisible () const
{
	return visibility_;
}

EXPORT_API void TObject::show (bool smooth)
{
	visibility_ = true;

	onShow(smooth);
}

EXPORT_API void TObject::hide (bool smooth)
{
	visibility_ = false;

	onHide(smooth);
}

EXPORT_API TObject& TObject::update ()
{
	onUpdate();

	return *this;
}

EXPORT_API TObject& TObject::moveToAngle (double angle, int r, int cx, int cy,
		bool center)
{
	double rad;
	double x, y;

	rad = (angle - 90.0) * (M_PI / 180);

	x = cx + r * cos(rad);
	y = cy + r * sin(rad);

	if (center) {
		x -= rect_.w / 2;
		y -= rect_.h / 2;
	}

	rect_.x = x;
	rect_.y = y;

	onMove(rect_.x, rect_.y);

	return *this;
}

EXPORT_API TObject& TObject::moveToAngle (double angle, int r, TObject *base,
		bool center)
{
	return moveToAngle(angle, r, base->rect_.x + base->rect_.w / 2,
			base->rect_.y + base->rect_.h / 2, center);
}

EXPORT_API TObject& TObject::layerTo (int layer)
{
	layer_ = layer;

	onLayer(layer_);

	return *this;
}

EXPORT_API TObject& TObject::layerBy (int layer)
{
	layer_ += layer;

	onLayer(layer_);

	return *this;
}

EXPORT_API TObject& TObject::opacityTo (int alpha)
{
	opacity_ = alpha;

	onOpacity(opacity_);

	return *this;
}

EXPORT_API void TObject::onMove (int UNUSED(x), int UNUSED(y))
{
}

EXPORT_API void TObject::onResize (int UNUSED(w), int UNUSED(h))
{
}

EXPORT_API void TObject::onLayer (int UNUSED(layer))
{
}

EXPORT_API void TObject::onOpacity (int UNUSED(alpha))
{
}

EXPORT_API void TObject::onHide (bool UNUSED(smooth))
{
}

EXPORT_API void TObject::onShow (bool UNUSED(smooth))
{
}

EXPORT_API void TObject::onUpdate ()
{
}

EXPORT_API TObject& TObject::setCallback (TObjectCallback func, void *user_data)
{
	callback_func_ = func;
	callback_func_user_data_ = user_data;

	return *this;
}

EXPORT_API TObject& TObject::emit (const char *msg, const void *data)
{
	if (!callback_func_)
		return *this;

	callback_func_(this, msg, data, callback_func_user_data_);

	return *this;
}
