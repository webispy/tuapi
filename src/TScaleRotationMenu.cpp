#include "TLibrary.h"

#include "TScaleRotationMenu.h"

#ifdef DOSIRAK
#define GUIDE_R_ICONLINE (320*136/360)
#else
#define GUIDE_R_ICONLINE (136 * get_scale())
#endif

#define ICON_SIZE (74 * get_scale())
#define PADDING   (10 * get_scale())

EXPORT_API TScaleRotationMenuItem::TScaleRotationMenuItem (TMenu *parent_menu,
		const char *title, const char *object_name) :
		TMenuItem(parent_menu, title, object_name)
{
	angle.current = 0.0;
	angle.end = 0.0;
	angle.start = 0.0;
	scale.current = 0.0;
	scale.end = 0.0;
	scale.start = 0.0;
	opacity_icon.current = 0.0;
	text_y.current = 0.0;
	opacity.current = 0.0;

	radius = GUIDE_R_ICONLINE;
}

EXPORT_API TScaleRotationMenuItem::~TScaleRotationMenuItem ()
{

}

EXPORT_API void TScaleRotationMenuItem::update ()
{
	int alpha;
	double ratio = (double) _parent_menu->width() / DEFAULT_WIDTH;
	double s = ratio * scale.current;

	if (_icon.getStateImage()) {
		double r = _parent_menu->width() * radius / DEFAULT_WIDTH;
		alpha = opacity_icon.current * _parent_menu->getOpacity() / 255;

		switch (static_cast<TScaleRotationMenu*>(_parent_menu)->getVAlign()) {
		case ALIGN_BOTTOM:
			r = r - s * _parent_menu->getIconSize() / 2;
			break;

		case ALIGN_TOP:
			r = r + s * _parent_menu->getIconSize() / 2;
			break;

		case ALIGN_MIDDLE:
			break;

		}

		_icon.getStateImage()->getTransform().moveToAngle(angle.current, r,
				_parent_menu->center_x(), _parent_menu->center_y()).scale(s).apply();

		_icon.getStateImage()->setOpacity(alpha);
	}

	if (_text) {
		int x = _parent_menu->center_x() - _text->width() / 2;
		int y = _parent_menu->end_y() - _parent_menu->getIconSize() * s
				- PADDING - _text->height() * ratio + text_y.current;

		alpha = opacity.current * _parent_menu->getOpacity() / 255;

		_text->getTransform().move(x, y).scale(ratio).apply();
		_text->setOpacity(alpha);
	}
}

EXPORT_API TScaleRotationMenuItem& TScaleRotationMenuItem::stop ()
{
	if (!isVisible())
		return *this;

	if (_icon.getStateImage()->isAnimationStarted()) {
		_icon.getStateImage()->stopAnimation(Transition::END);
	}

	return *this;
}

EXPORT_API void TScaleRotationMenuItem::hide (bool smooth)
{
	if (!isVisible())
		return;

	TMenuItem::hide(smooth);

	if (_icon.getStateImage()->isAnimationStarted()) {
		_icon.getStateImage()->stopAnimation(Transition::END);
	}
}

EXPORT_API TScaleRotationMenuItem& TScaleRotationMenuItem::focus ()
{
	TMenuItem::focus();

	_icon.setState(STATE_FOCUSED);

	if (_icon.getStateImage()->isAnimationSupport() && angle.current == 180.0) {
		_icon.getStateImage()->startAnimation(false);
	}

	if (_parent_menu->isVisible() && _text)
		_text->show();

	return *this;
}

EXPORT_API TScaleRotationMenuItem& TScaleRotationMenuItem::unfocus ()
{
	TMenuItem::unfocus();

	if (_icon.getStateImage()->isAnimationStarted()) {
		_icon.getStateImage()->stopAnimation();
	}

	_icon.setState(STATE_DEFAULT);

	if (_text)
		_text->hide();

	return *this;
}

EXPORT_API TScaleRotationMenu::TScaleRotationMenu (Evas_Object *parent,
		const char *object_name) :
		TMenu(parent, object_name), move_anim(0), showhide_anim(0)
{
	icon_size = ICON_SIZE;
	step_angle = 30;
	_angle.current = 180.0;
	_angle.start = 180.0;
	_angle.end = 180.0;
	_scale.start = 0.4;
	_scale.end = 1.0;
	radius = GUIDE_R_ICONLINE;
	valign = ALIGN_MIDDLE;
}

EXPORT_API TScaleRotationMenu::~TScaleRotationMenu ()
{
	if (move_anim)
		ecore_animator_del(move_anim);

	if (showhide_anim)
		ecore_animator_del(showhide_anim);
}

