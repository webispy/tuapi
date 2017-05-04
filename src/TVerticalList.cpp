#include "TLibrary.h"

#include "TVerticalList.h"

#define ICON_SIZE      (32*get_scale())
#define ITEMS_IN_PAGE  3

EXPORT_API TVerticalListItem::TVerticalListItem (TVerticalList *parent_menu,
		const char *title, const char *subtitle, const char *object_name) :
		TObject(parent_menu->getParentEvasObject(), object_name), _title(NULL), _icon(
		NULL), _background(
		NULL), _text(NULL), _parent_menu(parent_menu), _content(NULL)
{
	y_pos.current = 0.0;
	y_pos.end = 0.0;
	y_pos.start = 0.0;
	scale.current = 0.0;
	scale.end = 0.0;
	scale.start = 0.0;
	scroll_max = 0;
	y_pos_scroll.current = 0.0;
	y_pos_scroll.end = 0.0;
	y_pos_scroll.start = 0.0;

	if (title)
		setTitle(title);

	if (subtitle)
		setText(subtitle);
}

EXPORT_API TVerticalListItem::~TVerticalListItem ()
{
	if (_title)
		g_free(_title);
	if (_icon)
		delete _icon;
	if (_background)
		delete _background;
	if (_text)
		delete _text;
	if (_content)
		delete _content;
}

EXPORT_API TVerticalListItem& TVerticalListItem::setIcon (const char *path)
{
	if (_icon)
		delete _icon;

	_icon = new TImage(_parent_menu->getParentEvasObject(), path);
	_icon->layerTo(_parent_menu->getLayer());

	update_size();

	return *this;
}

EXPORT_API TVerticalListItem& TVerticalListItem::setIcon (TImage *icon)
{
	if (_icon)
		delete _icon;

	_icon = icon;
	if (_icon)
		_icon->layerTo(_parent_menu->getLayer());

	update_size();

	return *this;
}

EXPORT_API void TVerticalListItem::update_size ()
{
	int w = 0, h = 0;

	if (_text) {
		w = _text->width();
		h = _text->height();
	}

	if (_content) {
		if (w < _content->width())
			w = _content->width();
		h = h + _content->height();
	}

	if (_icon) {
		if (w < _icon->width())
			w = _icon->width();
		h = h + _icon->height();
	}

	resizeTo(w, h);

	if (h > _parent_menu->height())
		scroll_max = h - _parent_menu->height();
	else
		scroll_max = 0;

}

EXPORT_API TVerticalListItem& TVerticalListItem::setTitle (const char *title,
		unsigned int rgba, int size)
{
	Evas_Object *txt = NULL;
#ifdef FEATURE_TTEXT
	TText *tmp = new TText();
#endif
#ifdef FEATURE_TBUILDER
	TTextBuilder *tmp = new TTextBuilder(_parent_menu->getParentEvasObject());
#endif

	if (_title)
		g_free(_title);

	_title = g_strdup(title);

	if (_text)
		delete _text;

#ifdef FEATURE_TTEXT
	if (_title)
		tmp->setFont(FONT_NAME, size, PANGO_WEIGHT_BOLD).setColor(rgba).setAlignment(
				PANGO_ALIGN_LEFT).addLine(_title, _parent_menu->width() - 72,
		FALSE);

	txt = tmp->makeImage(_parent_menu->getParentEvasObject());
	delete tmp;
#endif
#ifdef FEATURE_TBUILDER
	if (_title)
		tmp->setSize(size).setColor(rgba).setText(_title).done();
	txt = tmp->makeImage();
	delete tmp;
#endif

	_text = new TImage(_parent_menu->getParentEvasObject(), txt);
	if (_text)
		_text->layerTo(_parent_menu->getLayer());

	if (_parent_menu->isVisible())
		_text->show();
	else
		_text->hide();

	update_size();

	return *this;
}

