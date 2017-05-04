extern "C"
{
#include "main.h"
}

#include "TLibrary.h"
#include "TVerticalList.h"

static TWindow *win;
static TVerticalList *list;

static void create_tvlist()
{
	TVerticalListItem *item;

	list = new TVerticalList(win->getParentEvasObject());

	item = new TVerticalListItem(list, "Clock face");
	list->append(item);

	item = new TVerticalListItem(list, "클래쉬오브클랜", "엘릭서 정재소 레벨 10 업그레이드 완료!!");
	list->append(item);

	item = new TVerticalListItem(list, "Information");
	item->setText("haha", "hoho");
	list->append(item);

	item = new TVerticalListItem(list, "Power off", "Reset\nOff");
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
	win->show();
	win->setTimeBgColor(40, 40, 40);
	win->setTimeVisible(true);

	/* Label */
	ad->label = elm_label_add(win->getEvasObject());
	elm_object_text_set(ad->label, "<align=center>Hello Tizen</align>");
	evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND,
	EVAS_HINT_EXPAND);
	//elm_object_content_set(win->getEvasObject(), ad->label);
	evas_object_hide(ad->label);

	create_tvlist();
}

}
