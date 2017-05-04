#include "TLibrary.h"

#include "TContainer.h"

EXPORT_API TContainer::TContainer (Evas_Object *parent_eo,
		const char *object_name) :
		TObject(parent_eo, object_name), scroller_(NULL), list_(NULL), cursor_(
		NULL)
{
	padding.bottom = 0;
	padding.left = 0;
	padding.right = 0;
	padding.top = 0;

	moveTo(0, 0);
	resizeWith(parent_eo);
}

EXPORT_API TContainer::~TContainer ()
{
	if (scroller_)
		delete scroller_;

	removeAll(TRUE);
}

EXPORT_API TObject* TContainer::getObjectNth (unsigned int index)
{
	GList *tmp;

	if (!list_)
		return NULL;

	tmp = g_list_nth(list_, index);
	if (!tmp)
		return NULL;

	return static_cast<TObject *>(tmp->data);
}

EXPORT_API TObject* TContainer::getCursorObject ()
{
	if (!cursor_ || !list_)
		return NULL;

	return static_cast<TObject *>(cursor_->data);
}

EXPORT_API int TContainer::getCursorIndex ()
{
	if (!cursor_ || !list_)
		return 0;

	return g_list_position(list_, cursor_);
}

EXPORT_API bool TContainer::isCursorObject (const TObject *obj)
{
	if (!cursor_ || !list_ || !obj)
		return false;

	if (cursor_->data != obj)
		return false;

	return true;
}

EXPORT_API TContainer& TContainer::setCursorIndex (unsigned int index)
{
	GList *tmp;
	GList *prev = cursor_;

	if (!list_)
		return *this;

	tmp = g_list_nth(list_, index);
	if (!tmp)
		return *this;

	cursor_ = tmp;

	onCursorChanged(prev);

	return *this;
}

EXPORT_API TContainer& TContainer::setCursorObject (const TObject *child)
{
	GList *tmp;
	GList *prev = cursor_;

	if (!list_ || !child)
		return *this;

	tmp = g_list_find(list_, child);
	if (!tmp)
		return *this;

	cursor_ = tmp;

	onCursorChanged(prev);

	return *this;
}

EXPORT_API TContainer& TContainer::setCursor (const TContainerIterator *iter)
{
	GList *prev = cursor_;

	if (!list_)
		return *this;

	cursor_ = iter->pos_;

	onCursorChanged(prev);

	return *this;
}

EXPORT_API unsigned int TContainer::getSize ()
{
	if (!list_)
		return 0;

	return g_list_length(list_);
}

EXPORT_API TContainer& TContainer::next (bool circular)
{
	GList *prev = cursor_;

	if (!list_)
		return *this;

	cursor_ = g_list_next(cursor_);
	if (!cursor_) {
		if (circular)
			cursor_ = g_list_first(list_);
		else
			cursor_ = g_list_last(list_);
	}

	onCursorChanged(prev);

	return *this;
}

EXPORT_API TContainer& TContainer::prev (bool circular)
{
	GList *prev = cursor_;

	if (!list_)
		return *this;

	cursor_ = g_list_previous(cursor_);
	if (!cursor_) {
		if (circular)
			cursor_ = g_list_last(list_);
		else
			cursor_ = g_list_first(list_);
	}

	onCursorChanged(prev);

	return *this;
}

EXPORT_API TContainer& TContainer::append (TObject *child, bool to_head)
{
	if (!child)
		return *this;

	if (to_head)
		list_ = g_list_prepend(list_, child);
	else
		list_ = g_list_append(list_, child);

	if (!cursor_) {
		cursor_ = g_list_first(list_);
		onCursorChanged(NULL);
	}

	if (child->x() + child->width() > width()
			|| child->y() + child->height() > height())
		updateSize();

	dump();

	onAdded(child);

	return *this;
}

EXPORT_API TContainer& TContainer::remove (TObject *child, bool with_delete)
{
	GList *tmp;
	int pos = -1;

	if (!child || !list_)
		return *this;

	tmp = g_list_find(list_, child);
	if (!tmp)
		return *this;

	if (tmp == cursor_)
		pos = g_list_position(list_, tmp);

	list_ = g_list_remove_link(list_, tmp);

	if (pos != -1) {
		cursor_ = g_list_nth(list_, pos);
		if (!cursor_)
			cursor_ = g_list_last(list_);

		onCursorChanged(NULL);
	}

	if (child->x() + child->width() == width()
			|| child->y() + child->height() == height())
		updateSize();

	onRemoved(child);

	if (with_delete)
		delete child;

	dump();

	return *this;
}

EXPORT_API TContainer& TContainer::removeNth (unsigned int index,
		bool with_delete)
{
	GList *tmp;

	if (!list_)
		return *this;

	tmp = g_list_nth(list_, index);
	if (!tmp)
		return *this;

	return remove(static_cast<TObject *>(tmp->data), with_delete);
}