EXPORT_API TVerticalListItem& TVerticalListItem::setText (const char *title,
		const char *subtitle, unsigned int rgba, unsigned int sub_rgba)
{
	Evas_Object *txt = NULL;
#ifdef FEATURE_TTEXT
	TText *tmp = new TText();
#endif
#ifdef FEATURE_TBUILDER
	TTextBuilder *tmp = new TTextBuilder(_parent_menu->getParentEvasObject());
#endif

	if (_content)
		delete _content;

#ifdef FEATURE_TTEXT
	if (title)
		tmp->setFont(FONT_NAME, FONT_TITLE_SIZE * get_scale()).setColor(rgba).setAlignment(
				PANGO_ALIGN_CENTER).addLine(title, _parent_menu->width());

	if (subtitle)
		tmp->setFont(FONT_NAME, FONT_SUBTITLE_SIZE * get_scale()).setColor(sub_rgba).setAlignment(
				PANGO_ALIGN_RIGHT).addLine(subtitle, _parent_menu->width());

	txt = tmp->makeImage(_parent_menu->getParentEvasObject());
	delete tmp;
#endif
#ifdef FEATURE_TBUILDER
	if (title)
		tmp->setSize(FONT_TITLE_SIZE).setColor(rgba).setText(title).done();
	if (subtitle)
		tmp->setSize(FONT_SUBTITLE_SIZE).setWeight(TTEXT_BUILDER_WEIGHT_ULTRABOLD).setColor(rgba).setText(subtitle).done();
	txt = tmp->makeImage();
	delete tmp;
#endif

	_content = new TImage(_parent_menu->getParentEvasObject(), txt);
	if (_content)
		_content->layerTo(_parent_menu->getLayer());

	if (_parent_menu->isVisible())
		_content->show();
	else
		_content->hide();

	update_size();

	return *this;
}

EXPORT_API void TVerticalListItem::onShow (bool smooth)
{
	update();
}

EXPORT_API void TVerticalListItem::onHide (bool smooth)
{
	if (_text)
		_text->hide();

	if (_content)
		_content->hide();

	if (_icon)
		_icon->hide();
}

EXPORT_API void TVerticalListItem::onUpdate ()
{
	int x = _parent_menu->center_x();
	int y = _parent_menu->y() + y_pos.current;
	//- ((TVerticalList*) _parent_menu)->scroll_y.current;
	int alpha;

	if (_text)
		x = x - _text->width() / 2;

	y -= y_pos_scroll.current;

	alpha = opacity.current * _parent_menu->getOpacity() / 255;

	//dbg("y: %d, alpha: %d", y, alpha);

	if (_parent_menu->isCursorObject(this)) {
		if (scroll_max != 0.0) {
			if (y_pos_scroll.end == scroll_max) {
				((TVerticalList*) _parent_menu)->_more_top_bg->show();
				((TVerticalList*) _parent_menu)->_more_bottom_bg->hide();
			}
			else if (y_pos_scroll.end == 0) {
				((TVerticalList*) _parent_menu)->_more_top_bg->hide();
				((TVerticalList*) _parent_menu)->_more_bottom_bg->show();
			}
			else {
				((TVerticalList*) _parent_menu)->_more_top_bg->show();
				((TVerticalList*) _parent_menu)->_more_bottom_bg->show();
			}
		}
	}

	if (_text) {
		_text->layerTo(_parent_menu->getLayer() + 2);
		_text->getTransform().reset().move(x, y).scale(scale.current).apply();
		_text->setOpacity(alpha);

		if (_parent_menu->isVisible()) {
			if (y + _text->height() * scale.current > 0 && y < DEFAULT_HEIGHT)
				_text->show();
			else
				_text->hide();
		}
		else {
			_text->hide();
		}

		y = y + _text->height() * scale.current;
	}

	if (_content) {
		x = _parent_menu->center_x() - _content->width() / 2;

		_content->layerTo(_parent_menu->getLayer() + 2);
		_content->getTransform().reset().move(x, y).scale(scale.current).apply();
		_content->setOpacity(alpha);

		if (_parent_menu->isVisible()) {
			if (y + _content->height() * scale.current > 0 && y < DEFAULT_HEIGHT)
				_content->show();
			else
				_content->hide();
		}
		else {
			_content->hide();
		}

		y = y + _content->height() * scale.current;
	}

	if (_icon) {
		x = _parent_menu->center_x() - _icon->width() / 2;

		_icon->layerTo(_parent_menu->getLayer() + 2);
		_icon->getTransform().reset().move(x, y).scale(scale.current).apply();
		_icon->setOpacity(alpha);

		if (_parent_menu->isVisible()) {
			if (y + _icon->height() * scale.current > 0 && y < DEFAULT_HEIGHT)
				_icon->show();
			else
				_icon->hide();
		}
		else {
			_icon->hide();
		}
	}

}

