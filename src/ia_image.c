/*********************************************************************/
/*                                                                   */
/* Copyright (C) 2005, Alexander Marinov, Nadezhda Zlateva           */
/*                                                                   */
/* Project:       ia                                                 */
/* Filename:      ia_image.c                                         */
/* Description:   Image management module                            */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#ifdef __GNUC__
#include <strings.h>
#else
#include <string.h>
#define strcasecmp _stricmp
#endif
#include <ia/ia_image.h>
#include <ia/ia_signal.h>
#include <ia/ia_line.h>
#include <ia/algo/ia_otsu.h>

#ifndef ABS
#define ABS(a) ((a)>=0 ? (a) : (-(a)))
#endif

/*********************************************************************/
/*                        Local prototypes                           */
/*********************************************************************/

static void                ia_image_destroy              (struct _ia_image_t*);
static void                ia_image_set_pixel_2          (struct _ia_image_t*, ia_uint16_t, ia_uint16_t, ia_bool_t);
static ia_bool_t           ia_image_get_pixel_2          (struct _ia_image_t*, ia_uint16_t, ia_uint16_t);
static void                ia_image_set_pixel_8          (struct _ia_image_t*, ia_uint16_t, ia_uint16_t, ia_uint8_t);
static ia_uint8_t          ia_image_get_pixel_8          (struct _ia_image_t*, ia_uint16_t, ia_uint16_t);
static void                ia_image_set_pixel_16         (struct _ia_image_t*, ia_uint16_t, ia_uint16_t, ia_uint16_t);
static ia_uint16_t         ia_image_get_pixel_16         (struct _ia_image_t*, ia_uint16_t, ia_uint16_t);
static void                ia_image_set_pixel_24         (struct _ia_image_t*, ia_uint16_t, ia_uint16_t, ia_uint32_t);
static ia_uint32_t         ia_image_get_pixel_24         (struct _ia_image_t*, ia_uint16_t, ia_uint16_t);
static void                ia_image_set_pixel_32         (struct _ia_image_t*, ia_uint16_t, ia_uint16_t, ia_uint32_t);
static ia_uint32_t         ia_image_get_pixel_32         (struct _ia_image_t*, ia_uint16_t, ia_uint16_t);
static void                ia_image_set_pixel            (struct _ia_image_t*, ia_uint16_t, ia_uint16_t, ia_uint32_t);
static ia_uint32_t         ia_image_get_pixel            (struct _ia_image_t*, ia_uint16_t, ia_uint16_t);
static void                ia_image_fill                 (struct _ia_image_t*, ia_uint32_t);
static struct _ia_image_t* ia_image_convert_rgb          (struct _ia_image_t*);
static struct _ia_image_t* ia_image_convert_gray         (struct _ia_image_t*, ia_format_t);
static void                ia_image_normalize_colors     (struct _ia_image_t*, ia_int32_t, ia_uint32_t, ia_int32_t, ia_uint32_t);
static void                ia_image_extract_hsv          (struct _ia_image_t*, ia_uint32_t, ia_uint32_t, ia_uint32_t, ia_uint32_t, ia_uint32_t, ia_uint32_t);
static void                ia_image_inverse              (struct _ia_image_t*);
static void                ia_image_mask                 (struct _ia_image_t*, struct _ia_image_t*, ia_mask_t);
static struct _ia_image_t* ia_image_substract            (struct _ia_image_t*, struct _ia_image_t*);
static void                ia_image_binarize_threshold   (struct _ia_image_t*, ia_int32_t);
static void                ia_image_binarize_threshold_2 (struct _ia_image_t*, ia_int32_t, ia_int32_t);
static int                 ia_image_binarize_otsu        (struct _ia_image_t*, ia_signal_p);
static int                 ia_image_binarize_otsu_2      (struct _ia_image_t*, ia_signal_p);
static void                ia_image_get_min_max          (struct _ia_image_t*, ia_int32_t*, ia_uint32_t*);
static void                ia_image_print                (struct _ia_image_t*);
static ia_signal_p         ia_image_histogram            (struct _ia_image_t*, ia_color_element_t);
static ia_signal_p         ia_image_line_to_signal       (struct _ia_image_t*, ia_int32_t, ia_int32_t, ia_int32_t, ia_int32_t);
static struct _ia_image_t* ia_image_copy                 (struct _ia_image_t*);
static void                ia_image_draw_line            (struct _ia_image_t*, ia_int32_t, ia_int32_t, ia_int32_t, ia_int32_t, ia_uint32_t);
static void                ia_image_save                 (struct _ia_image_t*, const ia_string_t);
static void                ia_image_save_img             (struct _ia_image_t*, const ia_string_t);
static ia_image_p          ia_image_load_img             (const ia_string_t image_name);

