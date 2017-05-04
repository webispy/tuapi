#ifndef TTRANSITON_H
#define TTRANSITON_H

#include <vector>
#include <TObject.h>

namespace Transition
{
enum timing_function
{
	MANUAL, EASE, LINEAR, EASE_IN, EASE_OUT, EASE_IN_OUT
};

enum position
{
	START, CURRENT, END
};
}

class TTransition;

typedef Eina_Bool (*TransitionFunction) (TTransition *t, TObject *target,
		double pos, double frame, void *user_data);

struct bezier_info
{
	double cx, bx, ax;
	double cy, by, ay;
};

struct tr_item
{
	double duration;
	bool repeat;
	TransitionFunction func;
	void *user_data;
	Transition::timing_function timing;

	struct bezier_info bi;
	int cnt;
};

class TTransitionPreset
{
public:
	TTransitionPreset (TTransition *tr);
	virtual ~TTransitionPreset ();
};

class TTransition
{
private:
	TObject *target;
	Ecore_Animator *anim;
	bool repeat;
	bool request_next;
	std::vector<struct tr_item> items;
	std::vector<struct tr_item>::iterator cursor;
	static Eina_Bool on_animation (void *user_data, double pos);

public:
	explicit TTransition (TObject *target);
	virtual ~TTransition ();

	TTransition& append (double duration, Transition::timing_function t,
			TransitionFunction func, void *user_data, bool repeat = false);
	TTransition& start (bool repeat = false, int start_idx = 0);
	TTransition& stop (bool visit_1_0 = false);
	TTransition& next (bool rightnow = false);
	bool isRunning ();
};

class TBezier
{
private:
	double x1, y1, x2, y2;
	struct bezier_info bi;

public:
	explicit TBezier (double x1, double y1, double x2, double y2);
	virtual ~TBezier ();

	double getFrame (double pos);

};

class TBezierCubicEaseIn: public TBezier
{
public:
	TBezierCubicEaseIn () :
			TBezier(0.55, 0.055, 0.675, 0.19)
	{
	}
	virtual ~TBezierCubicEaseIn ()
	{
	}
};

class TBezierCubicEaseOut: public TBezier
{
public:
	TBezierCubicEaseOut () :
			TBezier(0.215, 0.61, 0.355, 1)
	{
	}
	virtual ~TBezierCubicEaseOut ()
	{
	}
};
#endif
