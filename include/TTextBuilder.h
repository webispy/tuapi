#ifndef TTEXTBUILDER_H
#define TTEXTBUILDER_H

#include <TObject.h>

enum TTextBuilderAlign {
	TTEXT_BUILDER_ALIGN_LEFT,
	TTEXT_BUILDER_ALIGN_CENTER,
	TTEXT_BUILDER_ALIGN_RIGHT
};


enum TTextBuilderVAlign {
	TTEXT_BUILDER_VALIGN_TOP,
	TTEXT_BUILDER_VALIGN_CENTER,
	TTEXT_BUILDER_VALIGN_BOTTOM,
	TTEXT_BUILDER_VALIGN_BASELINE
};

enum TTextBuilderWeight {
	TTEXT_BUILDER_WEIGHT_NORMAL,
	TTEXT_BUILDER_WEIGHT_THIN,
	TTEXT_BUILDER_WEIGHT_ULTRALIGHT,
	TTEXT_BUILDER_WEIGHT_LIGHT,
	TTEXT_BUILDER_WEIGHT_BOOK,
	TTEXT_BUILDER_WEIGHT_MEDIUM,
	TTEXT_BUILDER_WEIGHT_SEMIBOLD,
	TTEXT_BUILDER_WEIGHT_BOLD,
	TTEXT_BUILDER_WEIGHT_ULTRABOLD,
	TTEXT_BUILDER_WEIGHT_BLACK,
	TTEXT_BUILDER_WEIGHT_EXTRABLACK,
};

struct font {
	gchar *name;
	gchar *weight;
	gchar *color;
	gchar *bgcolor;
	gchar *align;
	double valign;
	int size;
};

class TTextBuilder
{
private:
	Evas_Object *pwin;
	Evas_Object *win;
	Evas_Object *tb;

	gchar *saved_text;
	GList *srcs;
	int total_height;
	int total_width;
	struct font font_style;

public:
	explicit TTextBuilder (Evas_Object *parent_win);
	virtual ~TTextBuilder ();

	TTextBuilder& setColor (const char *color);
	TTextBuilder& setColor (unsigned int rgba);
	TTextBuilder& setBgColor (const char *color);
	TTextBuilder& setBgColor (unsigned int rgba);
	TTextBuilder& setSize (int size);
	TTextBuilder& setAlign (enum TTextBuilderAlign align);
	TTextBuilder& setVAlign (enum TTextBuilderVAlign valign);
	TTextBuilder& setWeight (enum TTextBuilderWeight weight);
	TTextBuilder& setText (const char *str);
	TTextBuilder& setFont (const char *str);
	TTextBuilder& done (int width = -1, int height = -1);

	cairo_surface_t *makeSurface (unsigned int rgba_bg = 0, int crop_x = 0,
			int crop_y = 0, int crop_width = -1, int crop_height = -1);
	Evas_Object *makeImage (unsigned int rgba_bg = 0,
			int crop_x = 0, int crop_y = 0, int crop_width = -1,
			int crop_height = -1);
};

#endif