#ifdef HAVE_JPEGLIB
ia_image_p ia_image_load_jpeg(const ia_string_t src);
void ia_image_save_jpeg(ia_image_p, const ia_string_t src);
#endif
#ifdef HAVE_TIFFLIB
ia_image_p ia_image_load_tiff(const ia_string_t src);
void ia_image_save_tiff(ia_image_p, const ia_string_t src);
#endif
ia_image_p ia_image_load_gif(const ia_string_t src);

/*********************************************************************/
/*                        Implementation                             */
/*********************************************************************/

static void ia_rgb_to_hsv(ia_uint32_t color, ia_uint32_t* phue, ia_uint32_t* psat, ia_uint32_t* pval)
{
    float red = IA_RED(color) / 255.0f;
    float green = IA_GREEN(color) / 255.0f;
    float blue = IA_BLUE(color) / 255.0f;
    float mn = red, mx = red, hue, saturation, value, delta;
    int maxVal = 0;

    if (green > mx) 
	{ 
		mx = green; 
		maxVal = 1; 
	}

    if (blue > mx) 
	{ 
		mx = blue; 
		maxVal = 2; 
	}

    if (green < mn) 
	{
		mn = green;
	}

    if (blue < mn) 
	{
		mn = blue;
	}

    delta = mx - mn;

    value = mx;
    if (mx != 0)
	{
        saturation = delta / mx;
	}
    else 
	{
        saturation = 0;
        hue = 0;
    }

    if (saturation == 0.0f)
	{
		/* Arbritrary */
        hue = 0;
    } 
	else 
	{
        switch (maxVal)
        {
            case 0: hue = 0 + ( green - blue) / delta; break;    /* yel  < hue < mag  */
            case 1: hue = 2 + ( blue - red )  / delta; break;    /* cyan < hue < yel  */
            case 2: hue = 4 + ( red - green ) / delta; break;    /* mag  < hue < cyan */
            default: hue = 0;
        }
    }

    hue *= 60;
    if (hue < 0) 
	{
		hue += 360;
	}

	if (phue)
		*phue = (ia_uint32_t)hue;
	if (psat)
		*psat = (ia_uint32_t)(255.0f * saturation);
	if (pval)
		*pval = (ia_uint32_t)(255.0f * value);
}

static void ia_hsv_to_rgb(ia_uint32_t ahue, ia_uint32_t asat, ia_uint32_t aval, ia_uint32_t* pcolor)
{
	int i;
	float f, p, q, t, hTemp;
	float hue = (float)ahue;
	float saturation = asat / 255.0f;
	float value = aval / 255.0f;
	float red, green, blue;

	if (asat == 255)
	{ 
		/* totally unsaturated = grey */
		*pcolor = IA_RGB(aval, aval, aval);
		return;
	}

	hTemp = hue / 60.0f;
	i = (int) floor(hTemp);             /* which sector */
	f = hTemp - i;                      /* how far through sector */
	p = value * ( 1 - saturation );
	q = value * ( 1 - saturation * f );
	t = value * ( 1 - saturation * ( 1 - f ) );

	switch (i)
	{
		case 0:  red = value; green = t; blue = p; break;
		case 1:  red = q; green = value; blue = p; break;
		case 2:  red = p; green = value; blue = t; break;
		case 3:  red = p; green = q; blue = value; break;
		case 4:  red = t; green = p; blue = value; break;
		case 5:  red = value; green = p; blue = q; break;
		default: red = 0; green = 0; blue = 0;
	}

	*pcolor = IA_RGB((ia_uint8_t)red, (ia_uint8_t)green, (ia_uint8_t)blue);
}

static ia_image_p ia_image_load_img(const ia_string_t image_name)
{
	ia_image_p  img;
	ia_uint16_t w,h;
	ia_format_t format;
	ia_bool_t   is_gray;
	FILE* fp=fopen(image_name, "rb");
	if (!fp)
	{
		return 0;
	}
	fread(&w, 1, sizeof(ia_uint16_t), fp);
	fread(&h, 1, sizeof(ia_uint16_t), fp);
	fread(&format, 1, sizeof(ia_format_t), fp);
	fread(&is_gray, 1, sizeof(ia_bool_t), fp);
	img=ia_image_new(w, h, format, is_gray);
	fread(img->pixels.data, 1, img->pixels.size, fp);
	fclose(fp);
	return img;
}

static void ia_image_save_img(struct _ia_image_t* img, const ia_string_t image_name)
{
	FILE* fp=fopen(image_name, "wb");
	if (!fp)
	{
		return ;
	}
	fwrite(&img->width,   1, sizeof(ia_uint16_t), fp);
	fwrite(&img->height,  1, sizeof(ia_uint16_t), fp);
	fwrite(&img->format,  1, sizeof(ia_format_t), fp);
	fwrite(&img->is_gray, 1, sizeof(ia_bool_t),   fp);
	fwrite(img->pixels.data, 1, img->pixels.size, fp);
	fclose(fp);
}

