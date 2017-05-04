#include "TLibrary.h"

#include "TImage.h"

#include <cairo.h>

/* ------------------------------ stackblur -------------------------------- */

static unsigned short const stackblur_mul[255] =
{ 512, 512, 456, 512, 328, 456, 335, 512, 405, 328, 271, 456, 388, 335, 292,
		512, 454, 405, 364, 328, 298, 271, 496, 456, 420, 388, 360, 335, 312,
		292, 273, 512, 482, 454, 428, 405, 383, 364, 345, 328, 312, 298, 284,
		271, 259, 496, 475, 456, 437, 420, 404, 388, 374, 360, 347, 335, 323,
		312, 302, 292, 282, 273, 265, 512, 497, 482, 468, 454, 441, 428, 417,
		405, 394, 383, 373, 364, 354, 345, 337, 328, 320, 312, 305, 298, 291,
		284, 278, 271, 265, 259, 507, 496, 485, 475, 465, 456, 446, 437, 428,
		420, 412, 404, 396, 388, 381, 374, 367, 360, 354, 347, 341, 335, 329,
		323, 318, 312, 307, 302, 297, 292, 287, 282, 278, 273, 269, 265, 261,
		512, 505, 497, 489, 482, 475, 468, 461, 454, 447, 441, 435, 428, 422,
		417, 411, 405, 399, 394, 389, 383, 378, 373, 368, 364, 359, 354, 350,
		345, 341, 337, 332, 328, 324, 320, 316, 312, 309, 305, 301, 298, 294,
		291, 287, 284, 281, 278, 274, 271, 268, 265, 262, 259, 257, 507, 501,
		496, 491, 485, 480, 475, 470, 465, 460, 456, 451, 446, 442, 437, 433,
		428, 424, 420, 416, 412, 408, 404, 400, 396, 392, 388, 385, 381, 377,
		374, 370, 367, 363, 360, 357, 354, 350, 347, 344, 341, 338, 335, 332,
		329, 326, 323, 320, 318, 315, 312, 310, 307, 304, 302, 299, 297, 294,
		292, 289, 287, 285, 282, 280, 278, 275, 273, 271, 269, 267, 265, 263,
		261, 259 };

static unsigned char const stackblur_shr[255] =
{ 9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 17,
		17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19,
		19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
		20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
		21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22,
		22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
		22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
		23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
		23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
		23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24 };

/// Stackblur algorithm body
static void _stackblurJob(unsigned char* src,			///< input image data
		unsigned int w,					///< image width
		unsigned int h,					///< image height
		unsigned int radius,	///< blur intensity (should be in 2..254 range)
		int cores,						///< total number of working threads
		int core,							///< current thread number
		int step,							///< step of processing (1,2)
		unsigned char* stack				///< stack buffer
		)
{
	unsigned int x, y, xp, yp, i;
	unsigned int sp;
	unsigned int stack_start;
	unsigned char* stack_ptr;

	unsigned char* src_ptr;
	unsigned char* dst_ptr;

	unsigned long sum_r;
	unsigned long sum_g;
	unsigned long sum_b;
	unsigned long sum_in_r;
	unsigned long sum_in_g;
	unsigned long sum_in_b;
	unsigned long sum_out_r;
	unsigned long sum_out_g;
	unsigned long sum_out_b;

	unsigned int wm = w - 1;
	unsigned int hm = h - 1;
	unsigned int w4 = w * 4;
	unsigned int div = (radius * 2) + 1;
	unsigned int mul_sum = stackblur_mul[radius];
	unsigned char shr_sum = stackblur_shr[radius];

	if (step == 1)
	{
		unsigned int minY = core * h / cores;
		unsigned int maxY = (core + 1) * h / cores;

		for (y = minY; y < maxY; y++)
		{
			sum_r = sum_g = sum_b = sum_in_r = sum_in_g = sum_in_b = sum_out_r =
					sum_out_g = sum_out_b = 0;

			src_ptr = src + w4 * y; // start of line (0,y)

			for (i = 0; i <= radius; i++)
			{
				stack_ptr = &stack[4 * i];
				stack_ptr[0] = src_ptr[0];
				stack_ptr[1] = src_ptr[1];
				stack_ptr[2] = src_ptr[2];
				sum_r += src_ptr[0] * (i + 1);
				sum_g += src_ptr[1] * (i + 1);
				sum_b += src_ptr[2] * (i + 1);
				sum_out_r += src_ptr[0];
				sum_out_g += src_ptr[1];
				sum_out_b += src_ptr[2];
			}

			for (i = 1; i <= radius; i++)
			{
				if (i <= wm)
					src_ptr += 4;
				stack_ptr = &stack[4 * (i + radius)];
				stack_ptr[0] = src_ptr[0];
				stack_ptr[1] = src_ptr[1];
				stack_ptr[2] = src_ptr[2];
				sum_r += src_ptr[0] * (radius + 1 - i);
				sum_g += src_ptr[1] * (radius + 1 - i);
				sum_b += src_ptr[2] * (radius + 1 - i);
				sum_in_r += src_ptr[0];
				sum_in_g += src_ptr[1];
				sum_in_b += src_ptr[2];
			}

			sp = radius;
			xp = radius;
			if (xp > wm)
				xp = wm;
			src_ptr = src + 4 * (xp + y * w); //   img.pix_ptr(xp, y);
			dst_ptr = src + y * w4; // img.pix_ptr(0, y);
			for (x = 0; x < w; x++)
			{
				dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
				dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
				dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
				dst_ptr += 4;

				sum_r -= sum_out_r;
				sum_g -= sum_out_g;
				sum_b -= sum_out_b;

				stack_start = sp + div - radius;
				if (stack_start >= div)
					stack_start -= div;
				stack_ptr = &stack[4 * stack_start];

				sum_out_r -= stack_ptr[0];
				sum_out_g -= stack_ptr[1];
				sum_out_b -= stack_ptr[2];

				if (xp < wm)
				{
					src_ptr += 4;
					++xp;
				}

				stack_ptr[0] = src_ptr[0];
				stack_ptr[1] = src_ptr[1];
				stack_ptr[2] = src_ptr[2];

				sum_in_r += src_ptr[0];
				sum_in_g += src_ptr[1];
				sum_in_b += src_ptr[2];
				sum_r += sum_in_r;
				sum_g += sum_in_g;
				sum_b += sum_in_b;

				++sp;
				if (sp >= div)
					sp = 0;
				stack_ptr = &stack[sp * 4];

				sum_out_r += stack_ptr[0];
				sum_out_g += stack_ptr[1];
				sum_out_b += stack_ptr[2];
				sum_in_r -= stack_ptr[0];
				sum_in_g -= stack_ptr[1];
				sum_in_b -= stack_ptr[2];

			}

		}
	}

	// step 2
	if (step == 2)
	{
		unsigned int minX = core * w / cores;
		unsigned int maxX = (core + 1) * w / cores;

		for (x = minX; x < maxX; x++)
		{
			sum_r = sum_g = sum_b = sum_in_r = sum_in_g = sum_in_b = sum_out_r =
					sum_out_g = sum_out_b = 0;

			src_ptr = src + 4 * x; // x,0
			for (i = 0; i <= radius; i++)
			{
				stack_ptr = &stack[i * 4];
				stack_ptr[0] = src_ptr[0];
				stack_ptr[1] = src_ptr[1];
				stack_ptr[2] = src_ptr[2];
				sum_r += src_ptr[0] * (i + 1);
				sum_g += src_ptr[1] * (i + 1);
				sum_b += src_ptr[2] * (i + 1);
				sum_out_r += src_ptr[0];
				sum_out_g += src_ptr[1];
				sum_out_b += src_ptr[2];
			}
			for (i = 1; i <= radius; i++)
			{
				if (i <= hm)
					src_ptr += w4; // +stride

				stack_ptr = &stack[4 * (i + radius)];
				stack_ptr[0] = src_ptr[0];
				stack_ptr[1] = src_ptr[1];
				stack_ptr[2] = src_ptr[2];
				sum_r += src_ptr[0] * (radius + 1 - i);
				sum_g += src_ptr[1] * (radius + 1 - i);
				sum_b += src_ptr[2] * (radius + 1 - i);
				sum_in_r += src_ptr[0];
				sum_in_g += src_ptr[1];
				sum_in_b += src_ptr[2];
			}

			sp = radius;
			yp = radius;
			if (yp > hm)
				yp = hm;
			src_ptr = src + 4 * (x + yp * w); // img.pix_ptr(x, yp);
			dst_ptr = src + 4 * x; 			  // img.pix_ptr(x, 0);
			for (y = 0; y < h; y++)
			{
				dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
				dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
				dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
				dst_ptr += w4;

				sum_r -= sum_out_r;
				sum_g -= sum_out_g;
				sum_b -= sum_out_b;

				stack_start = sp + div - radius;
				if (stack_start >= div)
					stack_start -= div;
				stack_ptr = &stack[4 * stack_start];

				sum_out_r -= stack_ptr[0];
				sum_out_g -= stack_ptr[1];
				sum_out_b -= stack_ptr[2];

				if (yp < hm)
				{
					src_ptr += w4; // stride
					++yp;
				}

				stack_ptr[0] = src_ptr[0];
				stack_ptr[1] = src_ptr[1];
				stack_ptr[2] = src_ptr[2];

				sum_in_r += src_ptr[0];
				sum_in_g += src_ptr[1];
				sum_in_b += src_ptr[2];
				sum_r += sum_in_r;
				sum_g += sum_in_g;
				sum_b += sum_in_b;

				++sp;
				if (sp >= div)
					sp = 0;
				stack_ptr = &stack[sp * 4];

				sum_out_r += stack_ptr[0];
				sum_out_g += stack_ptr[1];
				sum_out_b += stack_ptr[2];
				sum_in_r -= stack_ptr[0];
				sum_in_g -= stack_ptr[1];
				sum_in_b -= stack_ptr[2];
			}
		}
	}
}