EXPORT_API TContainer& TContainer::removeAll (bool with_delete)
{
	if (!list_)
		return *this;

	if (with_delete) {
		GList *cur = list_;
		TObject *obj;

		while (cur) {
			obj = static_cast<TObject *>(cur->data);
			if (obj) {
				onRemoved(obj);
				delete obj;
			}
			cur = cur->next;
		}
	}

	g_list_free(list_);

	list_ = NULL;
	cursor_ = NULL;

	onCursorChanged(NULL);
	updateSize();

	return *this;
}

EXPORT_API void TContainer::onAdded (TObject *item)
{
	if (debug_enable) {
		dbg("Added: '%s'", item->getName());
	}
}

EXPORT_API void TContainer::onRemoved (TObject *item)
{
	if (debug_enable) {
		dbg("Removed: '%s'", item->getName());
	}
}

EXPORT_API void TContainer::onShow (bool smooth)
{
	GList *cur = list_;
	TObject *obj;

	while (cur) {
		obj = static_cast<TObject *>(cur->data);
		if (obj)
			obj->show();

		cur = cur->next;
	}
}

EXPORT_API void TContainer::onHide (bool smooth)
{
	GList *cur = list_;
	TObject *obj;

	while (cur) {
		obj = static_cast<TObject *>(cur->data);
		if (obj)
			obj->hide();

		cur = cur->next;
	}
}

EXPORT_API void TContainer::onLayer (int layer)
{
	GList *cur = list_;
	TObject *obj;

	while (cur) {
		obj = static_cast<TObject *>(cur->data);
		if (obj)
			obj->layerTo(layer);

		cur = cur->next;
	}
}

EXPORT_API void TContainer::onCursorChanged (GList *prev)
{
	dump();
}

static void _dump (gpointer data, gpointer user_data)
{
	TObject *obj = static_cast<TObject *>(data);
	GList *cursor = static_cast<GList *>(user_data);

	if (!cursor || !obj)
		return;

	if (data == cursor->data) {
		info("Object: '%s' <- cursor", obj->getName());
	}
	else {
		info("Object: '%s'", obj->getName());
	}
}

EXPORT_API void TContainer::dump ()
{
	if (!debug_enable)
		return;

	if (list_ && g_list_length(list_))
		g_list_foreach(list_, _dump, cursor_);
	else {
		info("No items");
	}
}

EXPORT_API void TContainer::updateSize ()
{
	GList *cur = list_;
	TObject *obj;
	int w = width();
	int h = height();

	while (cur) {
		obj = static_cast<TObject *>(cur->data);
		if (obj) {
			obj->hide();
			if (obj->x() + obj->width() > w) {
				w = obj->x() + obj->width();
				dbg("fuck!!, x=%d, width=%d", obj->x(), obj->width())
			}
			if (obj->y() + obj->height() > h)
				h = obj->y() + obj->height();
		}

		cur = cur->next;
	}

	resizeTo(w, h);
}

EXPORT_API TContainerIterator::TContainerIterator (TContainer *target)
{
	setContainer(target);
}

EXPORT_API TContainerIterator::~TContainerIterator ()
{

}

EXPORT_API TObject* TContainerIterator::getObject ()
{
	if (!pos_)
		return NULL;

	return static_cast<TObject *>(pos_->data);
}

EXPORT_API void TContainerIterator::setContainer (TContainer *target)
{
	if (target)
		target_list_ = target->list_;
	else
		target_list_ = NULL;

	pos_ = target_list_;
}

EXPORT_API TContainerIterator& TContainerIterator::begin ()
{
	pos_ = g_list_first(target_list_);

	return *this;
}

EXPORT_API TContainerIterator& TContainerIterator::end ()
{
	pos_ = g_list_last(target_list_);

	return *this;
}

EXPORT_API TContainerIterator& TContainerIterator::next (bool circular)
{
	pos_ = g_list_next(pos_);
	if (!pos_) {
		if (circular)
			pos_ = g_list_first(target_list_);
		else
			pos_ = g_list_last(target_list_);
	}

	return *this;
}

EXPORT_API TContainerIterator& TContainerIterator::prev (bool circular)
{
	pos_ = g_list_previous(pos_);
	if (!pos_) {
		if (circular)
			pos_ = g_list_last(target_list_);
		else
			pos_ = g_list_first(target_list_);
	}

	return *this;
}

EXPORT_API bool TContainerIterator::isFirst ()
{
	if (pos_ == g_list_first(target_list_))
		return TRUE;

	return FALSE;
}

EXPORT_API bool TContainerIterator::isLast ()
{
	if (pos_ == g_list_last(target_list_))
		return TRUE;

	return FALSE;
}