ia_image_p ia_image_load(const ia_string_t image_name)
{
	ia_string_t ext=strrchr(image_name, '.');
	if (ext)
	{
		if (!strcasecmp(ext, ".img"))
			return ia_image_load_img(image_name);
#ifdef HAVE_JPEGLIB
		if (!strcasecmp(ext, ".jpg") || !strcasecmp(ext, ".jpeg"))
			return ia_image_load_jpeg(image_name);
#endif
#ifdef HAVE_TIFFLIB
		if (!strcasecmp(ext, ".tif") || !strcasecmp(ext, ".tiff"))
			return ia_image_load_tiff(image_name);
#endif
		if (!strcasecmp(ext, ".gif"))
			return ia_image_load_gif(image_name);
	}
	return 0;
}

void ia_image_save(struct _ia_image_t* self, const ia_string_t image_name)
{
	ia_string_t ext=strrchr(image_name, '.');
	if (ext)
	{
		if (!strcasecmp(ext, ".img"))
		{
			ia_image_save_img(self, image_name);
			return ;
		}
#ifdef HAVE_JPEGLIB
		if (!strcasecmp(ext, ".jpg") || !strcasecmp(ext, ".jpeg"))
		{
			ia_image_save_jpeg(self, image_name);
			return ;
		}
#endif
#ifdef HAVE_TIFFLIB
		if (!strcasecmp(ext, ".tif") || !strcasecmp(ext, ".tiff"))
		{
			ia_image_save_tiff(self, image_name);
			return ;
		}
#endif
	}
}

ia_image_p ia_image_from_data(ia_uint16_t width, ia_uint16_t height, ia_format_t format, ia_bool_t is_gray, void* data, ia_uint32_t size)
{
	ia_image_p  img               = (ia_image_p)malloc(sizeof(ia_image_t));
	img->width                    = width;
	img->height                   = height;
	img->format                   = format;
	img->is_gray                  = is_gray;
	img->is_user_data             = IA_TRUE;
	img->marker_x                 = 0;
	img->marker_y                 = 0;
	img->pixels.data              = data;
	img->pixels.size              = size;
	img->destroy                  = ia_image_destroy;
	img->set_pixel                = ia_image_set_pixel;
	img->get_pixel                = ia_image_get_pixel;
	img->fill                     = ia_image_fill;
	img->convert_rgb              = ia_image_convert_rgb;
	img->convert_gray             = ia_image_convert_gray;
	img->normalize_colors         = ia_image_normalize_colors;
	img->mask                     = ia_image_mask;
	img->extract_hsv              = ia_image_extract_hsv;
	img->inverse                  = ia_image_inverse;
	img->substract                = ia_image_substract;
	img->get_min_max              = ia_image_get_min_max;
	img->histogram                = ia_image_histogram;
	img->line_to_signal           = ia_image_line_to_signal;
	img->binarize_threshold       = ia_image_binarize_threshold;
	img->binarize_threshold_2     = ia_image_binarize_threshold_2;
	img->binarize_otsu            = ia_image_binarize_otsu;
	img->binarize_otsu_2          = ia_image_binarize_otsu_2;
	img->copy                     = ia_image_copy;
	img->draw_line                = ia_image_draw_line;
	img->save                     = ia_image_save;
	img->print                    = ia_image_print;

	return img;
}

ia_image_p ia_image_new(ia_uint16_t width, ia_uint16_t height, ia_format_t format, ia_bool_t is_gray)
{
	ia_image_p img;
	ia_uint32_t size = height * ((width * ia_format_size(format) + 7) >> 3);
	void* data = calloc(1, size);
	if (!data)
	{
		return NULL;
	}

	img = ia_image_from_data(width, height, format, is_gray, data, size);
	img->is_user_data = IA_FALSE;
	return img;
}

static void ia_image_destroy(struct _ia_image_t* self)
{
	if ((self->is_user_data == IA_FALSE) && (self->pixels.data != NULL))
	{
		free(self->pixels.data);
	}
	free(self);
}

static void ia_image_set_pixel_2(struct _ia_image_t* self, ia_uint16_t x, ia_uint16_t y, ia_bool_t value)
{
	ia_uint32_t ofs=y * ((self->width+7) >> 3) + (x >> 3);
	ia_uint32_t bit=1<<(x & 7);
	if (value) 
	{
		((ia_uint8_t*)self->pixels.data)[ofs]|=bit;
	}
	else
	{
		((ia_uint8_t*)self->pixels.data)[ofs]&=~bit;
	}
}

static ia_bool_t ia_image_get_pixel_2(struct _ia_image_t* self, ia_uint16_t x, ia_uint16_t y)
{
	ia_uint32_t ofs=y * ((self->width+7) >> 3) + (x >> 3);
	ia_uint32_t bit=1<<(x & 7);
	return ((((ia_uint8_t*)self->pixels.data)[ofs]&bit)?1:0);
}

