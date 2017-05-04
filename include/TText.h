#ifndef TTEXT_H
#define TTEXT_H

//#include <pango/pangocairo.h>
#include <TObject.h>

typedef enum {
  PANGO_ALIGN_LEFT,
  PANGO_ALIGN_CENTER,
  PANGO_ALIGN_RIGHT
} PangoAlignment;

typedef enum {
  PANGO_STYLE_NORMAL,
  PANGO_STYLE_OBLIQUE,
  PANGO_STYLE_ITALIC
} PangoStyle;

typedef enum {
  PANGO_WEIGHT_THIN = 100,
  PANGO_WEIGHT_ULTRALIGHT = 200,
  PANGO_WEIGHT_LIGHT = 300,
  PANGO_WEIGHT_BOOK = 380,
  PANGO_WEIGHT_NORMAL = 400,
  PANGO_WEIGHT_MEDIUM = 500,
  PANGO_WEIGHT_SEMIBOLD = 600,
  PANGO_WEIGHT_BOLD = 700,
  PANGO_WEIGHT_ULTRABOLD = 800,
  PANGO_WEIGHT_HEAVY = 900,
  PANGO_WEIGHT_ULTRAHEAVY = 1000
} PangoWeight;

typedef struct _PangoContext {} PangoContext;
typedef struct _PangoLayout {} PangoLayout;
typedef struct _PangoFontDescription {} PangoFontDescription;

class TText
{
private:
	cairo_surface_t *sf;
	cairo_t *cr;
	FT_Face ft_face;
	cairo_font_face_t *font_face;
	cairo_font_options_t *font_opt;

	int total_height;
	int total_width;
	unsigned int color;
	bool useTrim;
	bool useDropShadow;
	unsigned int drop_color;
	int drop_off_x;
	int drop_off_y;

public:
	explicit TText ();
	virtual ~TText ();

	int getTextWidth ()
	{
		return total_width;
	}
	int getTextHeight ()
	{
		return total_height;
	}

	TText& setTextWidth (int w)
	{
		total_width = w;
		return *this;
	}
	TText& setTextHeight (int h)
	{
		total_height = h;
		return *this;
	}
	TText& setTrim (bool onoff)
	{
		useTrim = onoff;
		return *this;
	}
	TText& setFont (const char *description);
	TText& setFont (const char *family, int size, PangoWeight weight =
			PANGO_WEIGHT_NORMAL, PangoStyle style = PANGO_STYLE_NORMAL);
	TText& setColor (int r, int g, int b, int a);
	TText& setColor (unsigned rgba);
	TText& setAlignment (PangoAlignment align);
	TText& setDropShadow (unsigned int rgba = 0x00000080, int off_x = 0,
			int off_y = 1);

	TText& addMarkupLine (const char *str, int wrap_width = -1,
			bool fill_width = TRUE);
	TText& addLine (const char *str, int wrap_width = -1,
			bool fill_width = TRUE);

	cairo_surface_t *makeSurface (unsigned int rgba_bg = 0, int crop_x = 0,
			int crop_y = 0, int crop_width = -1, int crop_height = -1);
	Evas_Object *makeImage (Evas_Object *parent, unsigned int rgba_bg = 0,
			int crop_x = 0, int crop_y = 0, int crop_width = -1,
			int crop_height = -1);
};

#endif
