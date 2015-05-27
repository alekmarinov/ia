/*********************************************************************/
/*                                                                   */
/* Copyright (C) 2005, Alexander Marinov, Nadezhda Zlateva           */
/*                                                                   */
/* Project:       ia                                                 */
/* Filename:      ia_image.h                                         */
/* Description:   Image management module                            */
/*                                                                   */
/*********************************************************************/

#ifndef __IA_IMAGE_H
#define __IA_IMAGE_H

#include <ia/ia.h>
#include <ia/ia_signal.h>

/*********************************************************************/
/*                              Image API                            */
/*********************************************************************/

/**
	Type ia_color_element_t 

	Defines enumeration of all color element types
*/
typedef enum
{
	IA_COLOR_ELEMENT_RED,
	IA_COLOR_ELEMENT_GREEN,
	IA_COLOR_ELEMENT_BLUE,
	IA_COLOR_ELEMENT_VALUE,
	IA_COLOR_ELEMENT_HUE,
	IA_COLOR_ELEMENT_SATURATION
} ia_color_element_t;

/**
	Type ia_image_t

	Defines set ot functions for primitive image manipulations
*/
typedef struct _ia_image_t
{
	/** image horizontal pixels count */
	ia_uint16_t                         width;

	/** image vertical pixels count */
	ia_uint16_t                         height;
	
	/** image pixel format */
	ia_format_t                         format;

	/** true if the image is gray */
	ia_bool_t                           is_gray;

	/** raw image data */
	ia_data_t                           pixels;

	/** true if image data is passed by the user and should not be freed */
	ia_bool_t                           is_user_data;

	/** marker x position */
	ia_uint32_t                         marker_x;

	/** marker y position */
	ia_uint32_t                         marker_y;

	/** set a pixel color at specified position */
	void (*set_pixel)                   (
		struct _ia_image_t*, /** self */
		ia_uint16_t,         /** x coordinate */
		ia_uint16_t,         /** y coordinate */
		ia_uint32_t          /** color value with bit size depending of the image pixel format */
	);

	/** get a pixel color from specified position */
	ia_uint32_t (*get_pixel)            (
		struct _ia_image_t*, /** self */
		ia_uint16_t,         /** x coordinate */
		ia_uint16_t          /** y coordinate */
	);

	/** fill the image with specified color */
	void (*fill)                        (
		struct _ia_image_t*, /** self */
		ia_uint32_t          /** color value with bit size depending of the image pixel format */
	);

	/** save image to file */
	void (*save)                        (
		struct _ia_image_t*, /** self */
		const ia_string_t    /* file name */
	);

	/** convert an image into 32-bit ARGB pixel format, where the alpha mask is 0 */
	struct _ia_image_t* (*convert_rgb)  (
		struct _ia_image_t* /** self */
	);

	/** convert an image to gray */
	struct _ia_image_t* (*convert_gray) (
		struct _ia_image_t*, /** self */
		ia_format_t          /** pixel format */
	);
	
	/** normalize pixel colors to occupy better the specified range */
	void (*normalize_colors)            (
		struct _ia_image_t*, /** self */
		ia_int32_t,           /** from min color or 0 if unknown */
		ia_uint32_t,          /** from max color or 0 if unknown */
		ia_int32_t,           /** to min color */
		ia_uint32_t           /** to max color */
	);

	/** inverse */
	void (*inverse)                     (
		struct _ia_image_t* /** self */
	);

	/** mask */
	void (*mask)                        (
		struct _ia_image_t*, /** self */
		struct _ia_image_t*, /** mask */
		ia_mask_t            /** mask operation **/
	);

	/** substract image */
	struct _ia_image_t* (*substract)    (
		struct _ia_image_t*, /** self */
		struct _ia_image_t*  /** image substractor */
	);

	/** keep all image pixels in given HSV ranges */
	void (*extract_hsv)                 (
		struct _ia_image_t*, /** self */
		ia_uint32_t, ia_uint32_t, /* hue min/max */
		ia_uint32_t, ia_uint32_t, /* sat min/max */
		ia_uint32_t, ia_uint32_t  /* val min/max */
	);

	/** determines min and max colors in the image */
	void (*get_min_max)                 (
		struct _ia_image_t*, /** self */
		ia_int32_t  *,       /* return min color */
		ia_uint32_t *        /* return max color */
	);

	/** calculates image histogram */
	ia_signal_p (*histogram)            (
		struct _ia_image_t*,  /** self */
		ia_color_element_t    /** color element */
	);

	/** create signal from image line  */
	ia_signal_p (*line_to_signal)       (
		struct _ia_image_t*,  /** self */
		ia_int32_t x1,
		ia_int32_t y1,
		ia_int32_t x2,
		ia_int32_t y2
	);

	/** binarize image by threshold */
	void (*binarize_threshold)          (
		struct _ia_image_t*, /** self */
		ia_int32_t           /** threshold value */
	);

	/** binarize image by 2 thresholds */
	void (*binarize_threshold_2)        (
		struct _ia_image_t*, /** self */
		ia_int32_t,          /** threshold value 1 */
		ia_int32_t           /** threshold value 2 */
	);

	/** binarize image by otsu */
	int  (*binarize_otsu)               (
		struct _ia_image_t*,  /** self */
		ia_signal_p histogram /** image histogram */
	);

	/** binarize image by otsu by 2 thresholds */
	int  (*binarize_otsu_2)             (
		struct _ia_image_t*,  /** self */
		ia_signal_p histogram /** image histogram */
	);

	/** allocate memory and copy this image */
	struct _ia_image_t* (*copy)         (
		struct _ia_image_t* /** self */
	);

	/** draw line */
	void (*draw_line)                   (
		struct _ia_image_t*, /** self */
		ia_int32_t x1,
		ia_int32_t y1,
		ia_int32_t x2,
		ia_int32_t y2,
		ia_uint32_t color
	);

	/** destroy image object */
	void (*destroy)                     (
		struct _ia_image_t* /** self */
	);

	/** print binarized image to the console */
	void (*print)                       (
		struct _ia_image_t* /** self */
	);

} ia_image_t, *ia_image_p;


/** load an image from file                */
IA_API ia_image_p ia_image_load   (
	const ia_string_t /** file name        */
);

/** creates new image */
IA_API ia_image_p ia_image_new    (
	ia_uint16_t, /** image width            */
	ia_uint16_t, /** image height           */
	ia_format_t, /** image format           */
	ia_bool_t    /** 1 if the image represents gray pixels */
);

/** creates new image from user data */
IA_API ia_image_p ia_image_from_data    (
	ia_uint16_t, /** image width            */
	ia_uint16_t, /** image height           */
	ia_format_t, /** image format           */
	ia_bool_t,   /** 1 if the image represents gray pixels */
	void*,
	ia_uint32_t
);

#endif /* __IA_IMAGE_H */
