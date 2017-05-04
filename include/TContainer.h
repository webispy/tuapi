#ifndef TCONTAINER_H
#define TCONTAINER_H

#include <glib.h>
#include <TObject.h>
#include <TScroller.h>

class TContainer;

class TContainerIterator
{
protected:
	GList *target_list_;
	GList *pos_;

public:
	explicit TContainerIterator (TContainer *target);
	virtual ~TContainerIterator ();

	void setContainer (TContainer *target);

	TObject *getObject ();

	TContainerIterator& begin ();
	TContainerIterator& end ();
	TContainerIterator& next (bool circular = false);
	TContainerIterator& prev (bool circular = false);

	bool isFirst ();
	bool isLast ();

	friend class TContainer;
};

class TContainer: public TObject
{
protected:
	TScroller *scroller_;
	GList *list_;
	GList *cursor_;
	virtual void onAdded (TObject *item);
	virtual void onRemoved (TObject *item);
	virtual void onShow (bool smooth);
	virtual void onHide (bool smooth);
	virtual void onCursorChanged (GList *prev);
	virtual void onLayer (int layer);

public:
	BoxType padding;

public:
	explicit TContainer (Evas_Object *parent_eo, const char *object_name =
			"TContainer");
	virtual ~TContainer ();

	TObject *getObjectNth (unsigned int index);
	unsigned int getSize ();

	TObject *getCursorObject ();
	int getCursorIndex ();
	bool isCursorObject (const TObject *obj);
	TContainer& setCursorIndex (unsigned int index);
	TContainer& setCursorObject (const TObject *child);
	TContainer& setCursor (const TContainerIterator *iter);

	virtual TContainer& next (bool circular = false);
	virtual TContainer& prev (bool circular = false);

	TContainer& append (TObject *child, bool to_head = FALSE);
	TContainer& remove (TObject *child, bool with_delete = TRUE);
	TContainer& removeNth (unsigned int index, bool with_delete = TRUE);
	TContainer& removeAll (bool with_delete = TRUE);

	void updateSize ();
	void dump ();

	friend class TContainerIterator;
};

#endif
