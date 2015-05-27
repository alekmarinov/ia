/*********************************************************************/
/*                                                                   */
/* Copyright (C) 2005, Alexander Marinov, Nadezhda Zlateva           */
/*                                                                   */
/* Project:       ia                                                 */
/* Filename:      ia_signal.c                                        */
/* Description:   Signal management module                           */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <ia/ia_signal.h>
#include <ia/algo/ia_otsu.h>

/*********************************************************************/
/*                        Local prototypes                           */
/*********************************************************************/

static void                 ia_signal_destroy         (struct _ia_signal_t*);
static void                 ia_signal_set_pixel_2     (struct _ia_signal_t*, ia_uint16_t, ia_bool_t);
static ia_bool_t            ia_signal_get_pixel_2     (struct _ia_signal_t*, ia_uint16_t);
static void                 ia_signal_set_pixel_8     (struct _ia_signal_t*, ia_uint16_t, ia_uint8_t);
static ia_uint8_t           ia_signal_get_pixel_8     (struct _ia_signal_t*, ia_uint16_t);
static void                 ia_signal_set_pixel_16    (struct _ia_signal_t*, ia_uint16_t, ia_uint16_t);
static ia_uint16_t          ia_signal_get_pixel_16    (struct _ia_signal_t*, ia_uint16_t);
static void                 ia_signal_set_pixel_24    (struct _ia_signal_t*, ia_uint16_t, ia_uint32_t);
static ia_uint32_t          ia_signal_get_pixel_24    (struct _ia_signal_t*, ia_uint16_t);
static void                 ia_signal_set_pixel_32    (struct _ia_signal_t*, ia_uint16_t, ia_uint32_t);
static ia_uint32_t          ia_signal_get_pixel_32    (struct _ia_signal_t*, ia_uint16_t);
static void                 ia_signal_set_pixel       (struct _ia_signal_t*, ia_uint16_t, ia_uint32_t);
static ia_uint32_t          ia_signal_get_pixel       (struct _ia_signal_t*, ia_uint16_t);
static void                 ia_signal_fill            (struct _ia_signal_t*, ia_uint32_t);
static struct _ia_signal_t* ia_signal_convert_rgb     (struct _ia_signal_t*);
static struct _ia_signal_t* ia_signal_convert_gray    (struct _ia_signal_t*, ia_format_t);
static void                 ia_signal_normalize_colors(struct _ia_signal_t*, ia_int32_t, ia_uint32_t, ia_int32_t, ia_uint32_t);
static void                 ia_signal_inverse         (struct _ia_signal_t*);
static void                 ia_signal_multiply_number (struct _ia_signal_t*, ia_double_t);
static void                 ia_signal_add_number      (struct _ia_signal_t*, ia_double_t);
static void                 ia_signal_add_signal      (struct _ia_signal_t*, struct _ia_signal_t*);
static ia_int32_t           ia_signal_sum             (struct _ia_signal_t*);
static ia_int32_t           ia_signal_linear_combination(struct _ia_signal_t*, struct _ia_signal_t*);
static void                 ia_signal_get_min_max     (struct _ia_signal_t*, ia_int32_t*, ia_uint32_t*);
static void                 ia_signal_shift           (struct _ia_signal_t*, ia_int32_t);
static struct _ia_signal_t* ia_signal_copy            (struct _ia_signal_t*);

/*********************************************************************/
/*                        Implementation                             */
/*********************************************************************/

ia_signal_p ia_signal_new(ia_uint16_t length, ia_format_t format, ia_bool_t is_gray)
{
	ia_uint32_t sig_size  = ((length * ia_format_size(format) + 7) >> 3);
	ia_signal_p sig       = (ia_signal_p)malloc(sizeof(ia_signal_t));

	sig->length           = length;
	sig->format           = format;
	sig->is_gray          = is_gray;
	sig->pixels.data      = (ia_data_p)calloc(1, sig_size);
	sig->pixels.size      = sig_size;

	sig->destroy          = ia_signal_destroy;
	sig->set_pixel        = ia_signal_set_pixel;
	sig->get_pixel        = ia_signal_get_pixel;
	sig->fill             = ia_signal_fill;
	sig->convert_rgb      = ia_signal_convert_rgb;
	sig->convert_gray     = ia_signal_convert_gray;
	sig->normalize_colors = ia_signal_normalize_colors;
	sig->inverse          = ia_signal_inverse;
	sig->multiply_number  = ia_signal_multiply_number;
	sig->add_number       = ia_signal_add_number;
	sig->add_signal       = ia_signal_add_signal;
	sig->sum              = ia_signal_sum;
	sig->linear_combination = ia_signal_linear_combination;
	sig->threshold_otsu   = ia_otsu;
	sig->threshold_otsu_2 = ia_otsu_2;
	sig->get_min_max      = ia_signal_get_min_max;
	sig->shift            = ia_signal_shift;
	sig->copy             = ia_signal_copy;

	return sig;
}

