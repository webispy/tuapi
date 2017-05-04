#ifndef TSCROLLER_LIST_H
#define TSCROLLER_LIST_H

#include <TObject.h>
#include <TImage.h>

class TScroller
{
protected:
	TObject *target;
	int target_max_height;
	TImage *bg;
	TImage *bar;
	double bar_size;

public:
	explicit TScroller (TObject *target);
	virtual ~TScroller ();

	void update (int max_height, int last_item_size = 0);
	void set (int position);
	void show ();
	void hide ();
	void setLayer (int layer);
};

#endif