/// Stackblur algorithm body
static void _stackblurJob_Alpha(unsigned char* src,	///< input image data
		unsigned int w,					///< image width
		unsigned int h,					///< image height
		unsigned int radius,	///< blur intensity (should be in 2..254 range)
		int cores,						///< total number of working threads
		int core,							///< current thread number
		int step,							///< step of processing (1,2)
		unsigned char* stack				///< stack buffer
		)
{
	unsigned int x, y, xp, yp, i;
	unsigned int sp;
	unsigned int stack_start;
	unsigned char* stack_ptr;

	unsigned char* src_ptr;
	unsigned char* dst_ptr;

	unsigned long sum_r;
	unsigned long sum_g;
	unsigned long sum_b;
	unsigned long sum_a;
	unsigned long sum_in_r;
	unsigned long sum_in_g;
	unsigned long sum_in_b;
	unsigned long sum_in_a;
	unsigned long sum_out_r;
	unsigned long sum_out_g;
	unsigned long sum_out_b;
	unsigned long sum_out_a;

	unsigned int wm = w - 1;
	unsigned int hm = h - 1;
	unsigned int w4 = w * 4;
	unsigned int div = (radius * 2) + 1;
	unsigned int mul_sum = stackblur_mul[radius];
	unsigned char shr_sum = stackblur_shr[radius];

	if (step == 1)
	{
		unsigned int minY = core * h / cores;
		unsigned int maxY = (core + 1) * h / cores;

		for (y = minY; y < maxY; y++)
		{
			sum_r = sum_g = sum_b = sum_a = sum_in_r = sum_in_g = sum_in_b =
					sum_in_a = sum_out_r = sum_out_g = sum_out_b = sum_out_a =
							0;

			src_ptr = src + w4 * y; // start of line (0,y)

			for (i = 0; i <= radius; i++)
			{
				stack_ptr = &stack[4 * i];
				stack_ptr[0] = src_ptr[0];
				stack_ptr[1] = src_ptr[1];
				stack_ptr[2] = src_ptr[2];
				stack_ptr[3] = src_ptr[3];
				sum_r += src_ptr[0] * (i + 1);
				sum_g += src_ptr[1] * (i + 1);
				sum_b += src_ptr[2] * (i + 1);
				sum_a += src_ptr[3] * (i + 1);
				sum_out_r += src_ptr[0];
				sum_out_g += src_ptr[1];
				sum_out_b += src_ptr[2];
				sum_out_a += src_ptr[3];
			}

			for (i = 1; i <= radius; i++)
			{
				if (i <= wm)
					src_ptr += 4;
				stack_ptr = &stack[4 * (i + radius)];
				stack_ptr[0] = src_ptr[0];
				stack_ptr[1] = src_ptr[1];
				stack_ptr[2] = src_ptr[2];
				stack_ptr[3] = src_ptr[3];
				sum_r += src_ptr[0] * (radius + 1 - i);
				sum_g += src_ptr[1] * (radius + 1 - i);
				sum_b += src_ptr[2] * (radius + 1 - i);
				sum_a += src_ptr[3] * (radius + 1 - i);
				sum_in_r += src_ptr[0];
				sum_in_g += src_ptr[1];
				sum_in_b += src_ptr[2];
				sum_in_a += src_ptr[3];
			}

			sp = radius;
			xp = radius;
			if (xp > wm)
				xp = wm;
			src_ptr = src + 4 * (xp + y * w); //   img.pix_ptr(xp, y);
			dst_ptr = src + y * w4; // img.pix_ptr(0, y);
			for (x = 0; x < w; x++)
			{
				dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
				dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
				dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
				dst_ptr[3] = (sum_a * mul_sum) >> shr_sum;
				dst_ptr += 4;

				sum_r -= sum_out_r;
				sum_g -= sum_out_g;
				sum_b -= sum_out_b;
				sum_a -= sum_out_a;

				stack_start = sp + div - radius;
				if (stack_start >= div)
					stack_start -= div;
				stack_ptr = &stack[4 * stack_start];

				sum_out_r -= stack_ptr[0];
				sum_out_g -= stack_ptr[1];
				sum_out_b -= stack_ptr[2];
				sum_out_a -= stack_ptr[3];

				if (xp < wm)
				{
					src_ptr += 4;
					++xp;
				}

				stack_ptr[0] = src_ptr[0];
				stack_ptr[1] = src_ptr[1];
				stack_ptr[2] = src_ptr[2];
				stack_ptr[3] = src_ptr[3];

				sum_in_r += src_ptr[0];
				sum_in_g += src_ptr[1];
				sum_in_b += src_ptr[2];
				sum_in_a += src_ptr[3];
				sum_r += sum_in_r;
				sum_g += sum_in_g;
				sum_b += sum_in_b;
				sum_a += sum_in_a;

				++sp;
				if (sp >= div)
					sp = 0;
				stack_ptr = &stack[sp * 4];

				sum_out_r += stack_ptr[0];
				sum_out_g += stack_ptr[1];
				sum_out_b += stack_ptr[2];
				sum_out_a += stack_ptr[3];
				sum_in_r -= stack_ptr[0];
				sum_in_g -= stack_ptr[1];
				sum_in_b -= stack_ptr[2];
				sum_in_a -= stack_ptr[3];

			}

		}
	}

	// step 2
	if (step == 2)
	{
		unsigned int minX = core * w / cores;
		unsigned int maxX = (core + 1) * w / cores;

		for (x = minX; x < maxX; x++)
		{
			sum_r = sum_g = sum_b = sum_a = sum_in_r = sum_in_g = sum_in_b =
					sum_in_a = sum_out_r = sum_out_g = sum_out_b = sum_out_a =
							0;

			src_ptr = src + 4 * x; // x,0
			for (i = 0; i <= radius; i++)
			{
				stack_ptr = &stack[i * 4];
				stack_ptr[0] = src_ptr[0];
				stack_ptr[1] = src_ptr[1];
				stack_ptr[2] = src_ptr[2];
				stack_ptr[3] = src_ptr[3];
				sum_r += src_ptr[0] * (i + 1);
				sum_g += src_ptr[1] * (i + 1);
				sum_b += src_ptr[2] * (i + 1);
				sum_a += src_ptr[3] * (i + 1);
				sum_out_r += src_ptr[0];
				sum_out_g += src_ptr[1];
				sum_out_b += src_ptr[2];
				sum_out_a += src_ptr[3];
			}
			for (i = 1; i <= radius; i++)
			{
				if (i <= hm)
					src_ptr += w4; // +stride

				stack_ptr = &stack[4 * (i + radius)];
				stack_ptr[0] = src_ptr[0];
				stack_ptr[1] = src_ptr[1];
				stack_ptr[2] = src_ptr[2];
				stack_ptr[3] = src_ptr[3];
				sum_r += src_ptr[0] * (radius + 1 - i);
				sum_g += src_ptr[1] * (radius + 1 - i);
				sum_b += src_ptr[2] * (radius + 1 - i);
				sum_a += src_ptr[3] * (radius + 1 - i);
				sum_in_r += src_ptr[0];
				sum_in_g += src_ptr[1];
				sum_in_b += src_ptr[2];
				sum_in_a += src_ptr[3];
			}

			sp = radius;
			yp = radius;
			if (yp > hm)
				yp = hm;
			src_ptr = src + 4 * (x + yp * w); // img.pix_ptr(x, yp);
			dst_ptr = src + 4 * x; 			  // img.pix_ptr(x, 0);
			for (y = 0; y < h; y++)
			{
				dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
				dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
				dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
				dst_ptr[3] = (sum_a * mul_sum) >> shr_sum;
				dst_ptr += w4;

				sum_r -= sum_out_r;
				sum_g -= sum_out_g;
				sum_b -= sum_out_b;
				sum_a -= sum_out_a;

				stack_start = sp + div - radius;
				if (stack_start >= div)
					stack_start -= div;
				stack_ptr = &stack[4 * stack_start];

				sum_out_r -= stack_ptr[0];
				sum_out_g -= stack_ptr[1];
				sum_out_b -= stack_ptr[2];
				sum_out_a -= stack_ptr[3];

				if (yp < hm)
				{
					src_ptr += w4; // stride
					++yp;
				}

				stack_ptr[0] = src_ptr[0];
				stack_ptr[1] = src_ptr[1];
				stack_ptr[2] = src_ptr[2];
				stack_ptr[3] = src_ptr[3];

				sum_in_r += src_ptr[0];
				sum_in_g += src_ptr[1];
				sum_in_b += src_ptr[2];
				sum_in_a += src_ptr[3];
				sum_r += sum_in_r;
				sum_g += sum_in_g;
				sum_b += sum_in_b;
				sum_a += sum_in_a;

				++sp;
				if (sp >= div)
					sp = 0;
				stack_ptr = &stack[sp * 4];

				sum_out_r += stack_ptr[0];
				sum_out_g += stack_ptr[1];
				sum_out_b += stack_ptr[2];
				sum_out_a += stack_ptr[3];
				sum_in_r -= stack_ptr[0];
				sum_in_g -= stack_ptr[1];
				sum_in_b -= stack_ptr[2];
				sum_in_a -= stack_ptr[3];
			}
		}
	}
}