static void ia_signal_destroy(struct _ia_signal_t* self)
{
	if (self->pixels.data)
	{
		free(self->pixels.data);
	}
	free(self);
}
static void ia_signal_set_pixel_2(struct _ia_signal_t* self, ia_uint16_t x, ia_bool_t value)
{
	ia_uint32_t ofs=(x >> 3);
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

static ia_bool_t ia_signal_get_pixel_2(struct _ia_signal_t* self, ia_uint16_t x)
{
	ia_uint32_t ofs=(x >> 3);
	ia_uint32_t bit=1<<(x & 7);
	return ((((ia_uint8_t*)self->pixels.data)[ofs]&bit)?1:0);
}

static void ia_signal_set_pixel_8(struct _ia_signal_t* self, ia_uint16_t x, ia_uint8_t value)
{
	((ia_uint8_t*)self->pixels.data)[x]=value;
}

static ia_uint8_t ia_signal_get_pixel_8(struct _ia_signal_t* self, ia_uint16_t x)
{
	return ((ia_uint8_t*)self->pixels.data)[x];
}

static void ia_signal_set_pixel_16(struct _ia_signal_t* self, ia_uint16_t x, ia_uint16_t value)
{
	((ia_uint16_t*)self->pixels.data)[x]=value;
}

static ia_uint16_t ia_signal_get_pixel_16(struct _ia_signal_t* self, ia_uint16_t x)
{
	return ((ia_uint16_t*)self->pixels.data)[x];
}

static void ia_signal_set_pixel_24(struct _ia_signal_t* self, ia_uint16_t x, ia_uint32_t value)
{
	((ia_uint8_t*)self->pixels.data)[x + 0] = (ia_uint8_t)((value >>  0) & 0xFF);
	((ia_uint8_t*)self->pixels.data)[x + 1] = (ia_uint8_t)((value >>  8) & 0xFF);
	((ia_uint8_t*)self->pixels.data)[x + 2] = (ia_uint8_t)((value >> 16) & 0xFF);
}

static ia_uint32_t ia_signal_get_pixel_24(struct _ia_signal_t* self, ia_uint16_t x)
{
	return 
	(((ia_uint8_t*)self->pixels.data)[x + 0] << 0) |
	(((ia_uint8_t*)self->pixels.data)[x + 1] << 8) |
	(((ia_uint8_t*)self->pixels.data)[x + 2] << 16);
}

static void ia_signal_set_pixel_32(struct _ia_signal_t* self, ia_uint16_t x, ia_uint32_t value)
{
	((ia_uint32_t*)self->pixels.data)[x]=value;
}

static ia_uint32_t ia_signal_get_pixel_32(struct _ia_signal_t* self, ia_uint16_t x)
{
	return ((ia_uint32_t*)self->pixels.data)[x];
}

static void ia_signal_set_pixel(struct _ia_signal_t* self, ia_uint16_t x, ia_uint32_t value)
{
	if (x<self->length)
	{
		switch (self->format)
		{
		case IAT_BOOL:
				ia_signal_set_pixel_2(self, x, (ia_bool_t)value);
				break;
		case IAT_UINT_8: case IAT_INT_8:
				ia_signal_set_pixel_8(self, x, (ia_uint8_t)value);
				break;
		case IAT_UINT_16: case IAT_INT_16:
				ia_signal_set_pixel_16(self, x, (ia_uint16_t)value);
				break;
		case IAT_UINT_24: case IAT_INT_24:
				ia_signal_set_pixel_24(self, x, (ia_uint32_t)value);
				break;
		case IAT_UINT_32: case IAT_INT_32: case IAT_FLOAT:
				ia_signal_set_pixel_32(self, x, (ia_uint32_t)value);
				break;
			default:
				ASSERT(0), "signal:set_pixel(%d, %X) -> Not supported format %d!\n", x, (unsigned int)value, self->format);
		}
	}
}

