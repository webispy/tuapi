#include "TLibrary.h"

#include "TTransition.h"

struct bezier_prop
{
	double x1, y1;
	double x2, y2;
};

static struct bezier_prop preset[] = { { 0, 0, 0, 0 }, // manual
		{ 0.25, 0.1, 0.25, 1 }, // ease
		{ 0, 0, 1, 1 }, // linear
		{ 0.42, 0, 1, 1 }, // ease-in
		{ 0, 0, 0.58, 1 }, // ease-out
		{ 0.42, 0, 0.58, 1 }, // ease-in-out
		};

static inline double bezier_x (double pos, struct bezier_info *info)
{
	return pos * (info->cx + pos * (info->bx + pos * info->ax));
}

static inline double bezier_y (double pos, struct bezier_info *info)
{
	return pos * (info->cy + pos * (info->by + pos * info->ay));
}

static inline double bezier_x_der (double pos, struct bezier_info *info)
{
	return info->cx + pos * (2 * info->bx + pos * 3 * info->ax);
}

static double find_x_for (double pos, struct bezier_info *info)
{
	double x = pos, z;
	int i = 0;

	while (i < 5) {
		z = bezier_x(x, info) - pos;
		if (fabs(z) < 1e-3)
			break;

		x = x - z / bezier_x_der(x, info);
		i++;
	}

	return x;
}

static inline double bezier_map (double pos, struct bezier_info *info)
{
	return bezier_y(find_x_for(pos, info), info);
}

/* ------------------------------------------------------------------------- */

EXPORT_API TBezier::TBezier (double x1, double y1, double x2, double y2) :
		x1(x1), y1(y1), x2(x2), y2(y2)
{
	bi.cx = 3.0 * x1;
	bi.bx = 3.0 * (x2 - x1) - bi.cx;
	bi.ax = 1.0 - bi.cx - bi.bx;
	bi.cy = 3.0 * y1;
	bi.by = 3.0 * (y2 - y1) - bi.cy;
	bi.ay = 1.0 - bi.cy - bi.by;
}

EXPORT_API TBezier::~TBezier ()
{

}

EXPORT_API double TBezier::getFrame (double pos)
{
	return bezier_map(pos, &bi);
}

EXPORT_API TTransition::TTransition (TObject *target) :
		target(target), anim(0), repeat(0), request_next(0)
{

}

EXPORT_API TTransition::~TTransition ()
{
	if (anim)
		ecore_animator_del(anim);
}

EXPORT_API TTransition& TTransition::append (double duration,
		Transition::timing_function t, TransitionFunction func, void *user_data,
		bool repeat)
{
	struct tr_item item;
	struct bezier_prop *p = &preset[t];

	item.duration = duration;
	item.func = func;
	item.repeat = repeat;
	item.user_data = user_data;
	item.timing = t;
	item.cnt = 0;

	if (t != Transition::MANUAL) {
		item.bi.cx = 3.0 * p->x1;
		item.bi.bx = 3.0 * (p->x2 - p->x1) - item.bi.cx;
		item.bi.ax = 1.0 - item.bi.cx - item.bi.bx;
		item.bi.cy = 3.0 * p->y1;
		item.bi.by = 3.0 * (p->y2 - p->y1) - item.bi.cy;
		item.bi.ay = 1.0 - item.bi.cy - item.bi.by;
	}

	this->items.push_back(item);

	return *this;
}

Eina_Bool TTransition::on_animation (void *user_data, double pos)
{
	TTransition *tr = (TTransition *) user_data;
	double frame = 0.0;
	Eina_Bool ret;

	if (tr->cursor->timing != Transition::MANUAL)
		frame = bezier_map(pos, &(tr->cursor->bi));
	else
		frame = pos;

	tr->cursor->cnt++;
	if (tr->cursor->func)
		ret = tr->cursor->func(tr, tr->target, pos, frame,
				tr->cursor->user_data);
	else
		ret = EINA_TRUE;

	if (pos == 1.0 || ret == EINA_FALSE) {
//		dbg("frame: %d, animation-index: %ld", tr->cursor->cnt,
//				tr->cursor - tr->items.begin());
		if (!tr->cursor->repeat || tr->request_next) {
			if (tr->request_next) {
				tr->request_next = false;
//				dbg("next request (%ld)", tr->cursor - tr->items.begin());
			}

			tr->cursor++;
//			dbg("next index (%ld)", tr->cursor - tr->items.begin());
			if (tr->cursor == tr->items.end()) {
				//dbg("reach end");
				if (tr->repeat)
					tr->cursor = tr->items.begin();
				else {
					//dbg("reach end: anim=%x", tr->anim);
					tr->anim = NULL;
					//dbg("reach end: anim=%x", tr->anim);
					return EINA_FALSE;
				}
			}
			else {
//				dbg("...");
			}
		}
		else {
//			dbg("continue");
		}

		tr->cursor->cnt = 0;
		if (tr->cursor->duration == 0.0) {
			tr->anim = NULL;
			on_animation(tr, 1.0);
		}
		else {
			tr->anim = ecore_animator_timeline_add(tr->cursor->duration,
					on_animation, user_data);
		}
	}

	return EINA_TRUE;
}

EXPORT_API TTransition& TTransition::start (bool repeat, int start_idx)
{
	this->repeat = repeat;

	if (this->anim) {
		ecore_animator_del(this->anim);
		this->anim = NULL;
	}

	if (this->items.size() < 1)
		return *this;

	cursor = this->items.begin() + start_idx;
	cursor->cnt = 0;
	request_next = false;

	if (cursor->duration == 0.0)
		on_animation(this, 1.0);
	else {
		this->anim = ecore_animator_timeline_add(cursor->duration, on_animation,
				this);
	}

	return *this;
}

EXPORT_API TTransition& TTransition::stop (bool visit_1_0)
{
	if (this->anim) {
		ecore_animator_del(this->anim);
		this->anim = NULL;

		if (visit_1_0) {
			this->repeat = false;
			on_animation(this, 1.0);
		}
	}

	return *this;
}

EXPORT_API TTransition& TTransition::next (bool UNUSED(rightnow))
{
	request_next = true;

	return *this;
}

EXPORT_API bool TTransition::isRunning ()
{
	if (this->anim) {
		//dbg("anim: %x", this->anim);
		return true;
	}
	else
		return false;
}