static void ia_image_set_pixel_8(struct _ia_image_t* self, ia_uint16_t x, ia_uint16_t y, ia_uint8_t value)
{
	((ia_uint8_t*)self->pixels.data)[y*self->width + x]=value;
}

static ia_uint8_t ia_image_get_pixel_8(struct _ia_image_t* self, ia_uint16_t x, ia_uint16_t y)
{
	return ((ia_uint8_t*)self->pixels.data)[y*self->width + x];
}

static void ia_image_set_pixel_16(struct _ia_image_t* self, ia_uint16_t x, ia_uint16_t y, ia_uint16_t value)
{
	((ia_uint16_t*)self->pixels.data)[y*self->width + x]=value;
}

static ia_uint16_t ia_image_get_pixel_16(struct _ia_image_t* self, ia_uint16_t x, ia_uint16_t y)
{
	return ((ia_uint16_t*)self->pixels.data)[y*self->width + x];
}

static void ia_image_set_pixel_24(struct _ia_image_t* self, ia_uint16_t x, ia_uint16_t y, ia_uint32_t value)
{
	((ia_uint8_t*)self->pixels.data)[3*(y*self->width + x) + 0] = (ia_uint8_t)((value >>  0) & 0xFF);
	((ia_uint8_t*)self->pixels.data)[3*(y*self->width + x) + 1] = (ia_uint8_t)((value >>  8) & 0xFF);
	((ia_uint8_t*)self->pixels.data)[3*(y*self->width + x) + 2] = (ia_uint8_t)((value >> 16) & 0xFF);
}

static ia_uint32_t ia_image_get_pixel_24(struct _ia_image_t* self, ia_uint16_t x, ia_uint16_t y)
{
	return 
	(((ia_uint8_t*)self->pixels.data)[3*(y*self->width + x) + 0] << 0) |
	(((ia_uint8_t*)self->pixels.data)[3*(y*self->width + x) + 1] << 8) |
	(((ia_uint8_t*)self->pixels.data)[3*(y*self->width + x) + 2] << 16);
}

static void ia_image_set_pixel_32(struct _ia_image_t* self, ia_uint16_t x, ia_uint16_t y, ia_uint32_t value)
{
	((ia_uint32_t*)self->pixels.data)[y*self->width + x]=value;
}

static ia_uint32_t ia_image_get_pixel_32(struct _ia_image_t* self, ia_uint16_t x, ia_uint16_t y)
{
	return ((ia_uint32_t*)self->pixels.data)[y*self->width + x];
}

static void ia_image_set_pixel(struct _ia_image_t* self, ia_uint16_t x, ia_uint16_t y, ia_uint32_t value)
{
	if (x<self->width && y<self->height)
	{
		switch (self->format)
		{
		case IAT_BOOL:
				ia_image_set_pixel_2(self, x, y, (ia_bool_t)value);
				break;
		case IAT_UINT_8: case IAT_INT_8:
				ia_image_set_pixel_8(self, x, y, (ia_uint8_t)value);
				break;
		case IAT_UINT_16: case IAT_INT_16:
				ia_image_set_pixel_16(self, x, y, (ia_uint16_t)value);
				break;
		case IAT_UINT_24: case IAT_INT_24:
				ia_image_set_pixel_24(self, x, y, (ia_uint32_t)value);
				break;
		case IAT_UINT_32: case IAT_INT_32:
				ia_image_set_pixel_32(self, x, y, (ia_uint32_t)value);
				break;
			default:
				ASSERT(0), "image:set_pixel(%d, %d, %X) -> Not supported format %d!\n", x, y, (unsigned int)value, self->format);
		}
	}
}

static ia_uint32_t ia_image_get_pixel(struct _ia_image_t* self, ia_uint16_t x, ia_uint16_t y)
{
	if (x<self->width && y<self->height)
	{
		switch (self->format)
		{
			case IAT_BOOL:
				return ia_image_get_pixel_2(self, x, y);
			case IAT_UINT_8: case IAT_INT_8: 
				return ia_image_get_pixel_8(self, x, y);
			case IAT_UINT_16: case IAT_INT_16:
				return ia_image_get_pixel_16(self, x, y);
			case IAT_UINT_24: case IAT_INT_24:
				return ia_image_get_pixel_24(self, x, y);
			case IAT_UINT_32: case IAT_INT_32:
				return ia_image_get_pixel_32(self, x, y);
			default:
				ASSERT(0), "image:get_pixel(%d, %d) -> Not supported format %d!\n", x, y, self->format);
		}
	}
	return 0;
}

static void ia_image_fill(struct _ia_image_t* self, ia_uint32_t value)
{
	int i, j;
	for (i=0; i<self->height; i++)
		for (j=0; j<self->width; j++)
			self->set_pixel(self, j, i, value);
}