static ia_uint32_t ia_signal_get_pixel(struct _ia_signal_t* self, ia_uint16_t x)
{
	if (x<self->length)
	{
		switch (self->format)
		{
			case IAT_BOOL:
				return ia_signal_get_pixel_2(self, x);
			case IAT_UINT_8: case IAT_INT_8: 
				return ia_signal_get_pixel_8(self, x);
			case IAT_UINT_16: case IAT_INT_16:
				return ia_signal_get_pixel_16(self, x);
			case IAT_UINT_24: case IAT_INT_24:
				return ia_signal_get_pixel_24(self, x);
			case IAT_UINT_32: case IAT_INT_32: case IAT_FLOAT:
				return ia_signal_get_pixel_32(self, x);
			default:
				ASSERT(0), "signal:get_pixel(%d) -> Not supported format %d!\n", x, self->format);
		}
	}
	return 0;
}

static void ia_signal_fill(struct _ia_signal_t* self, ia_uint32_t value)
{
	int i;
	for (i=0; i<self->length; i++)
			self->set_pixel(self, i, value);
}

static struct _ia_signal_t* ia_signal_convert_rgb(struct _ia_signal_t* self)
{
	int i;
	ia_signal_p sig_new;
	if (self->is_gray)
	{
		ia_signal_p sig_temp = self->copy(self);
		sig_temp->normalize_colors(sig_temp, 0, 0, 0, 255);
		sig_new = ia_signal_new(self->length, IAT_UINT_32, IA_IMAGE_RGB);
		for (i=0; i<sig_temp->length; i++)
		{
			ia_uint32_t c = (ia_uint32_t)sig_temp->get_pixel(sig_temp, i);
			sig_new->set_pixel(sig_new, i, IA_RGB(c, c, c));
		}
		sig_temp->destroy(sig_temp);
	}
	else if (ia_format_size(self->format) != ia_format_size(IAT_UINT_32))
	{
		ASSERT(ia_format_size(self->format) >= 24), "FIXME: Converting to 32 bit RGB format from %d bit is not supported!\n", ia_format_size(self->format));
		sig_new = ia_signal_new(self->length, IAT_UINT_32, IA_IMAGE_RGB);
		for (i=0; i<sig_new->length; i++)
			sig_new->set_pixel(sig_new, i, self->get_pixel(self, i));
	} 
	else
	{
		sig_new=self->copy(self);
	}
	return sig_new;
}

static struct _ia_signal_t* ia_signal_convert_gray(struct _ia_signal_t* self, ia_format_t format)
{
	ia_uint16_t i;
	ia_signal_p sig_new = NULL;
	if (!self->is_gray)
	{
		/* convert RGB signal */
		ia_signal_p sig_temp;
		ASSERT(ia_format_size(self->format) >= 24), "FIXME: Converting to gray from %d bit RGB format is not supported!\n", ia_format_size(self->format));
		sig_temp = self->convert_rgb(self);
		sig_new = ia_signal_new(self->length, format, IA_IMAGE_GRAY);
		for (i=0; i<sig_temp->length; i++)
		{
			ia_uint32_t c = sig_temp->get_pixel(sig_temp, i);
			ia_uint8_t  g = IA_GRAY(c);
			if (format == IAT_BOOL)
			{
				sig_new->set_pixel(sig_new, i, (g>=128?1:0));
			} 
			else
			{
				sig_new->set_pixel(sig_new, i, g); 
			}
		}
		sig_temp->destroy(sig_temp);
	}
	else if (ia_format_size(self->format) != ia_format_size(format))
	{
		/* convert gray signal */
		ia_int32_t min;
		ia_uint32_t max;
		if (ia_format_size(self->format) < ia_format_size(format))
		{
			sig_new = ia_signal_new(self->length, format, IA_IMAGE_GRAY);
			for (i=0; i<sig_new->length; i++)
				sig_new->set_pixel(sig_new, i, self->get_pixel(self, i));
		}
		else
		{
			sig_new=self->copy(self);
		}
		ia_format_min_max(format, &min, &max);
		sig_new->normalize_colors(sig_new, 0, 0, min, max);
	}		
	ASSERT(sig_new->is_gray), "Internal Error! The returned signal is not marked as gray\n");
	return sig_new;
}