EXPORT_API TVerticalList::TVerticalList (Evas_Object *parent) :
		TContainer(parent), bg_(NULL), is_cursor_pos_middle_(1), cursor_pos_offset_(
				0), item_min_height(0), title_area(0), move_anim(0), padsize(30)
{
	gchar *tmp_path;

	cursor_bg_ = new TImage(getParentEvasObject(), rect_.w, rect_.h);
	cursor_bg_->fillTo(25, 123, 226, 255);

	cursor_bg_->hide();
	cursor_bg_->layerTo(getLayer() + 1);

	tmp_path = get_img_path("/grad_top.png");
	_more_top_bg = new TImage(parent, tmp_path);
	g_free(tmp_path);

	tmp_path = get_img_path("/grad_bottom.png");
	_more_bottom_bg = new TImage(parent, tmp_path);
	g_free(tmp_path);

	_more_top_bg->moveTo(0, 0).hide();
	_more_bottom_bg->moveTo(0, 0).hide();

	_more_top_bg->layerTo(getLayer() + 3);
	_more_bottom_bg->layerTo(getLayer() + 3);

	bg_sy.current = 0.0;
	bg_ey.current = 0.0;
	scroll_y.current = 0.0;
	scroll_y.end = 0.0;

	tr_bounce_top = new TTransition(this);
	tr_bounce_top->append(1.0, Transition::MANUAL, __anim_bounce_top, NULL,
			false);

	tr_bounce_bottom = new TTransition(this);
	tr_bounce_bottom->append(1.0, Transition::MANUAL, __anim_bounce_bottom,
	NULL, false);

	scroller_ = new TScroller(this);
	scroller_->setLayer(getLayer() + 4);

	setAlign(true, 0);

	cursor_mode = 0;
//	dbg("x=%d, y=%d", _rect.x, _rect.y);
}

EXPORT_API TVerticalList::~TVerticalList ()
{
	if (bg_)
		delete bg_;

	if (cursor_bg_)
		delete cursor_bg_;

	if (tr_bounce_top)
		delete tr_bounce_top;
	if (tr_bounce_bottom)
		delete tr_bounce_bottom;

	if (title_area)
		delete title_area;

	if (_more_top_bg)
		delete _more_top_bg;
	if (_more_bottom_bg)
		delete _more_bottom_bg;

	if (move_anim) {
		ecore_animator_del(move_anim);
		move_anim = NULL;
	}
}

EXPORT_API void TVerticalList::setBackgroundColor (int r, int g, int b, int a)
{
	if (bg_)
		delete bg_;

	bg_ = new TImage(getParentEvasObject(), rect_.w, rect_.h);
	bg_->fillTo(r, g, b, a);
	bg_->moveTo(rect_.x, rect_.y).resizeTo(rect_.w, rect_.h);
	bg_->layerTo(getLayer());
}

EXPORT_API void TVerticalList::setBackground (const char *path)
{
	if (bg_)
		delete bg_;

	bg_ = new TImage(getParentEvasObject(), path);
	bg_->moveTo(rect_.x, rect_.y).resizeTo(rect_.w, rect_.h);
	bg_->layerTo(getLayer());
}

EXPORT_API void TVerticalList::setBackground (TImage *img)
{
	if (bg_)
		delete bg_;

	bg_ = new TImage(getParentEvasObject(), img);
	bg_->moveTo(rect_.x, rect_.y).resizeTo(rect_.w, rect_.h);
	bg_->layerTo(getLayer());
}

EXPORT_API void TVerticalList::setCursorColor (int r, int g, int b, int a)
{
	cursor_bg_->fillTo(r, g, b, a);
}