static struct _ia_image_t* ia_image_convert_rgb(struct _ia_image_t* self)
{
	int i, j;
	ia_image_p img_new;
	if (self->is_gray)
	{
		ia_image_p img_temp = self->copy(self);
		img_temp->normalize_colors(img_temp, 0, 0, 0, 0);
		img_new = ia_image_new(self->width, self->height, IAT_UINT_32, IA_IMAGE_RGB);
		for (i=0; i<img_temp->height; i++)
		for (j=0; j<img_temp->width; j++)
		{
			ia_uint32_t c = (ia_uint32_t)img_temp->get_pixel(img_temp, j, i);
			img_new->set_pixel(img_new, j, i, IA_RGB(c, c, c));
		}
		img_temp->destroy(img_temp);
	}
	else if (ia_format_size(self->format) != ia_format_size(IAT_UINT_32))
	{
		ASSERT(ia_format_size(self->format) >= 24), "FIXME: Converting to 32 bit RGB format from %d bit is not supported!\n", ia_format_size(self->format));
		img_new = ia_image_new(self->width, self->height, IAT_UINT_32, IA_IMAGE_RGB);
		for (i=0; i<img_new->height; i++)
		for (j=0; j<img_new->width; j++)
			img_new->set_pixel(img_new, j, i, self->get_pixel(self, j, i));
	} else
	{
		img_new=self->copy(self);
	}
	return img_new;
}

static struct _ia_image_t* ia_image_convert_gray(struct _ia_image_t* self, ia_format_t format)
{
	ia_uint16_t i, j;
	ia_image_p img_new;
	if (!self->is_gray)
	{
		/* convert RGB image */
		ia_image_p img_temp;
		ASSERT(ia_format_size(self->format) >= 24), "FIXME: Converting to gray from %d bit RGB format is not supported!\n", ia_format_size(self->format));
		img_temp = self->convert_rgb(self);
		img_new = ia_image_new(self->width, self->height, format, IA_IMAGE_GRAY);
		for (i=0; i<img_temp->height; i++)
		for (j=0; j<img_temp->width; j++)
		{
			ia_uint32_t c = img_temp->get_pixel(img_temp, j, i);
			ia_uint8_t  g = IA_GRAY(c);
			if (format == IAT_BOOL)
			{
				img_new->set_pixel(img_new, j, i, (g>=128?1:0));
			} else
			{
				img_new->set_pixel(img_new, j, i, g); 
			}
		}
		img_temp->destroy(img_temp);
	}
	else if (ia_format_size(self->format) != ia_format_size(format))
	{
		/* convert gray image */
		ia_int32_t min, new_min;
		ia_uint32_t max, new_max;
		img_new = ia_image_new(self->width, self->height, format, IA_IMAGE_GRAY);
		self->get_min_max(self, &min, &max);
		ia_format_min_max(format, &new_min, &new_max);
		for (i=0; i<img_new->height; i++)
		for (j=0; j<img_new->width; j++)
		{
			ia_uint32_t c = self->get_pixel(self, j, i);
			ia_uint32_t c1=(ia_uint32_t)floor(new_min + (new_max-new_min)*(float)(c-min)/(float)(max-min));
			img_new->set_pixel(img_new, j, i, c1);
		}
	}
	else
	{
		img_new=self->copy(self);
	}
	ASSERT(img_new->is_gray), "Internal Error! The returned image is not marked as gray\n");
	return img_new;
}

static void ia_image_normalize_colors(struct _ia_image_t* self, ia_int32_t min, ia_uint32_t max, ia_int32_t new_min, ia_uint32_t new_max)
{
	ia_uint16_t i,j;
	ASSERT(self->is_gray), "FIXME: RGB format is not supported by ia_image_normalize_colors!\n");
	if (!min && !max)
	{
		self->get_min_max(self, &min, &max);
	}

	if (!new_min && !new_max)
	{
		ia_format_min_max(self->format, &new_min, &new_max);
	}

	if (min != max)
	{
		for (i=0; i<self->height; i++)
		for (j=0; j<self->width; j++)
		{
			ia_uint32_t c = self->get_pixel(self, j, i);
			c=(ia_uint32_t)floor(new_min + (new_max-new_min)*(float)(c-min)/(float)(max-min));
			self->set_pixel(self, j, i, c);
		}
	}
}

static void ia_image_mask(struct _ia_image_t* self, struct _ia_image_t* mask, ia_mask_t mask_operation)
{
	ia_int32_t i, j;
	for (i=0; i<self->height; i++)
	for (j=0; j<self->width; j++)
	{
		ia_int32_t col = self->get_pixel(self, j, i);
		ia_int32_t maskcol = mask->get_pixel(mask, j, i);
		ia_int32_t outcol;

		switch (mask_operation)
		{
			case IA_MASK_OR:  
				/*outcol = (col || maskcol)?col:0;*/
				outcol = col | maskcol;
			break;
			case IA_MASK_AND: 
				/*outcol = (col && maskcol)?col:0;*/
				outcol = col & maskcol;
			break;
			case IA_MASK_XOR: 
				/*outcol = (col ^ maskcol)?col:0;*/
				outcol = col ^ maskcol;
			break;
			default:
				outcol = col;
			break;
		}
		self->set_pixel(self, j, i, outcol);
	}
}