static void ia_signal_normalize_colors(struct _ia_signal_t* self, ia_int32_t min, ia_uint32_t max, ia_int32_t new_min, ia_uint32_t new_max)
{
	ia_uint16_t i;
	ASSERT(self->is_gray), "FIXME: RGB format is not supported by ia_signal_normalize_colors!\n");
	if (!min && !max)
	{
		self->get_min_max(self, &min, &max);
	}

	if (min != max)
	{
		for (i=0; i<self->length; i++)
		{
			ia_uint32_t c = self->get_pixel(self, i);
			c=(ia_uint32_t)floor(new_min + (new_max-new_min)*(float)(c-min)/(float)(max-min));
			self->set_pixel(self, i, c);
		}
	}
}

static void ia_signal_inverse(struct _ia_signal_t* self)
{
	ia_int32_t  i;
	ia_int32_t  min;
	ia_uint32_t max;

	ASSERT(self->is_gray), "FIXME: RGB format is not supported by ia_signal_inverse!\n");

	self->get_min_max(self, &min, &max);

	for (i=0; i<self->length; i++)
		self->set_pixel(self, i, min+max-self->get_pixel(self, i));
}

static void ia_signal_multiply_number(struct _ia_signal_t* self, ia_double_t num)
{
	ia_int32_t i;
	for (i=0; i<self->length; i++)
	{
		self->set_pixel(self, i, (ia_int32_t)(num * self->get_pixel(self, i)));
	}
}

static void ia_signal_add_number(struct _ia_signal_t* self, ia_double_t num)
{
	ia_int32_t i;
	for (i=0; i<self->length; i++)
	{
		self->set_pixel(self, i, (ia_int32_t)(num + self->get_pixel(self, i)));
	}
}

static void ia_signal_add_signal(struct _ia_signal_t* self, struct _ia_signal_t* signal)
{
	ia_int32_t i;
	ia_int32_t len = MIN(self->length, signal->length);
	for (i=0; i<len; i++)
	{
		self->set_pixel(self, i, (ia_int32_t)(signal->get_pixel(signal, i) + self->get_pixel(self, i)));
	}
}

static ia_int32_t ia_signal_sum(struct _ia_signal_t* self)
{
	ia_int32_t sum = 0;
	ia_int32_t i;
	for (i=0; i<self->length; i++)
	{
		sum += self->get_pixel(self, i);
	}
	return sum;
}

static ia_int32_t ia_signal_linear_combination(struct _ia_signal_t* self, struct _ia_signal_t* combiner)
{
	ia_int32_t i, min = MIN(self->length, combiner->length);
	ia_int32_t sum = 0;
	for (i=0; i<min; i++)
	{
		sum += self->get_pixel(self, i)*combiner->get_pixel(combiner, i);
	}
	return sum;
}

static void ia_signal_get_min_max(struct _ia_signal_t* self, ia_int32_t* min, ia_uint32_t* max)
{
	ia_uint16_t i;
	ia_bool_t is_signed=ia_format_signed(self->format);
	ia_format_min_max(self->format, (ia_int32_t*)max, (ia_uint32_t*)min);
	for (i=0; i<self->length; i++)
	{
		if (is_signed)
		{
			ia_int32_t c = (ia_int32_t)self->get_pixel(self, i);
			if (c>(ia_int32_t)*max) *max=(ia_int32_t)c;
			if (c<*min) *min=c;
		}
		else
		{
			ia_uint32_t c = self->get_pixel(self, i);
			if (c>*max) *max=c;
			if (c<(ia_uint32_t)*min) *min=c;
		}
	}
}

static void ia_signal_shift(struct _ia_signal_t* self, ia_int32_t direction)
{
	ia_int32_t pixelsize = ia_format_size(self->format) >> 3;
	direction = direction % self->length;
	if (direction > 0)
	{
		ia_uint32_t shiftsize = direction*pixelsize;
		memmove(((char*)self->pixels.data) + shiftsize, self->pixels.data, self->pixels.size - shiftsize);
		memset(self->pixels.data, 0, shiftsize);
	}
	else if (direction < 0)
	{
		ia_uint32_t shiftsize = -direction*pixelsize;
		ia_uint32_t remainingsize = self->pixels.size - shiftsize;
		memmove(self->pixels.data, ((char*)self->pixels.data) + shiftsize, remainingsize);
		memset(((char*)self->pixels.data) + remainingsize, 0, shiftsize);
	}
}

static struct _ia_signal_t* ia_signal_copy(struct _ia_signal_t* self)
{
	ia_signal_p sig_new = ia_signal_new(self->length, self->format, self->is_gray);
	memcpy(sig_new->pixels.data, self->pixels.data, self->pixels.size);
	return sig_new;
}