EXPORT_API void TVerticalList::setAlign (bool isAlignMiddle, int offset)
{
	is_cursor_pos_middle_ = isAlignMiddle;

	if (is_cursor_pos_middle_)
		cursor_pos_offset_ = center_y();
	else
		cursor_pos_offset_ = 0;

	cursor_pos_offset_ += offset;
}

Eina_Bool TVerticalList::__anim_bounce_top (TTransition *t, TObject *target,
		double pos, double frame, void *user_data)
{
	TVerticalList *list = (TVerticalList *) target;

	frame = ecore_animator_pos_map(pos, ECORE_POS_MAP_BOUNCE, 3, 6);

	list->moveTo(0, 50 * (1.0 - frame));
	list->refresh();

	return EINA_TRUE;
}

Eina_Bool TVerticalList::__anim_bounce_bottom (TTransition *t, TObject *target,
		double pos, double frame, void *user_data)
{
	TVerticalList *list = (TVerticalList *) target;

	frame = ecore_animator_pos_map(pos, ECORE_POS_MAP_BOUNCE, 3, 6);

	list->moveTo(0, -50 * (1.0 - frame));
	list->refresh();

	return EINA_TRUE;
}

EXPORT_API void TVerticalList::show_cursor_bg ()
{
	if (cursor_bg_)
		cursor_bg_->show();
}

EXPORT_API void TVerticalList::hide_cursor_bg ()
{
	if (cursor_bg_)
		cursor_bg_->hide();
}

EXPORT_API void TVerticalList::onShow (bool smooth)
{
	TContainer::onShow(smooth);

	if (scroller_)
		scroller_->show();

	if (bg_)
		bg_->show();

	if (cursor_bg_)
		cursor_bg_->show();

	if (title_area)
		title_area->show();
}

EXPORT_API void TVerticalList::onHide (bool smooth)
{
	if (move_anim) {
		ecore_animator_del(move_anim);
		on_move_anim(this, 1.0);
		move_anim = NULL;
	}

	if (scroller_)
		scroller_->hide();

	if (bg_)
		bg_->hide();

	if (cursor_bg_)
		cursor_bg_->hide();

	if (_more_top_bg)
		_more_top_bg->hide();

	if (_more_bottom_bg)
		_more_bottom_bg->hide();

	if (title_area)
		title_area->hide();

	TContainer::onHide(smooth);
}

EXPORT_API void TVerticalList::onAdded (TObject *item)
{
	if (item) {
//	item->resizeTo(icon_size, icon_size);
		item->layerTo(getLayer() + 1);
	}
}

EXPORT_API void TVerticalList::onResize (int w, int h)
{
//	dbg("resize w=%d, h=%d", w, h);
}

static int get_y_pos (bool align_center, int offset, int item_height)
{
	if (align_center)
		return offset - item_height / 2;
	else
		return offset;
}

Eina_Bool TVerticalList::on_move_anim (void *user_data, double pos)
{
	TVerticalList *menu = (TVerticalList *) user_data;
	GList *cur;
	TVerticalListItem *item;
	double frame;
	double y_pos = 0;

	frame = ecore_animator_pos_map(pos, ECORE_POS_MAP_DECELERATE, 0, 0);

	menu->scroll_y.current = menu->scroll_y.start
			+ (menu->scroll_y.end - menu->scroll_y.start) * frame;

	item = static_cast<TVerticalListItem*>(menu->getCursorObject());
	if (!item)
		return EINA_FALSE;

	y_pos = get_y_pos(menu->is_cursor_pos_middle_, menu->cursor_pos_offset_,
			item->height());

	cur = g_list_first(menu->list_);
	while (cur) {
		item = static_cast<TVerticalListItem*>(cur->data);
		if (item) {
			item->y_pos.current = item->y_pos.end - menu->scroll_y.current
					+ y_pos;
			item->update();
		}

		cur = g_list_next(cur);
	}

	menu->bg_sy.current = menu->bg_sy.start
			+ (menu->bg_sy.end - menu->bg_sy.start) * frame;
	menu->bg_ey.current = menu->bg_ey.start
			+ (menu->bg_ey.end - menu->bg_ey.start) * frame;

	item = static_cast<TVerticalListItem*>(menu->getCursorObject());
	menu->cursor_bg_->resizeTo(menu->width(),
			menu->bg_ey.current - menu->bg_sy.current);
	if (menu->cursor_mode == 0) {
		menu->cursor_bg_->moveTo(menu->x(),
				menu->y() + menu->bg_sy.current - menu->scroll_y.current);
		menu->scroller_->set(menu->scroll_y.current);
	}
	else {
		menu->cursor_bg_->moveTo(menu->x(), menu->y() + menu->bg_sy.current);
		menu->scroller_->set(
				menu->scroll_y.current + menu->bg_sy.current
						- menu->cursor_pos_offset_ + menu->padsize / 2);
	}

	if (pos == 1.0)
		menu->move_anim = NULL;

	return EINA_TRUE;
}