static void ia_image_extract_hsv(struct _ia_image_t* self, ia_uint32_t huemin, ia_uint32_t huemax, ia_uint32_t satmin, ia_uint32_t satmax, ia_uint32_t valmin, ia_uint32_t valmax)
{
	ia_int32_t i, j;
	for (i=0; i<self->height; i++)
	for (j=0; j<self->width; j++)
	{
		ia_uint32_t hue, sat, val;
		ia_rgb_to_hsv(self->get_pixel(self, j, i), &hue, &sat, &val);
		if (!((hue>=huemin && hue<=huemax) || (sat>=satmin && sat<=satmax) || (val>=valmin && val<=valmax)))
		{
			self->set_pixel(self, j, i, 0);
		}
	}
}

static void ia_image_inverse(struct _ia_image_t* self)
{
	ia_int32_t  i,j;
	ia_int32_t  min;
	ia_uint32_t max;

	if (self->is_gray)
	{
		self->get_min_max(self, &min, &max);

		for (i=0; i<self->height; i++)
		for (j=0; j<self->width; j++)
			self->set_pixel(self, j, i, min+max-self->get_pixel(self, j, i));
	}
	else
	{
		for (i=0; i<self->height; i++)
		for (j=0; j<self->width; j++)
		{
			ia_uint32_t color = self->get_pixel(self, j, i);
			self->set_pixel(self, j, i, IA_RGB(ABS((ia_int32_t)IA_RED(color)-255), ABS((ia_int32_t)IA_GREEN(color)-255), ABS((ia_int32_t)IA_BLUE(color)-255)));
		}
	}
}

static struct _ia_image_t* ia_image_substract(struct _ia_image_t* self, struct _ia_image_t* substractor)
{
	ia_int32_t i, j;
	ia_image_p sub;
	ASSERT(self->format == substractor->format && self->width == substractor->width && self->height == substractor->height), 
		"format or dimmension does not match between substractor and substracted images\n");

	if (self->is_gray)
	{
		/* keep the original grayscale pixel format when dividing gray images */
		sub = ia_image_new(self->width, self->height, self->format, IA_IMAGE_GRAY);
		for (i=0; i<self->height; i++)
		for (j=0; j<self->width; j++)
		{
			ia_int32_t substracted_color = self->get_pixel(self, j, i);
			substracted_color -= substractor->get_pixel(substractor, j, i);
			if (substracted_color < 0) substracted_color = -substracted_color;
			sub->set_pixel(sub, j, i, substracted_color);
		}
	}
	else
	{
		/* 8-bit grayscale pixel format for dividing RGB images */
		sub = ia_image_new(self->width, self->height, IAT_UINT_8, IA_IMAGE_GRAY);
		for (i=0; i<self->height; i++)
		for (j=0; j<self->width; j++)
		{
			ia_int32_t color = self->get_pixel(self, j, i);
			ia_int32_t substracted_color = IA_GRAY(color);
			color = substractor->get_pixel(substractor, j, i);
			color = IA_GRAY(color);
			substracted_color -= color;
			if (substracted_color < 0) substracted_color = -substracted_color;
			sub->set_pixel(sub, j, i, substracted_color);
		}
	}

	return sub;
}

static void ia_image_binarize_threshold(struct _ia_image_t* img, ia_int32_t threshold)
{
	ia_uint16_t i,j;
	ia_int32_t min;
	ia_uint32_t max;
	ia_bool_t is_signed=ia_format_signed(img->format);
	ia_format_min_max(img->format, &min, &max);
	for (i=0; i<img->height; i++)
	for (j=0; j<img->width; j++)
	{
		ia_uint32_t c = img->get_pixel(img, j, i);
		/* Notice the different typecasts according if the image pixels are signed or not */
		if ((is_signed && ((ia_int32_t)c >= threshold)) || (!is_signed && (c>=(ia_uint32_t)threshold)))
		{
			img->set_pixel(img, j, i, max);
		}
		else
		{
			img->set_pixel(img, j, i, min);
		}
	}
}

