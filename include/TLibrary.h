#ifndef INCLUDE_TLIBRARY_H_
#define INCLUDE_TLIBRARY_H_

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#ifdef HOST
#define DEFAULT_WIDTH  (360.0 * elm_config_scale_get())
#define DEFAULT_HEIGHT (360.0 * elm_config_scale_get())
#else
#define DEFAULT_WIDTH  (360.0)
#define DEFAULT_HEIGHT (360.0)
#endif

#define DEFAULT_HALF   (DEFAULT_WIDTH/2)

#define FONT_NAME "Ubuntu"
#define FONT_NAME_MEDIUM "Ubuntu"

#define FONT_SIZE "20"
#define FONT_DESC FONT_NAME " " FONT_SIZE
#define FONT_PATH "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-R.ttf"

#define FONT_TITLE_SIZE    26
#define FONT_SUBTITLE_SIZE 21
#define FONT_CONTENT_SIZE  21

#define TUNER_LAYER_MAX		(EVAS_LAYER_MAX - 10)
#define TUNER_LAYER_CLIP	(TUNER_LAYER_MAX)
#define TUNER_LAYER_TIME	((TUNER_LAYER_CLIP) - 1)
#define TUNER_LAYER_TIME_BG	((TUNER_LAYER_TIME) - 1)

#ifndef HOST

#define TIZEN_ENGINEER_MODE
#include <dlog.h>

#ifndef TUNER_LOG_TAG
#define TUNER_LOG_TAG "TUNER"
#endif

#ifdef info
#undef info
#endif

#ifdef msg
#undef msg
#endif

#ifdef dbg
#undef dbg
#endif

#ifdef warn
#undef warn
#endif

#ifdef err
#undef err
#endif

#ifdef TIZEN_ENGINEER_MODE
#define info(fmt, args ...)   { __dlog_print(LOG_ID_MAIN, DLOG_INFO, TUNER_LOG_TAG, fmt "\n", ## args); }
#define msg(fmt, args ...)    { __dlog_print(LOG_ID_MAIN, DLOG_DEBUG, TUNER_LOG_TAG, fmt "\n", ## args); }
#define dbg(fmt, args ...)    { __dlog_print(LOG_ID_MAIN, DLOG_DEBUG, TUNER_LOG_TAG, "<%s:%d> " fmt "\n", __func__, __LINE__, ## args); }
#else
#define info(fmt, args ...)   ;
#define msg(fmt, args ...)    ;
#define dbg(fmt, args ...)    ;
#endif

#define warn(fmt, args ...)   { __dlog_print(LOG_ID_MAIN, DLOG_WARN, TUNER_LOG_TAG, "<%s:%d> " fmt "\n", __func__, __LINE__, ## args); }
#define err(fmt, args ...)    { __dlog_print(LOG_ID_MAIN, DLOG_ERROR, TUNER_LOG_TAG, "<%s:%d> " fmt "\n", __func__, __LINE__, ## args); }
#define fatal(fmt, args ...)  { __dlog_print(LOG_ID_MAIN, DLOG_FATAL, TUNER_LOG_TAG, "<%s:%d> " fmt "\n", __func__, __LINE__, ## args); }

#else

#define ANSI_COLOR_NORMAL       "\e[0m"

#define ANSI_COLOR_BLACK        "\e[0;30m"
#define ANSI_COLOR_RED          "\e[0;31m"
#define ANSI_COLOR_GREEN        "\e[0;32m"
#define ANSI_COLOR_BROWN        "\e[0;33m"
#define ANSI_COLOR_BLUE         "\e[0;34m"
#define ANSI_COLOR_MAGENTA      "\e[0;35m"
#define ANSI_COLOR_CYAN         "\e[0;36m"
#define ANSI_COLOR_LIGHTGRAY    "\e[0;37m"

#define ANSI_COLOR_DARKGRAY     "\e[1;30m"
#define ANSI_COLOR_LIGHTRED     "\e[1;31m"
#define ANSI_COLOR_LIGHTGREEN   "\e[1;32m"
#define ANSI_COLOR_YELLOW       "\e[1;33m"
#define ANSI_COLOR_LIGHTBLUE    "\e[1;34m"
#define ANSI_COLOR_LIGHTMAGENTA "\e[1;35m"
#define ANSI_COLOR_LIGHTCYAN    "\e[1;36m"
#define ANSI_COLOR_WHITE        "\e[1;37m"

#define info(fmt, args ...)   { printf(ANSI_COLOR_BROWN fmt ANSI_COLOR_NORMAL "\n", ## args); }
#define msg(fmt, args ...)    { printf(fmt "\n", ## args); }
#define dbg(fmt, args ...)    { printf("<%s:%d> " fmt "\n", __func__, __LINE__, ## args); }
#define warn(fmt, args ...)   { printf(ANSI_COLOR_GREEN "<%s:%d> " fmt ANSI_COLOR_NORMAL "\n", __func__, __LINE__, ## args); }
#define err(fmt, args ...)    { printf(ANSI_COLOR_RED "<%s:%d> " fmt ANSI_COLOR_NORMAL "\n", __func__, __LINE__, ## args); }
#define fatal(fmt, args ...)  { printf(ANSI_COLOR_LIGHTRED "<%s:%d> " fmt ANSI_COLOR_NORMAL "\n", __func__, __LINE__, ## args); }

#endif

struct moving_state {
	double current;
	double start;
	double end;
};

struct _box_type {
	int left;
	int right;
	int top;
	int bottom;
};
typedef struct _box_type BoxType;

#include <glib.h>
#include <gio/gio.h>
#include <Elementary.h>
#ifndef HOST
#include <efl_extension.h>
#endif
#include <Ecore_X.h>
#include <Ecore_Ipc.h>
#include <cairo.h>
#include <cairo-ft.h>

#include <TObject.h>
#include <TTransition.h>
#include <TImage.h>
#include <TWindow.h>
#include <TText.h>
#include <TTextBuilder.h>
#include <TContainer.h>

#ifdef __cplusplus
extern "C" {
#endif

char *get_img_path(const char *file);
double tuner_util_convert_angle_to_radian(double angle);
Evas_Object *tuner_util_make_circle_image(Evas_Object *parent,
		const gchar *path, int w, int h);
Evas_Object *tuner_util_make_text_image(Evas_Object *parent,
		const gchar *icon_path, int space, const gchar *text, int size,
		unsigned int rgba, unsigned int rgba_bg, cairo_font_weight_t weight,
		int *result_w, int *result_h);
Evas_Object *tuner_util_round_title(Evas_Object *parent, const char *title,
		unsigned int rgba, unsigned int rgba_bg, gboolean use_bg_pattern);
double get_scale();

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_TLIBRARY_H_ */
