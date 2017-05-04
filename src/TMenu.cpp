#include "TLibrary.h"
#include "TMenu.h"
#include "TText.h"

EXPORT_API TMenuItem::TMenuItem (TMenu *parent_menu, const char *title,
		const char *object_name) :
		TObject(parent_menu->getParentEvasObject(), object_name), _enabled(
				false), _focused(false), _checked(false), _title(0), _background(
				0), _text(0)
{
	_parent_menu = parent_menu;

	if (title)
		setTitle(title);
}

EXPORT_API TMenuItem::TMenuItem (TMenu *parent_menu, const char *object_name) :
		TObject(parent_menu->getParentEvasObject(), object_name), _enabled(
				false), _focused(false), _checked(false)
{
	_background = NULL;
	_text = NULL;
	_parent_menu = parent_menu;
	_title = NULL;
}

EXPORT_API TMenuItem::~TMenuItem ()
{
	if (_title)
		g_free(_title);

	if (_text)
		delete _text;

	if (_background)
		delete _background;
}

EXPORT_API const char *TMenuItem::getTitle ()
{
	return _title;
}

EXPORT_API void TMenuItem::onMove (int x, int y)
{
//	dbg("%s::(%d,%d)", getName(), x, y);

	_icon.moveTo(x, y);
//	if (_icon.getStateImage())
//		_icon.getStateImage()->getTransform().move(x, y).apply();
}

EXPORT_API void TMenuItem::onResize (int w, int h)
{
//	dbg("%s::(%d,%d)", getName(), w, h);

	_icon.resizeTo(w, h);
//	if (_icon.getStateImage())
//		_icon.getStateImage()->getTransform().resize(w, h).apply();

}

EXPORT_API void TMenuItem::onLayer (int layer)
{
	_icon.layerTo(layer);
	if (_text)
		_text->layerTo(layer);
}

EXPORT_API TMenuItem& TMenuItem::setTitle (const char *title, unsigned int rgba,
		int size)
{
	Evas_Object *txt;
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
	tmp->setFont(FONT_NAME, size, PANGO_WEIGHT_BOLD).setColor(rgba).setAlignment(
			PANGO_ALIGN_LEFT).addMarkupLine(title);

	txt = tmp->makeImage(_parent_menu->getParentEvasObject());
	delete tmp;
#endif
#ifdef FEATURE_TBUILDER
	tmp->setSize(size).setWeight(TTEXT_BUILDER_WEIGHT_BOLD).setColor(rgba).setText(_title).done();
	txt = tmp->makeImage();
	delete tmp;
#endif

	_text = new TImage(_parent_menu->getParentEvasObject(), txt);
	if (_text)
		_text->layerTo(_parent_menu->getLayer());

	if (isVisible())
		_text->show();
	else
		_text->hide();

	return *this;
}

EXPORT_API TMenuItem& TMenuItem::setIcon (const char *path,
		enum timage_state_types type)
{
	if (!path)
		_icon.setImage(NULL, type);
	else {
		_icon.setImage(new TImage(_parent_menu->getParentEvasObject(), path),
				type);
		_icon.getImage(type)->layerTo(_parent_menu->getLayer());
		//_icon.getImage(type)->resizeTo(width(), height());
	}

	return *this;
}

EXPORT_API TMenuItem& TMenuItem::setIcon (TImage *icon,
		enum timage_state_types type)
{
	if (!icon)
		_icon.setImage(NULL, type);
	else {
		_icon.setImage(icon, type);
		_icon.getImage(type)->layerTo(_parent_menu->getLayer());
	}

	return *this;
}

EXPORT_API void TMenuItem::show (bool smooth)
{
	TObject::show(smooth);

	_icon.show(smooth);

	if (_text) {
		if (isCursor()) {
			_text->show(smooth);
		}
		else {
			_text->hide();
		}
	}

//	dbg("show");
}

EXPORT_API void TMenuItem::hide (bool smooth)
{
	TObject::hide(smooth);

	_icon.hide(smooth);

	if (_text)
		_text->hide(smooth);

//	dbg("hide");
}

EXPORT_API void TMenuItem::update ()
{

}

EXPORT_API TMenuItem& TMenuItem::focus ()
{
	_focused = true;

	_icon.setState(STATE_FOCUSED);

	return *this;
}

EXPORT_API TMenuItem& TMenuItem::unfocus ()
{
	_focused = false;

	_icon.setState(STATE_DEFAULT);

	return *this;
}

EXPORT_API bool TMenuItem::isFocused () const
{
	return _focused;
}

EXPORT_API TMenuItem& TMenuItem::enable ()
{
	_enabled = true;

	return *this;
}

EXPORT_API TMenuItem& TMenuItem::disable ()
{
	_enabled = false;

	return *this;
}

EXPORT_API bool TMenuItem::isEnabled () const
{
	return _enabled;
}

EXPORT_API TMenuItem& TMenuItem::check ()
{
	_checked = false;

	return *this;
}

EXPORT_API TMenuItem& TMenuItem::uncheck ()
{
	_checked = false;

	return *this;
}

EXPORT_API bool TMenuItem::isChecked () const
{
	return _checked;
}

EXPORT_API bool TMenuItem::isCursor ()
{
	TMenuItem *item;

	if (!_parent_menu)
		return false;

	item = _parent_menu->getCursor();
	if (item == this)
		return true;

	return false;
}

EXPORT_API TMenu::TMenu (Evas_Object *parent, const char *object_name) :
		TObject(parent, object_name), bg(0), default_bg_opacity(64), dir(
				TMenu::COUNTER_CW)
{
	moveTo(0, 0);
	resizeWith(parent);

	base_angle = 180.0;
	step_angle = 30.0;
	icon_size = 64;
	radius = rect_.w / 2 - icon_size / 2;
}