/// Stackblur algorithm by Mario Klingemann
/// Details here:
/// http://www.quasimondo.com/StackBlurForCanvas/StackBlurDemo.html
/// C++ implemenation base from:
/// https://gist.github.com/benjamin9999/3809142
/// http://www.antigrain.com/__code/include/agg_blur.h.html
/// This version works only with RGBA color
static void _stackblur(unsigned char* src,				///< input image data
		unsigned int w,					///< image width
		unsigned int h,					///< image height
		unsigned int radius,	///< blur intensity (should be in 2..254 range)
		gboolean blur_alpha)
{
	unsigned int div;
	unsigned char* stack;

	if (radius > 254)
		return;
	if (radius < 2)
		return;

	div = (radius * 2) + 1;
	stack = (unsigned char *) malloc(sizeof(unsigned char) * (div * 4));

	if (blur_alpha)
	{
		_stackblurJob_Alpha(src, w, h, radius, 1, 0, 1, stack);
		_stackblurJob_Alpha(src, w, h, radius, 1, 0, 2, stack);
	}
	else
	{
		_stackblurJob(src, w, h, radius, 1, 0, 1, stack);
		_stackblurJob(src, w, h, radius, 1, 0, 2, stack);
	}
	free(stack);
}

/* ------------------------------------------------------------------------- */

