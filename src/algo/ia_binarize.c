/*********************************************************************/
/*                                                                   */
/* Copyright (C) 2005, Alexander Marinov, Nadezhda Zlateva           */
/*                                                                   */
/* Project:       ia                                                 */
/* Filename:      ia_binarize.c                                      */
/* Description:   Binarization module                                */
/*                                                                   */
/*********************************************************************/
#include <ia/algo/ia_binarize.h>

void ia_binarize_level(ia_image_p img, ia_uint32_t level)
{
	ia_uint16_t i,j;
	ia_int32_t min;
	ia_uint32_t max;
	ia_bool_t is_signed=ia_format_signed(img->format);
	ia_format_min_max(img->format, &min, &max);
	for (i=0; i<img->height; i++)
	for (j=0; j<img->width; j++)
	{
		ia_uint32_t c= img->get_pixel(img, j, i);
		if ((is_signed && ((ia_int32_t)c >= (ia_int32_t)level)) || (!is_signed && (c>=level)) )
		{
			img->set_pixel(img, j, i, max);
		}
		else
		{
			img->set_pixel(img, j, i, min);
		}
	}
}

/* FIXME: implement OTSU! */
void ia_binarize_otsu_(ia_image_p img)
{
        ia_int32_t min;
        ia_uint32_t max;
        img->get_min_max(img, &min, &max);
        ia_binarize_level(img, min + ((max-min)>>1));
}

