#ifndef TSIMPLELIST_H
#define TSIMPLELIST_H

#include <TVerticalList.h>

class TSimpleList;

class TSimpleListItem: public TVerticalListItem
{
protected:
	TImage *line;
	void update_size ();
	virtual void onShow (bool smooth = true);
	virtual void onHide (bool smooth = true);
	virtual void onUpdate ();

public:
	explicit TSimpleListItem (TSimpleList *parent_menu, const char *title,
			const char *subtitle = NULL);
	virtual ~TSimpleListItem ();

	TSimpleListItem& setText (const char *title, const char *subtitle = NULL,
			unsigned int rgba = 0x878787FF, unsigned int sub_rgba = 0x757575FF);

};

class TSimpleList: public TVerticalList
{
public:
	explicit TSimpleList (Evas_Object *parent);
	virtual ~TSimpleList ();
};

#endif