EXPORT_API TTransform::TTransform(TImage *img) :
		x_angle(0.0), y_angle(0.0), z_angle(0.0), x_scale(1.0), y_scale(1.0)
{
	this->map = evas_map_new(4);
	this->target = img;

	this->reset();
}

EXPORT_API TTransform::~TTransform()
{
	evas_map_free(this->map);
	this->map = NULL;
}

EXPORT_API Evas_Map* TTransform::getMap()
{
	return this->map;
}

EXPORT_API TTransform& TTransform::reset()
{
	evas_map_free(this->map);
	this->map = evas_map_new(4);

	show_rect.x = this->target->rect_.x;
	show_rect.y = this->target->rect_.y;
	show_rect.w = this->target->rect_.w;
	show_rect.h = this->target->rect_.h;

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.w = this->target->real_size.w;
	src_rect.h = this->target->real_size.h;

	x_angle = 0.0;
	y_angle = 0.0;
	z_angle = 0.0;

	x_scale = 1.0;
	y_scale = 1.0;

	evas_map_util_points_populate_from_geometry(this->map, show_rect.x,
			show_rect.y, show_rect.w, show_rect.h, 0);

#if 0
	dbg("view (%d,%d)~(%d,%d), real-src (%d,%d) ~ (%d, %d)", show_rect.x,
			show_rect.y, show_rect.w, show_rect.h, src_rect.x, src_rect.y,
			src_rect.w, src_rect.h)
#endif

	evas_map_point_image_uv_set(this->map, 0, src_rect.x, src_rect.y);
	evas_map_point_image_uv_set(this->map, 1, src_rect.w + src_rect.x,
			src_rect.y);
	evas_map_point_image_uv_set(this->map, 2, src_rect.w + src_rect.x,
			src_rect.h + src_rect.y);
	evas_map_point_image_uv_set(this->map, 3, src_rect.x,
			src_rect.h + src_rect.y);

	if (this->target->cursor_object)
	{
		evas_object_map_set(this->target->cursor_object, NULL);
		evas_object_map_enable_set(this->target->cursor_object, EINA_FALSE);
	}

	return *this;
}

EXPORT_API TTransform& TTransform::move(int x, int y, bool relative)
{
	if (relative)
	{
		show_rect.x += x;
		show_rect.y += y;
	}
	else
	{
		show_rect.x = x;
		show_rect.y = y;
	}

	evas_map_point_coord_set(this->map, 0, show_rect.x, show_rect.y, 0);
	evas_map_point_coord_set(this->map, 1, show_rect.w + show_rect.x,
			show_rect.y, 0);
	evas_map_point_coord_set(this->map, 2, show_rect.w + show_rect.x,
			show_rect.h + show_rect.y, 0);
	evas_map_point_coord_set(this->map, 3, show_rect.x,
			show_rect.h + show_rect.y, 0);

	return *this;
}

EXPORT_API TTransform& TTransform::moveX(int x, bool relative)
{
	if (relative)
	{
		show_rect.x += x;
	}
	else
	{
		show_rect.x = x;
	}

	evas_map_point_coord_set(this->map, 0, show_rect.x, show_rect.y, 0);
	evas_map_point_coord_set(this->map, 1, show_rect.w + show_rect.x,
			show_rect.y, 0);
	evas_map_point_coord_set(this->map, 2, show_rect.w + show_rect.x,
			show_rect.h + show_rect.y, 0);
	evas_map_point_coord_set(this->map, 3, show_rect.x,
			show_rect.h + show_rect.y, 0);

	return *this;
}

EXPORT_API TTransform& TTransform::moveY(int y, bool relative)
{
	if (relative)
	{
		show_rect.y += y;
	}
	else
	{
		show_rect.y = y;
	}

	evas_map_point_coord_set(this->map, 0, show_rect.x, show_rect.y, 0);
	evas_map_point_coord_set(this->map, 1, show_rect.w + show_rect.x,
			show_rect.y, 0);
	evas_map_point_coord_set(this->map, 2, show_rect.w + show_rect.x,
			show_rect.h + show_rect.y, 0);
	evas_map_point_coord_set(this->map, 3, show_rect.x,
			show_rect.h + show_rect.y, 0);

	return *this;
}

EXPORT_API TTransform& TTransform::moveToAngle(double angle, int r, int cx,
		int cy, bool center)
{
	double rad;
	double x, y;

	rad = (angle - 90.0) * (M_PI / 180);

	x = cx + (double) r * cos(rad);
	y = cy + (double) r * sin(rad);

	if (center)
	{
		x -= this->show_rect.w / 2;
		y -= this->show_rect.h / 2;
	}

	return move(x, y, false);
}

EXPORT_API TTransform& TTransform::resize(int w, int h, bool relative)
{
	if (relative)
	{
		show_rect.w += w;
		show_rect.h += h;
	}
	else
	{
		show_rect.w = w;
		show_rect.h = h;
	}

	evas_map_point_coord_set(this->map, 0, show_rect.x, show_rect.y, 0);
	evas_map_point_coord_set(this->map, 1, show_rect.w + show_rect.x,
			show_rect.y, 0);
	evas_map_point_coord_set(this->map, 2, show_rect.w + show_rect.x,
			show_rect.h + show_rect.y, 0);
	evas_map_point_coord_set(this->map, 3, show_rect.x,
			show_rect.h + show_rect.y, 0);

	return *this;
}

EXPORT_API TTransform& TTransform::scale(double ratio)
{
	return scale(ratio, ratio, show_rect.x + show_rect.w / 2,
			show_rect.y + show_rect.h / 2);
}

EXPORT_API TTransform& TTransform::scale(double ratio, int cx, int cy)
{
	return scale(ratio, ratio, cx, cy);
}

EXPORT_API TTransform& TTransform::scale(double x_ratio, double y_ratio)
{
	return scale(x_ratio, y_ratio, show_rect.x + show_rect.w / 2,
			show_rect.y + show_rect.h / 2);
}

EXPORT_API TTransform& TTransform::scale(double x_ratio, double y_ratio, int cx,
		int cy)
{
	x_scale = x_ratio;
	y_scale = y_ratio;

	if (target->debug_enable)
	{
		dbg("x_scale:%f, y_scale:%f // this=%p", x_scale, y_scale, this);
	}

	evas_map_util_zoom(this->map, x_ratio, y_ratio, cx, cy);

	return *this;
}

EXPORT_API TTransform& TTransform::selectSourcePos(int x, int y)
{
	src_rect.x = x;
	src_rect.y = y;

	evas_map_point_image_uv_set(this->map, 0, src_rect.x, src_rect.y);
	evas_map_point_image_uv_set(this->map, 1, src_rect.w + src_rect.x,
			src_rect.y);
	evas_map_point_image_uv_set(this->map, 2, src_rect.w + src_rect.x,
			src_rect.h + src_rect.y);
	evas_map_point_image_uv_set(this->map, 3, src_rect.x,
			src_rect.h + src_rect.y);

	return *this;
}