EXPORT_API TMenu::~TMenu ()
{
	if (bg)
		delete bg;

	cursor = items.begin();
	for (; cursor != items.end(); ++cursor) {
		delete (*cursor);
	}

	items.erase(items.begin(), items.end());
}

EXPORT_API TMenu& TMenu::append (TMenuItem *item, bool to_bottom)
{
	item->resizeTo(icon_size, icon_size);
	item->layerTo(getLayer() + 1);

	if (to_bottom)
		items.push_back(item);
	else
		items.insert(items.begin(), item);

	cursor = items.begin();
	prev_cursor = cursor;

	return *this;
}

EXPORT_API TMenu& TMenu::update ()
{
	std::vector<TMenuItem*>::iterator cur;
	double angle;

	angle = base_angle + (cursor - items.begin()) * step_angle;

	if (bg)
		bg->moveTo(rect_.x, rect_.y).resizeTo(rect_.w, rect_.h);

	cur = items.begin();
	for (; cur != items.end(); ++cur) {
		(*cur)->moveToAngle(angle, radius, this);
		angle -= step_angle;
	}

	return *this;
}

EXPORT_API void TMenu::show (bool smooth)
{
	std::vector<TMenuItem*>::iterator cur;

	TObject::show(smooth);

	if (bg)
		bg->show(smooth);

	cur = items.begin();
	for (; cur != items.end(); ++cur) {
		(*cur)->show(smooth);
	}

	//dbg("show");
}

EXPORT_API void TMenu::hide (bool smooth)
{
	std::vector<TMenuItem*>::iterator cur;

	TObject::hide(smooth);

	if (bg)
		bg->hide(smooth);

	cur = items.begin();
	for (; cur != items.end(); ++cur) {
		(*cur)->hide(smooth);
	}

	//dbg("hide");
}

EXPORT_API TMenu& TMenu::refresh ()
{
	std::vector<TMenuItem*>::iterator cur;

	if (bg) {
		bg->moveTo(rect_.x, rect_.y).resizeTo(rect_.w, rect_.h);
		bg->setOpacity(default_bg_opacity * getOpacity() / 255);
	}

	cur = items.begin();
	for (; cur != items.end(); ++cur) {
		(*cur)->update();
	}

	return *this;
}

EXPORT_API TMenuItem* TMenu::getCursor ()
{
	return *cursor;
}

EXPORT_API int TMenu::getCursorIndex ()
{
	return cursor - items.begin();
}

EXPORT_API int TMenu::getItemCount ()
{
	return items.size();
}

EXPORT_API TMenu& TMenu::setCursor (int index, enum rotation_direction dir)
{
	int ci = cursor - items.begin();

	if (ci == index)
		return *this;

	if (dir == UNKNOWN) {
		int diff = ci - index;
		int mid = items.size() / 2;

		if (abs(diff) <= mid) {
			if (diff > 0)
				dir = COUNTER_CW;
			else
				dir = CW;
		}
		else {
			if (diff > 0)
				dir = CW;
			else
				dir = COUNTER_CW;
		}
	}

	this->dir = dir;

	prev_cursor = cursor;
	cursor = items.begin() + index;

#if 0
	dbg("Cursor:'%s', Prev-cursor:'%s'", (*cursor)->getTitle(),
			(*prev_cursor)->getTitle());
#endif

	update();

	return *this;
}

EXPORT_API TMenu& TMenu::next (bool circular)
{
	if (cursor == items.end() - 1) {
		if (!circular)
			return *this;
		else
			setCursor(0, CW);
	}
	else
		setCursor(cursor - items.begin() + 1, CW);

	return *this;
}

EXPORT_API TMenu& TMenu::prev (bool circular)
{
	if (cursor == items.begin()) {
		if (!circular)
			return *this;
		else
			setCursor(items.end() - items.begin() - 1, COUNTER_CW);
	}
	else
		setCursor(cursor - items.begin() - 1, COUNTER_CW);

	return *this;
}

EXPORT_API TMenu& TMenu::setBackground (const char *path, int default_opacity)
{
	dbg("path:%s'", path);
	if (bg)
		delete bg;

	bg = NULL;

	if (path) {
		bg = new TImage(getParentEvasObject(), path);
		bg->moveTo(rect_.x, rect_.y).resizeTo(rect_.w, rect_.h);
		bg->layerTo(getLayer());
		bg->setOpacity(default_opacity);
	}

	default_bg_opacity = default_opacity;

	return *this;
}

EXPORT_API TMenu& TMenu::setBackground (TImage *img, int default_opacity)
{
	dbg("img:%p", img);

	if (bg)
		delete bg;

	bg = NULL;

	if (img) {
		bg = img;
		bg->moveTo(rect_.x, rect_.y).resizeTo(rect_.w, rect_.h);
		bg->layerTo(getLayer());
		bg->setOpacity(default_opacity);
	}

	default_bg_opacity = default_opacity;

	return *this;
}

EXPORT_API TMenu& TMenu::setBackgroundColor (int r, int g, int b, int a)
{
	if (bg)
		delete bg;

	bg = new TImage(getParentEvasObject(), rect_.w, rect_.h);
	bg->fillTo(r, g, b, a);
	bg->moveTo(rect_.x, rect_.y).resizeTo(rect_.w, rect_.h);
	bg->layerTo(getLayer());

	default_bg_opacity = a;

	return *this;
}

EXPORT_API TImage* TMenu::getBackground ()
{
	return bg;
}
