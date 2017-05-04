#include "TLibrary.h"

#include "TSimpleList.h"
#include "TText.h"

EXPORT_API TSimpleListItem::TSimpleListItem(TSimpleList *parent_menu,
		const char *title, const char *subtitle) :
		TVerticalListItem(parent_menu, NULL, NULL)
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
		setTitle(title, 0x121212FF, FONT_TITLE_SIZE * get_scale());

	if (subtitle)
		setText(subtitle, NULL);

	Evas_Object *tmp;
	tmp = evas_object_rectangle_add(
			evas_object_evas_get(parent_menu->getParentEvasObject()));
	evas_object_color_set(tmp, 214, 214, 214, 255);
	evas_object_resize(tmp, DEFAULT_WIDTH, 1);
	evas_object_move(tmp, 0, 0);

	line = new TImage(parent_menu->getParentEvasObject(), tmp);
}

EXPORT_API TSimpleListItem::~TSimpleListItem()
{
	if (line)
		delete line;
}

EXPORT_API void TSimpleListItem::onShow(bool smooth)
{
	TVerticalListItem::onShow(smooth);

	update();
}

EXPORT_API void TSimpleListItem::onHide(bool smooth)
{
	TVerticalListItem::onHide(smooth);

	if (line)
		line->hide();
}

EXPORT_API void TSimpleListItem::update_size()
{
	int w = 0, h = 0;

	if (_icon) {
		_icon->resizeTo(69 * get_scale(),
				69 * get_scale());
	}

	resizeTo(w, 69 * get_scale());

	if (h > _parent_menu->height())
		scroll_max = h - _parent_menu->height();
	else
		scroll_max = 0;

}

EXPORT_API TSimpleListItem& TSimpleListItem::setText(const char *title,
		const char *subtitle, unsigned int rgba, unsigned int sub_rgba)
{
	Evas_Object *txt = NULL;

	if (_content)
		delete _content;

	if (title) {
#ifdef FEATURE_TTEXT
		TText *tmp = new TText();

		tmp->setFont(FONT_NAME, 23 * get_scale()).setColor(rgba).addMarkupLine(
				title, _parent_menu->width() - 72 * get_scale());

		txt = tmp->makeImage(_parent_menu->getParentEvasObject());
		delete tmp;
#endif
#ifdef FEATURE_TBUILDER
		TTextBuilder *tmp = new TTextBuilder(
				_parent_menu->getParentEvasObject());
		tmp->setSize(FONT_CONTENT_SIZE).setColor(rgba).setText(title).done();
		txt = tmp->makeImage();
		delete tmp;
#endif

		_content = new TImage(_parent_menu->getParentEvasObject(), txt);

		if (_parent_menu->isVisible())
			_content->show();
		else
			_content->hide();
	}

	update_size();

	return *this;
}

EXPORT_API void TSimpleListItem::onUpdate()
{
	int x = 0;
	int y = _parent_menu->y() + y_pos.current;
	int alpha;
	//int content_y = 0;
	int max_firstline = 0;
	int start_y;

	y -= y_pos_scroll.current;
	start_y = y;

	alpha = opacity.current * _parent_menu->getOpacity() / 255;

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

	x = _parent_menu->x();
	if (_icon) {
		x += 24 * get_scale();

		_icon->layerTo(_parent_menu->getLayer() + 2);
		_icon->getTransform().reset().move(x, y).scale(scale.current).apply();
		_icon->setOpacity(alpha);

		if (_parent_menu->isVisible()) {
			if (y < DEFAULT_HEIGHT)
				_icon->show();
			else
				_icon->hide();
		}
		else {
			_icon->hide();
		}

		x += _icon->width();
		max_firstline = _icon->height();
	}

	if (_text) {
		x += 10 * get_scale();

		_text->layerTo(_parent_menu->getLayer() + 2);
		if (_content) {
			_text->getTransform().reset().move(x, y).scale(scale.current).apply();
		}
		else
			_text->getTransform().reset().move(x,
					y + height() / 2 - _text->height() / 2).scale(scale.current).apply();

		_text->setOpacity(alpha);

		if (_parent_menu->isVisible()) {
			if (y < DEFAULT_HEIGHT)
				_text->show();
			else
				_text->hide();
		}
		else {
			_text->hide();
		}

		if (_text->height() > max_firstline)
			max_firstline = _text->height();
	}

	y += (max_firstline * scale.current);

	if (_content) {
		y += 5 * get_scale();
		_content->layerTo(_parent_menu->getLayer() + 2);
		_content->getTransform().reset().move(x, y).scale(scale.current).apply();
		_content->setOpacity(alpha);

		if (_parent_menu->isVisible()) {
			if (y < DEFAULT_HEIGHT)
				_content->show();
			else
				_content->hide();
		}
		else {
			_content->hide();
		}

		y += _content->height() * scale.current;
	}

	if (line) {
		x = _parent_menu->center_x() - line->width() / 2;

		y = start_y + height() + (_parent_menu->padsize/2) - line->height();

		line->layerTo(_parent_menu->getLayer());
		line->moveTo(x, y);

		if (_parent_menu->isVisible()) {
			if (y < DEFAULT_HEIGHT)
				line->show();
			else
				line->hide();
		}
		else {
			line->hide();
		}

	}
}

EXPORT_API TSimpleList::TSimpleList(Evas_Object *parent) :
		TVerticalList(parent)
{
	setBackgroundColor(246, 246, 246, 255);
	setCursorColor(240, 188, 7, 255);

	padsize = 22;
	setAlign(false, 48 + padsize / 2);

	cursor_mode = 1;
}

EXPORT_API TSimpleList::~TSimpleList()
{

}