EXPORT_API TTransform& TTransform::selectSourceSize(int w, int h)
{
	src_rect.w = w;
	src_rect.h = h;

	evas_map_point_image_uv_set(this->map, 0, src_rect.x, src_rect.y);
	evas_map_point_image_uv_set(this->map, 1, src_rect.w + src_rect.x,
			src_rect.y);
	evas_map_point_image_uv_set(this->map, 2, src_rect.w + src_rect.x,
			src_rect.h + src_rect.y);
	evas_map_point_image_uv_set(this->map, 3, src_rect.x,
			src_rect.h + src_rect.y);

	return *this;
}

EXPORT_API TTransform& TTransform::rotate(double angle, bool relative)
{
	return rotate(angle, show_rect.x + show_rect.w / 2,
			show_rect.y + show_rect.h / 2, relative);
}

EXPORT_API TTransform& TTransform::rotate(double angle, TObject *base,
		bool relative)
{
	if (!relative)
	{
		this->move(0, 0, true);
	}

	x_angle = 0.0;
	y_angle = 0.0;
	z_angle = 0.0;

	evas_map_util_rotate(this->map, angle, base->center_x(), base->center_y());

	return *this;
}

EXPORT_API TTransform& TTransform::rotate(double angle, int cx, int cy,
		bool relative)
{
	if (!relative)
	{
		this->move(0, 0, true);
	}

	x_angle = 0.0;
	y_angle = 0.0;
	z_angle = 0.0;

	evas_map_util_rotate(this->map, angle, cx, cy);

	return *this;
}

EXPORT_API TTransform& TTransform::rotateX(double angle, bool relative)
{
	return rotateX(angle, show_rect.x + show_rect.w / 2,
			show_rect.y + show_rect.h / 2, 0, relative);
}

EXPORT_API TTransform& TTransform::rotateX(double angle, int cx, int cy, int cz,
		bool relative)
{
	if (!relative)
	{
		this->move(0, 0, true);
		this->x_angle = angle;
	}
	else
	{
		this->x_angle += angle;
	}

	evas_map_util_3d_rotate(this->map, this->x_angle, this->y_angle,
			this->z_angle, cx, cy, cz);

	return *this;
}

EXPORT_API TTransform& TTransform::rotateY(double angle, bool relative)
{
	return rotateY(angle, show_rect.x + show_rect.w / 2,
			show_rect.y + show_rect.h / 2, 0, relative);
}

EXPORT_API TTransform& TTransform::rotateY(double angle, int cx, int cy, int cz,
		bool relative)
{
	if (!relative)
	{
		this->move(0, 0, true);
		this->y_angle = angle;
	}
	else
	{
		this->y_angle += angle;
	}

	evas_map_util_3d_rotate(this->map, this->x_angle, this->y_angle,
			this->z_angle, cx, cy, cz);

	return *this;
}

EXPORT_API void TTransform::apply()
{
//	dbg("apply: %p", *(this->target->cursor));
	if (this->target->cursor_object)
	{
		evas_object_map_set(this->target->cursor_object, this->map);
		evas_object_map_enable_set(this->target->cursor_object, EINA_TRUE);
	}
}

/* ---------------------------------------------------------- */

static Evas_Object *_init(Evas *e)
{
	Evas_Object *src;
	src = evas_object_image_filled_add(e);
	evas_object_image_alpha_set(src, EINA_TRUE);
	evas_object_move(src, 0, 0);

	return src;
}

EXPORT_API TImage::TImage(Evas_Object *parent) :
		TObject(parent, "TImage"), stride(0), map(0), rawdata(0), cursor_object(
				0), cur_frame(0), max_frame(0), animation_start_frame(0), animation_timer(
				0), _stop_request(false), _repeat(false), prev_blur(0)
{
	memset(&real_size, 0, sizeof(real_size));
}

EXPORT_API TImage::TImage(Evas_Object *parent, const char *path) :
		TObject(parent, "TImage"), stride(0), map(0), rawdata(0), cursor_object(
				0), cur_frame(0), animation_start_frame(0), animation_timer(0), _stop_request(
				false), _repeat(false), prev_blur(0)
{
	Evas_Object *src;
	Evas_Load_Error load_err;

	src = _init(evas_object_evas_get(parent));

	evas_object_image_file_set(src, path, NULL);
	load_err = evas_object_image_load_error_get(src);
	if (load_err != EVAS_LOAD_ERROR_NONE)
	{
		err("image('%s') load failed. ('%s')", path,
				evas_load_error_str(load_err));
		evas_object_del(src);
		return;
	}

	evas_object_image_size_get(src, &(this->real_size.w), &(this->real_size.h));
	this->rect_.w = this->real_size.w;
	this->rect_.h = this->real_size.h;
	evas_object_resize(src, this->rect_.w, this->rect_.h);

	this->stride = evas_object_image_stride_get(src);
	this->max_frame = evas_object_image_animated_frame_count_get(src);
	if (this->max_frame < 0)
		this->max_frame = 0;

	items.push_back(src);
	cursor = items.begin();
	cursor_object = *cursor;
}

EXPORT_API TImage::TImage(Evas_Object * parent, Evas_Object * proxy_src,
		bool is_proxy) :
		TObject(parent, "TImage"), stride(0), map(0), rawdata(0), cursor_object(
				0), cur_frame(0), max_frame(0), animation_start_frame(0), animation_timer(
				0), _stop_request(false), _repeat(false), prev_blur(0)
{
	Evas_Object *src;

	if (is_proxy)
		src = _init(evas_object_evas_get(parent));
	else
		src = proxy_src;

	evas_object_geometry_get(src, &(this->rect_.x), &(this->rect_.y),
			&(this->rect_.w), &(this->rect_.h));

	if (!g_strcmp0(evas_object_type_get(proxy_src), "image"))
	{
		evas_object_image_size_get(proxy_src, &(this->real_size.w),
				&(this->real_size.h));
		if (this->real_size.w == 0 && this->real_size.h == 0)
		{
			this->real_size.w = this->rect_.w;
			this->real_size.h = this->rect_.h;
		}
		else if (this->rect_.w == 0 && this->rect_.h == 0)
		{
			this->rect_.w = this->real_size.w;
			this->rect_.h = this->real_size.h;
		}
		this->stride = evas_object_image_stride_get(src);
	}
	else
	{
		this->real_size.w = this->rect_.w;
		this->real_size.h = this->rect_.h;
	}

	if (is_proxy)
		evas_object_image_source_set(src, proxy_src);

	evas_object_resize(src, this->real_size.w, this->real_size.h);

	items.push_back(src);
	cursor = items.begin();
	cursor_object = *cursor;
}