Eina_Bool TScaleRotationMenu::on_move_anim (void *user_data, double pos)
{
	TScaleRotationMenu *menu = (TScaleRotationMenu *) user_data;

	std::vector<TMenuItem*>::iterator cur;
	TScaleRotationMenuItem *item;
	double frame;
	double r;
	double d;

	//frame = ecore_animator_pos_map(pos, ECORE_POS_MAP_DECELERATE, 0, 0);
	frame = menu->motion.getFrame(pos);

	menu->_angle.current = menu->_angle.start
			+ (menu->_angle.end - menu->_angle.start) * frame;

	r = menu->_angle.current;
	for (cur = menu->items.begin(); cur != menu->items.end(); cur++) {
		item = static_cast<TScaleRotationMenuItem*>(*cur);

		if (r == 180.0) {
			d = menu->_scale.end;
		}
		else if (r > 180.0 - menu->step_angle && r < 180.0) {
			d = menu->_scale.start
					+ (menu->step_angle - (180.0 - r)) / menu->step_angle
							* (menu->_scale.end - menu->_scale.start);
			item->text_y.current = 0 + 15 * frame;
			item->opacity.current = 255 - 255 * frame;
		}
		else if (r < 180.0 + menu->step_angle && r > 180.0) {
			d = menu->_scale.start
					+ (menu->step_angle - (r - 180.0)) / menu->step_angle
							* (menu->_scale.end - menu->_scale.start);
			item->text_y.current = 0 + 15 * frame;
			item->opacity.current = 255 - 255 * frame * 2;
		}
		else {
			d = menu->_scale.start;
		}

		if (cur == menu->cursor) {
			item->text_y.current = 15 + -15 * frame;
			item->opacity.current = 255 * frame;
			if (!item->isFocused())
				item->focus();
		}
		else {
			if (item->isFocused())
				item->unfocus();
		}

		item->angle.current = r;
		item->scale.current = d;

		item->update();
		r -= menu->step_angle;
	}

	if (pos == 1.0) {
		menu->move_anim = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

EXPORT_API TScaleRotationMenu& TScaleRotationMenu::setCursor (int index,
		enum rotation_direction dir)
{
	if (move_anim) {
		ecore_animator_del(move_anim);
		on_move_anim(this, 1.0);
		move_anim = NULL;
	}

	TMenu::setCursor(index, dir);

	return *this;
}

EXPORT_API TScaleRotationMenu& TScaleRotationMenu::update ()
{
	std::vector<TMenuItem*>::iterator cur;
	double angle;
	TScaleRotationMenuItem *item;

	angle = base_angle;
	cur = cursor;
	while (1) {
		item = static_cast<TScaleRotationMenuItem*>(*cur);

		item->angle.current = angle;

		if (cur == items.begin())
			break;

		angle += step_angle;
		cur--;
	}

	item = static_cast<TScaleRotationMenuItem*>(*cur);

	_angle.end = item->angle.current;
	_angle.start = _angle.current;

	angle = base_angle;
	cur = cursor;
	while (1) {
		item = static_cast<TScaleRotationMenuItem*>(*cur);

		item->angle.current = angle;

		if (cur == items.end() - 1)
			break;

		angle -= step_angle;
		cur++;
	}

	if (isVisible())
		move_anim = ecore_animator_timeline_add(.5, on_move_anim, this);
	else
		on_move_anim(this, 1.0);

	return *this;
}

Eina_Bool TScaleRotationMenu::on_show_anim (void *user_data, double pos)
{
	TScaleRotationMenu *menu = (TScaleRotationMenu *) user_data;
	std::vector<TMenuItem*>::iterator cur;
	TScaleRotationMenuItem *item;
	double frame;

	frame = menu->motion.getFrame(pos);

	for (cur = menu->items.begin(); cur != menu->items.end(); cur++) {
		item = static_cast<TScaleRotationMenuItem*>(*cur);

		item->radius = menu->radius + 30 - 30 * frame;
		item->opacity_icon.current = 255 * frame;
		item->opacity.current = 255 * frame;
		item->stop().update();
	}

	if (menu->bg)
		menu->bg->setOpacity(menu->default_bg_opacity * frame);

	if (pos == 1.0) {
		menu->showhide_anim = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool TScaleRotationMenu::on_hide_anim (void *user_data, double pos)
{
	TScaleRotationMenu *menu = (TScaleRotationMenu *) user_data;
	std::vector<TMenuItem*>::iterator cur;
	TScaleRotationMenuItem *item;
	double frame;

	frame = menu->motion.getFrame(pos);

	for (cur = menu->items.begin(); cur != menu->items.end(); cur++) {
		item = static_cast<TScaleRotationMenuItem*>(*cur);

		item->radius = menu->radius + 30 * frame;
		item->opacity_icon.current = 255 - 255 * frame;
		item->opacity.current = 255 - 255 * frame;
		item->stop().update();
		if (pos == 1.0)
			item->hide();
	}

	if (menu->bg)
		menu->bg->setOpacity(
				menu->default_bg_opacity - menu->default_bg_opacity * frame);

	if (pos == 1.0) {
		if (menu->bg) {
			menu->bg->setOpacity(menu->default_bg_opacity);
			menu->bg->hide();
		}

		menu->visibility_ = false;
		menu->showhide_anim = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

EXPORT_API void TScaleRotationMenu::show (bool smooth)
{
	if (showhide_anim) {
		ecore_animator_del(showhide_anim);
		on_hide_anim(this, 1.0);
		showhide_anim = NULL;
	}

	if (isVisible())
		return;

	TMenu::show(smooth);
	on_show_anim(this, 0.0);

	if (smooth)
		showhide_anim = ecore_animator_timeline_add(.5, on_show_anim, this);
	else
		on_show_anim(this, 1.0);
}

EXPORT_API void TScaleRotationMenu::hide (bool smooth)
{
	if (!isVisible())
		return;

	if (move_anim) {
		dbg("remove move_anim");

		ecore_animator_del(move_anim);
		on_move_anim(this, 1.0);
		move_anim = NULL;
	}

	if (showhide_anim) {
		ecore_animator_del(showhide_anim);
		showhide_anim = NULL;
	}

	if (smooth)
		showhide_anim = ecore_animator_timeline_add(.5, on_hide_anim, this);
	else
		on_hide_anim(this, 1.0);
}