EXPORT_API TVerticalList& TVerticalList::refresh ()
{
	GList *cur;
	TVerticalListItem *item = NULL;

	cursor_bg_->resizeTo(width(), bg_ey.current - bg_sy.current);
	if (cursor_mode == 0)
		cursor_bg_->moveTo(x(), y() + bg_sy.current - scroll_y.current);
	else
		cursor_bg_->moveTo(x(), y() + bg_sy.current);
	cursor_bg_->layerTo(getLayer() + 1);

	_more_top_bg->moveTo(x(), 0).hide();
	_more_bottom_bg->moveTo(x(), 0).hide();

	cur = g_list_first(list_);
	while (cur) {
		item = static_cast<TVerticalListItem*>(cur->data);
		if (item)
			item->update();

		cur = g_list_next(cur);
	}

	return *this;
}

EXPORT_API void TVerticalList::onUpdate ()
{
	GList *cur;
	double y_pos = 0.0;
	TVerticalListItem *item = NULL;

	if (!list_)
		return;

	cur = g_list_first(list_);
	while (cur) {
		item = static_cast<TVerticalListItem*>(cur->data);
		if (item) {
			item->y_pos.end = y_pos;
			item->y_pos.start = y_pos;

			item->scale.end = 1;
			item->scale.current = 1;
			item->scale.start = 1;

			item->opacity.end = 255;
			item->opacity.current = 255;
			item->opacity.start = 255;

			y_pos += item->height();
			y_pos += padsize;
		}

		cur = g_list_next(cur);
	}

	int last_item_size = padsize;
	if (item)
		last_item_size = item->height() + padsize;

	scroller_->update(y_pos, last_item_size);

	if (cursor_mode == 0) {
		item = static_cast<TVerticalListItem*>(getCursorObject());
		scroll_y.end = item->y_pos.end;
		scroll_y.start = scroll_y.current;

		scroller_->set(scroll_y.current);

		y_pos = get_y_pos(is_cursor_pos_middle_, cursor_pos_offset_,
				item->height());

		bg_sy.end = item->y_pos.end + y_pos - padsize / 2;
		bg_sy.start = bg_sy.current;
		bg_ey.end = item->y_pos.end + item->height() + y_pos + padsize / 2;
		bg_ey.start = bg_ey.current;

		move_anim = ecore_animator_timeline_add(.2, on_move_anim, this);
	}
	else {
		int base_y = cursor_pos_offset_ - padsize / 2;

		item = static_cast<TVerticalListItem*>(getCursorObject());
		bg_sy.start = bg_sy.current;
		bg_ey.start = bg_ey.current;

		bg_ey.end = item->y_pos.end + item->height() + cursor_pos_offset_
				+ padsize / 2 - scroll_y.end;
		bg_sy.end = bg_ey.end - item->height() - padsize;

		if (bg_ey.end > height()) {
			TVerticalListItem *tmp = NULL;
			cur = g_list_first(list_);
			scroll_y.end = 0;
			while (cur) {
				tmp = static_cast<TVerticalListItem*>(cur->data);
				scroll_y.end += tmp->height() + padsize;

				bg_ey.end = item->y_pos.end + item->height()
						+ cursor_pos_offset_ + padsize / 2 - scroll_y.end;

				if (bg_ey.end < height())
					break;

				cur = g_list_next(cur);
			}
			bg_sy.end = bg_ey.end - item->height() - padsize;
		}
		else if (bg_sy.end < base_y) {
			scroll_y.end -= item->height() + padsize;
			bg_sy.end = base_y;
			bg_ey.end = bg_sy.end + item->height() + padsize;
		}

		scroll_y.start = scroll_y.current;
		move_anim = ecore_animator_timeline_add(.2, on_move_anim, this);

		scroller_->set(
				scroll_y.current + bg_sy.current - cursor_pos_offset_
						+ padsize / 2);
	}

}