EXPORT_API TImage::TImage(Evas_Object *parent, void *data, int size,
		char *format) :
		TObject(parent, "TImage"), stride(0), map(0), rawdata(0), cursor_object(
				0), cur_frame(0), animation_start_frame(0), animation_timer(0), _stop_request(
				false), _repeat(false), prev_blur(0)
{
	Evas_Object *src;

	src = _init(evas_object_evas_get(parent));

	evas_object_image_memfile_set(src, data, size, format,
	NULL);
	evas_object_image_size_get(src, &(this->real_size.w), &(this->real_size.h));
	this->rect_.w = this->real_size.w;
	this->rect_.h = this->real_size.h;
	evas_object_resize(src, this->rect_.w, this->rect_.h);

	this->stride = evas_object_image_stride_get(src);
	this->max_frame = evas_object_image_animated_frame_count_get(src);
	if (this->max_frame < 0)
		this->max_frame = 0;

	items.push_back(src);
	cursor = items.begin();
	cursor_object = *cursor;
}

EXPORT_API TImage::TImage(Evas_Object *parent, int width, int height) :
		TObject(parent, "TImage"), map(0), rawdata(0), cursor_object(0), cur_frame(
				0), max_frame(0), animation_start_frame(0), animation_timer(0), _stop_request(
				false), _repeat(false), prev_blur(0)
{
	Evas_Object *src;

	src = _init(evas_object_evas_get(parent));
	evas_object_image_size_set(src, width, height);
	evas_object_resize(src, width, height);
	evas_object_move(src, 0, 0);
	//evas_object_image_content_hint_set(src, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);

	real_size.w = width;
	real_size.h = height;
	rect_.w = this->real_size.w;
	rect_.h = this->real_size.h;

	this->stride = evas_object_image_stride_get(src);

	items.push_back(src);
	cursor = items.begin();
	cursor_object = *cursor;
}

EXPORT_API TImage::TImage(Evas_Object *parent, TImage *orig) :
		TObject(parent, "TImage"), map(0), rawdata(0), cursor_object(0), cur_frame(
				0), max_frame(0), animation_start_frame(0), animation_timer(0), _stop_request(
				false), _repeat(false), prev_blur(0)
{
	Evas_Object *src;

	src = _init(evas_object_evas_get(parent));
	evas_object_image_size_set(src, orig->real_size.w, orig->real_size.h);
	evas_object_resize(src, orig->rect_.w, orig->rect_.h);
	evas_object_move(src, 0, 0);

	real_size.w = orig->real_size.w;
	real_size.h = orig->real_size.h;
	rect_.w = orig->rect_.w;
	rect_.h = orig->rect_.h;

	evas_object_image_data_copy_set(src,
			evas_object_image_data_get(orig->getFrameSource(), EINA_FALSE));

	this->stride = evas_object_image_stride_get(src);

	items.push_back(src);
	cursor = items.begin();
	cursor_object = *cursor;
}

EXPORT_API TImage::~TImage()
{
	if (animation_timer)
		ecore_timer_del(animation_timer);

	if (map)
		delete map;

	if (rawdata)
		free(rawdata);

	cursor = items.begin();
//	dbg("items count: %ld", items.size());
	for (; cursor != items.end(); ++cursor)
	{
//		dbg("delete item: %p", *cursor);
		evas_object_del(*cursor);
	}

	items.erase(items.begin(), items.end());
}

EXPORT_API TImage& TImage::replace(const char *path)
{
	Evas_Object *src;
	Evas_Load_Error load_err;

	src = _init(evas_object_evas_get(parent_eo_));

	evas_object_image_file_set(src, path, NULL);
	load_err = evas_object_image_load_error_get(src);
	if (load_err != EVAS_LOAD_ERROR_NONE)
	{
		err("image('%s') load failed. ('%s')", path,
				evas_load_error_str(load_err));
		evas_object_del(src);
		return *this;
	}

	this->stopAnimation();

	if (this->rawdata)
	{
		free(this->rawdata);
		this->rawdata = NULL;
	}

	evas_object_del(cursor_object);
	items.erase(cursor);

	evas_object_image_size_get(src, &(this->real_size.w), &(this->real_size.h));
	evas_object_resize(src, this->rect_.w, this->rect_.h);
	evas_object_move(src, this->rect_.x, this->rect_.y);
	this->stride = evas_object_image_stride_get(src);

	this->max_frame = evas_object_image_animated_frame_count_get(src);
	if (this->max_frame < 0)
		this->max_frame = 0;

	this->cur_frame = 0;

	if (this->map)
	{
		this->map->apply();
	}

	if (this->visibility_)
		evas_object_show(src);
	else
		evas_object_hide(src);

	items.push_back(src);
	cursor = items.begin();
	cursor_object = *cursor;

	return *this;
}

EXPORT_API TImage& TImage::replace(const char *path, int frame)
{
	Evas_Object *src;
	Evas_Load_Error load_err;

	if (frame < 0 || frame >= this->max_frame)
		return *this;

	src = _init(evas_object_evas_get(parent_eo_));

	evas_object_image_file_set(src, path, NULL);
	load_err = evas_object_image_load_error_get(src);
	if (load_err != EVAS_LOAD_ERROR_NONE)
	{
		err("image('%s') load failed. ('%s')", path,
				evas_load_error_str(load_err));
		evas_object_del(src);
		return *this;
	}

	evas_object_del(items.at(frame));

	evas_object_image_size_get(src, &(this->real_size.w), &(this->real_size.h));
	evas_object_resize(src, this->rect_.w, this->rect_.h);
	evas_object_move(src, this->rect_.x, this->rect_.y);
	evas_object_layer_set(src, layer_);
	evas_object_hide(src);

	items.at(frame) = src;
	cursor = items.begin();
	cursor_object = *cursor;

	return *this;
}

EXPORT_API Evas_Object *TImage::getFrameSource()
{
	if (!cursor_object)
		return NULL;

	return cursor_object;
}

EXPORT_API TImage& TImage::append(const char *path)
{
	Evas_Object *src;
	Evas_Load_Error load_err;

	src = _init(evas_object_evas_get(parent_eo_));

	evas_object_image_file_set(src, path, NULL);
	load_err = evas_object_image_load_error_get(src);
	if (load_err != EVAS_LOAD_ERROR_NONE)
	{
		err("image('%s') load failed. ('%s')", path,
				evas_load_error_str(load_err));
		evas_object_del(src);
		return *this;
	}

	evas_object_image_size_get(src, &(this->real_size.w), &(this->real_size.h));

	if (items.size() == 0)
	{
		this->rect_.x = 0;
		this->rect_.y = 0;
		this->rect_.w = this->real_size.w;
		this->rect_.h = this->real_size.h;

		this->stride = evas_object_image_stride_get(src);
		this->max_frame = evas_object_image_animated_frame_count_get(src);
		if (this->max_frame < 0)
			this->max_frame = 0;
	}

	evas_object_resize(src, this->rect_.w, this->rect_.h);
	evas_object_move(src, this->rect_.x, this->rect_.y);
	evas_object_layer_set(src, layer_);
	evas_object_hide(src);

	this->max_frame++;

	items.push_back(src);
	cursor = items.begin();
	cursor_object = *cursor;

	return *this;
}

