extern "C"
{
#include "main.h"
}

#include "TLibrary.h"
#include "TSimpleList.h"

static TWindow *win;
static TSimpleList *list;

static void create_tvlist()
{
	TSimpleListItem *item;

	list = new TSimpleList(win->getParentEvasObject());

	item = new TSimpleListItem(list, "Lucas", "Hi June!");
	list->append(item);

	item = new TSimpleListItem(list, "Sophia", "Great! See you then");
	list->append(item);

	item = new TSimpleListItem(list, "Emma", "{emoticon}");
	list->append(item);

	item = new TSimpleListItem(list, "David", "{emoticon}");
	list->append(item);

	list->update();
	list->show();
}

class MyWin: public TWindow
{
protected:
	virtual void onWheel(int dir)
	{
		if (dir > 0)
			list->next();
		else
			list->prev();
	}

public:
	explicit MyWin(const char *name) :
			TWindow(name)
	{
	}
	virtual ~MyWin()
	{
	}
};

extern "C"
{

void create_view(appdata_s *ad)
{
	win = new MyWin("haha");
	win->resizeTo(DEFAULT_WIDTH, DEFAULT_WIDTH);
	win->show();
	win->setTimeColor(0,0,0);
	win->setTimeBgColor(255,255,255);
	win->setTimeVisible(true);

	create_tvlist();
}

}
