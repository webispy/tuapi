#ifndef MAIN_H_
#define MAIN_H_

#include <Elementary.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "basicui"

#if !defined(PACKAGE)
#define PACKAGE "org.example.basicui"
#endif

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
} appdata_s;

#ifdef __cplusplus
extern "C" {
#endif

void create_view(appdata_s *ad);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H_ */