EXPORT_API void TImage::show(bool smooth)
{
	if (isVisible())
		return;

	TObject::show(smooth);

	if (!cursor_object)
		return;

	evas_object_show(cursor_object);
}

EXPORT_API void TImage::hide(bool smooth)
{
	if (!isVisible())
		return;

	TObject::hide(smooth);

	if (!cursor_object)
		return;

	evas_object_hide(cursor_object);
}

void TImage::onMove(int x, int y)
{
	TObject::onMove(x, y);
	//dbg("-%d- TImage (x=%d, y=%d)", getId(), x, y);

	if (!cursor_object)
		return;

	evas_object_move(cursor_object, this->rect_.x, this->rect_.y);

	if (this->map)
		this->map->reset();
}

void TImage::onResize(int w, int h)
{
	TObject::onResize(w, h);
	//dbg("-%d- TImage (w=%d, h=%d)", getId(), w, h);

	if (!cursor_object)
		return;

	evas_object_resize(cursor_object, this->rect_.w, this->rect_.h);

	if (this->map)
		this->map->reset();
}

void TImage::onLayer(int layer)
{
	TObject::onLayer(layer);

	if (!cursor_object)
		return;

	evas_object_layer_set(cursor_object, layer);
}

EXPORT_API TImage& TImage::moveToAngle(double angle, int r, bool center)
{
	Evas_Coord w, h;

	if (!cursor_object)
		return *this;

	evas_output_viewport_get(evas_object_evas_get(cursor_object), NULL, NULL,
			&w, &h);

	TObject::moveToAngle(angle, r, w / 2, h / 2, center);

	return *this;
}

EXPORT_API int TImage::getRealWidth()
{
	return this->real_size.w;
}

EXPORT_API int TImage::getRealHeight()
{
	return this->real_size.h;
}

EXPORT_API TImage& TImage::copy(Evas_Object *src)
{
	if (!src)
		return *this;

	dbg("original data: %p",
			evas_object_image_data_get(cursor_object, EINA_FALSE));

	dbg("target   data: %p", evas_object_image_data_get(src, EINA_FALSE));

	evas_object_image_data_copy_set(cursor_object,
			evas_object_image_data_get(src, EINA_FALSE));
	evas_object_image_data_update_add(cursor_object, 0, 0, real_size.w,
			real_size.h);

	if (this->rawdata)
	{
		free(this->rawdata);
		this->rawdata = NULL;
	}

	return *this;
}

EXPORT_API TImage& TImage::setOpacity(int alpha, bool update_src_data)
{
	unsigned char *orig_pos, *updated, *cur;
	double v;
	int size;

	if (!cursor_object)
		return *this;

	if (update_src_data == false)
	{
		//dbg("type='%s'", evas_object_type_get(cursor_object));
		if (!g_strcmp0(evas_object_type_get(cursor_object), "rectangle"))
		{
			dbg("not support");
		}
		else
			evas_object_color_set(cursor_object, alpha, alpha, alpha, alpha);
		return *this;
	}

	updated = (unsigned char *) evas_object_image_data_get(cursor_object,
	EINA_TRUE);
	size = this->stride * this->real_size.h;

	if (!this->rawdata)
	{
		this->rawdata = (unsigned char *) malloc(size);
		memcpy(this->rawdata, updated, size);
	}

	orig_pos = this->rawdata;

	if (alpha < 0)
		alpha = 0;
	if (alpha > 255)
		alpha = 255;

	v = ((double) alpha / 255);

	cur = updated;
	for (int y = 0; y < size; y++, orig_pos++, cur++)
	{
		*cur = *(orig_pos) * v;
	}

	evas_object_image_data_set(cursor_object, updated);
	evas_object_image_data_update_add(cursor_object, 0, 0, this->real_size.w,
			this->real_size.h);

	return *this;
}

EXPORT_API TImage& TImage::fillTo(unsigned int rgba, bool circle)
{
	return fillTo((rgba & 0xFF000000) >> 24, (rgba & 0x00FF0000) >> 16,
			(rgba & 0x0000FF00) >> 8, (rgba & 0x000000FF), circle);
}