static void ia_image_binarize_threshold_2(struct _ia_image_t* img, ia_int32_t threashold1, ia_int32_t threashold2)
{
	ia_uint16_t i,j;
	ia_int32_t min;
	ia_uint32_t max;
	ia_int32_t mid;
	ia_bool_t is_signed=ia_format_signed(img->format);
	ia_format_min_max(img->format, &min, &max);
	if (threashold1 > threashold2)
	{
		/* here mid is used for temp var */
		mid = threashold2;
		threashold2 = threashold1;
		threashold1 = mid;
	}
	mid = (max + min) >> 1;

	for (i=0; i<img->height; i++)
	for (j=0; j<img->width; j++)
	{
		ia_uint32_t c = img->get_pixel(img, j, i);
		/* Notice different typecasts according if the image pixels are signed or not */
		if ((is_signed && ((ia_int32_t)c >= threashold2)) || (!is_signed && (c>=(ia_uint32_t)threashold2)))
		{
			img->set_pixel(img, j, i, max);
		}
		else
		if ((is_signed && ((ia_int32_t)c >= threashold1)) || (!is_signed && (c>=(ia_uint32_t)threashold1)))
		{
			img->set_pixel(img, j, i, mid);
		}
		else
		{
			img->set_pixel(img, j, i, min);
		}
	}
}

static void ia_image_binarize_threshold_3(struct _ia_image_t* img, ia_int32_t threashold1, ia_int32_t threashold2, ia_int32_t threashold3)
{
	ia_uint16_t i,j;
	ia_int32_t min;
	ia_uint32_t max;
	ia_int32_t mid1, mid2;
	ia_bool_t is_signed=ia_format_signed(img->format);
	ia_format_min_max(img->format, &min, &max);

	/* sorts threashold1 <= threashold2 <= threashold3 */
	/* below mid1 is used for temp var */
	if (threashold1 > threashold2)
	{
		mid1 = threashold2;
		threashold2 = threashold1;
		threashold1 = mid1;
	}

	mid1 = (max + min) / 3;
	mid2 = 2*mid1;

	for (i=0; i<img->height; i++)
	for (j=0; j<img->width; j++)
	{
		ia_uint32_t c = img->get_pixel(img, j, i);
		/* Notice different typecasts according if the image pixels are signed or not */
		if ((is_signed && ((ia_int32_t)c >= threashold3)) || (!is_signed && (c>=(ia_uint32_t)threashold3)))
		{
			img->set_pixel(img, j, i, max);
		}
		else
		if ((is_signed && ((ia_int32_t)c >= threashold2)) || (!is_signed && (c>=(ia_uint32_t)threashold2)))
		{
			img->set_pixel(img, j, i, mid2);
		}
		else
		if ((is_signed && ((ia_int32_t)c >= threashold1)) || (!is_signed && (c>=(ia_uint32_t)threashold1)))
		{
			img->set_pixel(img, j, i, mid1);
		}
		else
		{
			img->set_pixel(img, j, i, min);
		}
	}
}

static int ia_image_binarize_otsu(struct _ia_image_t* img, ia_signal_p histo)
{
	ia_int32_t result;
	ia_uint32_t threshold;
	int histoalloced = 0;
	if (!histo)
	{
		histo = img->histogram(img, IA_COLOR_ELEMENT_VALUE);
		histoalloced = 1;
	}

	if ((result=ia_otsu(histo, &threshold, 0, 0, 0, 0, 0, 0)) > 0)
	{
		ia_image_binarize_threshold(img, threshold);
	}

	if (histoalloced)
	{
		histo->destroy(histo);
	}
	return result;
}

static int ia_image_binarize_otsu_2(struct _ia_image_t* img, ia_signal_p histo)
{	
	ia_int32_t result;
	ia_uint32_t threshold1, threshold2;
	int histoalloced = 0;
	if (!histo)
	{
		histo = img->histogram(img, IA_COLOR_ELEMENT_VALUE);
		histoalloced = 1;
	}

	if ((result=ia_otsu_2(histo, &threshold1, &threshold2, 0, 0, 0, 0, 0, 0, 0, 0)) > 0)
	{
		ia_image_binarize_threshold_2(img, threshold1, threshold2);
	}

	if (histoalloced)
	{
		histo->destroy(histo);
	}
	return result;
}

static void ia_image_get_min_max(struct _ia_image_t* self, ia_int32_t* min, ia_uint32_t* max)
{
	ia_uint16_t i,j;
	ia_bool_t is_signed=ia_format_signed(self->format);
	ia_format_min_max(self->format, (ia_int32_t*)max, (ia_uint32_t*)min);
	for (i=0; i<self->height; i++)
	for (j=0; j<self->width; j++)
	{
		if (is_signed)
		{
			ia_int32_t c = (ia_int32_t)self->get_pixel(self, j, i);
			if (c>(ia_int32_t)*max) *max=(ia_int32_t)c;
			if (c<*min) *min=c;
		}
		else
		{
			ia_uint32_t c = self->get_pixel(self, j, i);
			if (c>*max) *max=c;
			if (c<(ia_uint32_t)*min) *min=c;
		}
	}
}

static struct _ia_image_t* ia_image_copy(struct _ia_image_t* self)
{
	ia_image_p img_new = ia_image_new(self->width, self->height, self->format, self->is_gray);
	memcpy(img_new->pixels.data, self->pixels.data, self->pixels.size);
	return img_new;
}