Eina_Bool TVerticalList::on_item_scroll_anim (void *user_data, double pos)
{
	TVerticalList *menu = (TVerticalList *) user_data;
	double frame;
	TVerticalListItem *item =
			static_cast<TVerticalListItem*>(menu->getCursorObject());

	if (!item)
		return EINA_FALSE;

	frame = ecore_animator_pos_map(pos, ECORE_POS_MAP_DECELERATE, 0, 0);

	item->y_pos_scroll.current = item->y_pos_scroll.start
			+ (item->y_pos_scroll.end - item->y_pos_scroll.start) * frame;
	item->update();

	if (pos == 1.0)
		menu->move_anim = NULL;

	return EINA_TRUE;
}

EXPORT_API void TVerticalList::item_scroll ()
{
	if (move_anim) {
		ecore_animator_del(move_anim);
		move_anim = NULL;
	}

	move_anim = ecore_animator_timeline_add(.2, on_item_scroll_anim, this);
}

EXPORT_API TVerticalList& TVerticalList::next (bool circular)
{
	TVerticalListItem *item = static_cast<TVerticalListItem*>(getCursorObject());

	if (getCursorIndex() == (int) (getSize() - 1)) {
		tr_bounce_bottom->start();
		return *this;
	}

	if (tr_bounce_top->isRunning())
		tr_bounce_top->stop(true);
	if (tr_bounce_bottom->isRunning())
		tr_bounce_bottom->stop(true);

	if (!item)
		return *this;

	if (item->height() > height()) {
		if (item->y_pos_scroll.end == item->scroll_max) {
			item->y_pos_scroll.current = item->y_pos_scroll.end;
			TContainer::next(circular);
		}
		else {
			item->y_pos_scroll.end += 30.0;
			if (item->y_pos_scroll.end > item->scroll_max)
				item->y_pos_scroll.end = item->scroll_max;

			item->y_pos_scroll.start = item->y_pos_scroll.current;

			item_scroll();
		}
	}
	else {
		TContainer::next(circular);
	}

	return *this;
}

EXPORT_API TVerticalList& TVerticalList::prev (bool circular)
{
	TVerticalListItem *item = static_cast<TVerticalListItem*>(getCursorObject());

	if (getCursorIndex() == 0) {
		tr_bounce_top->start();
		return *this;
	}

	if (tr_bounce_top->isRunning())
		tr_bounce_top->stop(true);
	if (tr_bounce_bottom->isRunning())
		tr_bounce_bottom->stop(true);

	if (!item)
		return *this;

	if (item->height() > height()) {
		if (item->y_pos_scroll.end == 0.0) {
			item->y_pos_scroll.current = item->y_pos_scroll.end;
			TContainer::prev(circular);
		}
		else {
			item->y_pos_scroll.end -= 30.0;
			if (item->y_pos_scroll.end < 0.0)
				item->y_pos_scroll.end = 0.0;

			item->y_pos_scroll.start = item->y_pos_scroll.current;

			item_scroll();
		}
	}
	else {
		TContainer::prev(circular);
	}

	return *this;
}

EXPORT_API void TVerticalList::onCursorChanged (GList *prev)
{
	TContainer::onCursorChanged(prev);

	update();
}

EXPORT_API void TVerticalList::setTitle (const char *title, unsigned int rgba,
		unsigned int rgba_bg, gboolean use_bg_pattern)
{
	if (title_area)
		delete title_area;
	title_area = NULL;

	if (title) {
		Evas_Object *tmp;
		tmp = tuner_util_round_title(this->parent_eo_, title, rgba, rgba_bg,
				use_bg_pattern);
		title_area = new TImage(parent_eo_, tmp);
		title_area->layerTo(TUNER_LAYER_TIME_BG - 1);
	}
}