EXPORT_API TImage& TImage::fillTo(int r, int g, int b, int a, bool circle)
{
	cairo_surface_t *sf;
	cairo_t *cr;
	unsigned char *updated;

	if (!cursor_object)
		return *this;

	updated = (unsigned char *) evas_object_image_data_get(cursor_object,
	EINA_TRUE);

	sf = cairo_image_surface_create_for_data(updated, CAIRO_FORMAT_ARGB32,
			real_size.w, real_size.h,
			evas_object_image_stride_get(cursor_object));

	cr = cairo_create(sf);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_paint(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	if (circle)
	{
		cairo_arc(cr, real_size.w / 2, real_size.h / 2, real_size.w / 2, 0,
		M_PI * 2);
		cairo_clip(cr);
	}

	if ((r != 0 || g != 0 || b != 0) && a != 0)
	{
		cairo_set_source_rgba(cr, (double) r / 255.0, (double) g / 255.0,
				(double) b / 255.0, (double) a / 255.0);
		cairo_paint(cr);
	}

	cairo_destroy(cr);

	cairo_surface_destroy(sf);

	evas_object_image_data_set(cursor_object, updated);
	evas_object_image_data_update_add(cursor_object, 0, 0, real_size.w,
			real_size.h);

	return *this;
}

EXPORT_API TImage& TImage::setBlur(int radius, bool with_alpha)
{
	unsigned char *updated;
	int size;

	if (!cursor_object)
		return *this;

	if (radius < 0)
		return *this;

	if (radius == prev_blur)
		return *this;

	prev_blur = radius;

	updated = (unsigned char *) evas_object_image_data_get(cursor_object,
	EINA_TRUE);
	size = this->stride * this->real_size.h;

	if (!this->rawdata)
	{
		dbg("new copy: size=%d", size);
		this->rawdata = (unsigned char *) malloc(size);
		memcpy(this->rawdata, updated, size);
	}
	else
	{
		memcpy(updated, this->rawdata, size);
	}

	if (debug_enable)
	{
		dbg("radius: %d, (%d,%d), %d, %p, %p", radius, this->real_size.w,
				this->real_size.h, with_alpha, this, updated);
	}

	if (radius >= 2)
	{
		_stackblur(updated, this->real_size.w, this->real_size.h, radius,
				with_alpha);
	}

	evas_object_image_data_set(cursor_object, updated);
	evas_object_image_data_update_add(cursor_object, 0, 0, this->real_size.w,
			this->real_size.h);

	return *this;
}

EXPORT_API TTransform& TImage::getTransform()
{
	if (!this->map)
		this->map = new TTransform(this);

	return *(this->map);
}

EXPORT_API bool TImage::isAnimationSupport() const
{
	if (evas_object_image_animated_get(cursor_object))
		return true;

	if (items.size() > 1)
		return true;

	return false;
}

EXPORT_API bool TImage::isAnimationStarted() const
{
	if (this->animation_timer)
		return true;

	return false;
}

EXPORT_API int TImage::getFrameIndex()
{
	return this->cur_frame;
}

EXPORT_API int TImage::getFrameCount()
{
	return this->max_frame;
}

EXPORT_API double TImage::getFrameDuration()
{
	double duration;

	duration = evas_object_image_animated_frame_duration_get(cursor_object,
			this->cur_frame, 0);

	//dbg("duration: %f", duration);

	if (duration < 0.05)
		return 0.05;

	return duration;
}

EXPORT_API TImage& TImage::setFrame(int frame_index)
{
	if (frame_index < 0 || frame_index > this->max_frame)
		return *this;

	if (this->cur_frame == frame_index)
		return *this;

	this->cur_frame = frame_index;

	if (evas_object_image_animated_get(cursor_object))
	{
		evas_object_image_animated_frame_set(cursor_object, this->cur_frame);
	}
	else
	{
		evas_object_hide(cursor_object);

		cursor = items.begin() + frame_index;
		cursor_object = *cursor;
#if 0
		dbg("cursor: %p, index: %d, size: %d", *cursor, frame_index,
				items.size());
#endif

		evas_object_resize(cursor_object, this->rect_.w, this->rect_.h);
		evas_object_move(cursor_object, this->rect_.x, this->rect_.y);
		evas_object_layer_set(cursor_object, layer_);

		if (this->map)
			this->map->apply();

		if (this->visibility_)
			evas_object_show(cursor_object);
		else
			evas_object_hide(cursor_object);

	}

	return *this;
}

EXPORT_API TImage& TImage::setStartFrame(int frame_index)
{
	if (frame_index < 0 || frame_index > this->max_frame)
		return *this;

	this->animation_start_frame = frame_index;

	return *this;
}

EXPORT_API TImage& TImage::nextFrame(bool rewind)
{
	int frame = this->cur_frame + 1;

	if (frame >= this->max_frame)
	{
		if (rewind)
			frame = 0;
		else
			frame = this->max_frame;
	}

	return setFrame(frame);
}

EXPORT_API TImage& TImage::nextAnimationFrame(bool rewind)
{
	int frame = this->cur_frame + 1;

	if (frame >= this->max_frame)
	{
		if (rewind)
			frame = this->animation_start_frame;
		else
			frame = this->max_frame;
	}

	return setFrame(frame);
}

Eina_Bool TImage::on_animation(void *user_data)
{
	TImage *img = (TImage *) user_data;

	img->nextAnimationFrame(img->_repeat);

	if (img->_stop_request && img->cur_frame == img->animation_start_frame)
	{
		dbg("delay stop");
		img->animation_timer = NULL;
		return EINA_FALSE;
	}

	if (!img->_repeat && img->cur_frame == img->max_frame)
	{
		dbg("finish");
		img->animation_timer = NULL;
		return EINA_FALSE;
	}

	img->animation_timer = ecore_timer_add(img->getFrameDuration(),
			on_animation, user_data);

	return EINA_FALSE;
}

EXPORT_API TImage& TImage::startAnimation(bool repeat_request)
{
	_stop_request = false;
	_repeat = repeat_request;

	if (this->animation_timer)
	{
		ecore_timer_del(this->animation_timer);
		this->animation_timer = NULL;
	}

	if (!(this->isAnimationSupport()))
		return *this;

	this->setFrame(this->animation_start_frame);

	this->animation_timer = ecore_timer_add(this->getFrameDuration(),
			on_animation, this);

	return *this;
}

EXPORT_API TImage& TImage::stopAnimation(Transition::position after_pos,
		bool rightnow)
{
//	dbg("stop");
	if (rightnow)
	{
		if (this->animation_timer)
			ecore_timer_del(this->animation_timer);
		this->animation_timer = NULL;
		if (after_pos == Transition::START)
			setFrame(this->animation_start_frame);
		else if (after_pos == Transition::END)
			setFrame(this->max_frame);
	}
	else
	{
		_stop_request = true;
	}

	return *this;
}

EXPORT_API TStateImage::TStateImage() :
		TObject(NULL, "TStateImage"), cur_state(STATE_DEFAULT)
{
	memset(items, 0, sizeof(items));
}

EXPORT_API TStateImage::~TStateImage()
{
	for (int i = 0; i < STATE_MAX; i++)
		if (items[i])
			delete items[i];
}

EXPORT_API void TStateImage::setImage(TImage *img, enum timage_state_types type)
{
	if (items[type])
		delete items[type];

	if (x() > 0 || y() > 0)
		img->moveTo(x(), y());
	else
	{
		rect_.x = img->x();
		rect_.y = img->y();
	}

	if (width() > 0 || height() > 0)
		img->resizeTo(width(), height());
	else
	{
		rect_.w = img->width();
		rect_.h = img->height();
	}

	if (getLayer() > 0)
		img->layerTo(getLayer());

	items[type] = img;
}

EXPORT_API TImage* TStateImage::getImage(enum timage_state_types type)
{
	return items[type];
}

EXPORT_API void TStateImage::setState(enum timage_state_types type)
{
	if (cur_state == type)
		return;

	if (!items[type])
		return;

	if (items[cur_state])
	{
		if (items[cur_state]->isAnimationStarted())
			items[cur_state]->stopAnimation();

		items[cur_state]->hide();
	}

	cur_state = type;

	if (isVisible())
		items[cur_state]->show();
}

EXPORT_API enum timage_state_types TStateImage::getState()
{
	return cur_state;
}

EXPORT_API TImage *TStateImage::getStateImage()
{
	return items[cur_state];
}

EXPORT_API void TStateImage::onMove(int x, int y)
{
	for (int i = 0; i < STATE_MAX; i++)
		if (items[i])
			items[i]->moveTo(x, y);
}

EXPORT_API void TStateImage::onResize(int w, int h)
{
	for (int i = 0; i < STATE_MAX; i++)
		if (items[i])
			items[i]->resizeTo(w, h);
}

EXPORT_API void TStateImage::onLayer(int layer)
{
	for (int i = 0; i < STATE_MAX; i++)
		if (items[i])
			items[i]->layerTo(layer);
}

EXPORT_API void TStateImage::onShow(bool smooth)
{
	if (!items[cur_state])
		return;

	items[cur_state]->show(smooth);
}

EXPORT_API void TStateImage::onHide(bool smooth)
{
	for (int i = 0; i < STATE_MAX; i++)
		if (items[i])
			items[i]->hide(smooth);
}