typedef struct 
{
	ia_image_p image;
	ia_uint32_t color;
} draw_line_param_t, *draw_line_param_p;

static void ia_image_draw_line_callback(void* param, ia_int32_t x, ia_int32_t y)
{
	draw_line_param_p p = (draw_line_param_p)param;
	p->image->set_pixel(p->image, x, y, p->color);
}

static void ia_image_draw_line(struct _ia_image_t* self, ia_int32_t x1, ia_int32_t y1, ia_int32_t x2, ia_int32_t y2, ia_uint32_t color)
{
	ia_rect_t clip_rgn;
	draw_line_param_t param;

	param.image = self;
	param.color = color;
	clip_rgn.l = 0;
	clip_rgn.t = 0;
	clip_rgn.r = self->width-1;
	clip_rgn.b = self->height-1;
	ia_line_draw(x1, y1, x2, y2, &clip_rgn, ia_image_draw_line_callback, (void*)&param);
}

static ia_signal_p ia_image_histogram(struct _ia_image_t* self, ia_color_element_t color_element)
{
	ia_int32_t i, j, length = 256;
	ia_signal_p histogram;
	if (color_element == IA_COLOR_ELEMENT_HUE)
	{
		length = 360;
	}
	histogram = ia_signal_new(length, IAT_UINT_32, IA_IMAGE_GRAY);

	for (i=0; i<self->height; i++)
	for (j=0; j<self->width; j++)
	{
		ia_uint32_t color = self->get_pixel(self, j, i);
		if (!self->is_gray) /* color element is ignored */
		{
			switch (color_element)
			{
			case IA_COLOR_ELEMENT_RED:color = IA_RED(color); break;
			case IA_COLOR_ELEMENT_GREEN:color = IA_GREEN(color); break;
			case IA_COLOR_ELEMENT_BLUE: color = IA_BLUE(color); break;
			case IA_COLOR_ELEMENT_HUE:ia_rgb_to_hsv(color, &color, 0, 0); break;
			case IA_COLOR_ELEMENT_SATURATION: ia_rgb_to_hsv(color, 0, &color, 0); break;
			default: /* case IA_COLOR_ELEMENT_VALUE */ ia_rgb_to_hsv(color, 0, 0, &color); break;
			}
		} 

		histogram->set_pixel(histogram, (ia_uint16_t)color, 1+histogram->get_pixel(histogram, (ia_uint16_t)color));
	}
	return histogram;
}

typedef struct
{
	ia_image_p image;
	ia_signal_p signal;
	ia_bool_t ishoriz;
	ia_int32_t x0;
	ia_int32_t y0;
} line_to_signal_param_t, *line_to_signal_param_p;

static void ia_image_line_to_signal_callback(void* param, ia_int32_t x, ia_int32_t y)
{
	line_to_signal_param_p p = (line_to_signal_param_p)param;
	ia_uint32_t c = p->image->get_pixel(p->image, x, y);
	if (p->ishoriz)
	{
		p->signal->set_pixel(p->signal, x - p->x0, c);
	}
	else
	{
		p->signal->set_pixel(p->signal, y - p->y0, c);
	}
}

static ia_signal_p ia_image_line_to_signal(struct _ia_image_t* self, ia_int32_t x1, ia_int32_t y1, ia_int32_t x2, ia_int32_t y2)
{
	ia_uint32_t temp;
	ia_int32_t dx = ABS(x1-x2);
	ia_int32_t dy = ABS(y1-y2);
	ia_rect_t clip_rgn;
	line_to_signal_param_t param;
	param.ishoriz = dx > dy;
	if ((param.ishoriz && (x1 > x2)) || (!param.ishoriz && (y1 > y2)))
	{
		temp = x1;
		x1 = x2;
		x2 = temp;
		temp = y1;
		y1 = y2;
		y2 = temp;
	}
	param.image = self; 
	param.signal = ia_signal_new(param.ishoriz?dx:dy, self->format, self->is_gray);
	if (!param.signal)
	{
		return NULL;
	}
	param.x0 = x1;
	param.y0 = y1;

	clip_rgn.l = 0;
	clip_rgn.t = 0;
	clip_rgn.r = self->width-1;
	clip_rgn.b = self->height-1;
	ia_line_draw(x1, y1, x2, y2, &clip_rgn, ia_image_line_to_signal_callback, (void*)&param);
	return param.signal;
}

static void ia_image_print(struct _ia_image_t* self)
{
	ia_int32_t i,j;
	for (i=0; i<self->height; i++)
	{
		for (j=0; j<self->width; j++)
		{
			ia_uint8_t c=(ia_uint8_t)self->get_pixel(self, j, i);
			if ((j == self->marker_x) && (i == self->marker_y))
			{
				printf("+");
			} else
			{
				if (c)
					printf("%d", c);
				else
					printf("-");
			}
		}
		printf("\n");
	}
	for (j=0; j<self->width; j++) printf("=");
	printf("\n");
}
