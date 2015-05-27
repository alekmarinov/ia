/*********************************************************************/
/*                                                                   */
/* Copyright (C) 2007, Alexander Marinov, Nadezhda Zlateva           */
/*                                                                   */
/* Project:       ia                                                 */
/* Filename:      ia_signal.h                                        */
/* Description:   Signal management module                           */
/*                                                                   */
/*********************************************************************/

#ifndef __IA_SIGNAL_H
#define __IA_SIGNAL_H

#include <ia/ia.h>

/*********************************************************************/
/*                              Signal API                           */
/*********************************************************************/

/**
	Type ia_signal_t

	Defines set ot functions for primitive signal manipulations
*/
typedef struct _ia_signal_t
{
	/** signal length */
	ia_uint16_t                         length;

	/** signal pixel format */
	ia_format_t                         format;

	/** true if the signal is gray */
	ia_bool_t                           is_gray;

	/** raw signal data */
	ia_data_t                           pixels;

	/** set a pixel color at specified position */
	void (*set_pixel)                   (
		struct _ia_signal_t*, /** self */
		ia_uint16_t,          /** x coordinate */
		ia_uint32_t           /** color value with bit size depending of the signal pixel format */
	);

	/** get a pixel color from specified position */
	ia_uint32_t (*get_pixel)            (
		struct _ia_signal_t*, /** self */
		ia_uint16_t           /** x coordinate */
	);

	/** fill the signal with specified color */
	void (*fill)                        (
		struct _ia_signal_t*, /** self */
		ia_uint32_t           /** color value with bit size depending of the signal pixel format */
	);

	/** convert an image into 32-bit ARGB pixel format, where the alpha mask is 0 */
	struct _ia_signal_t* (*convert_rgb)  (
		struct _ia_signal_t*  /** self */
	);

	/** convert an image to gray */
	struct _ia_signal_t* (*convert_gray) (
		struct _ia_signal_t*, /** self */
		ia_format_t           /** pixel format */
	);
	
	/** normalize pixel colors to occupy better the specified range */
	void (*normalize_colors)            (
		struct _ia_signal_t*, /** self */
		ia_int32_t,           /** from min color or 0 if unknown */
		ia_uint32_t,          /** from max color or 0 if unknown */
		ia_int32_t,           /** to min color */
		ia_uint32_t           /** to max color */
	);

	/** inverse */
	void (*inverse)                     (
		struct _ia_signal_t*  /** self */
	);

	/** multiply signal by number */
	void (*multiply_number)             (
		struct _ia_signal_t*,  /** self */
		ia_double_t            /** multiplier */
	);

	/** add number to this signal */
	void (*add_number)                  (
		struct _ia_signal_t*,  /** self */
		ia_double_t            /** number to add */
	);

	/** add signal to this signal */
	void (*add_signal)                  (
		struct _ia_signal_t*,  /** self */
		struct _ia_signal_t*   /** signal to add */
	);

	/** sum signal values */
	ia_int32_t (*sum)                   (
		struct _ia_signal_t*   /** self */
	);

	/** linear combination */
	ia_int32_t (*linear_combination)    (
		struct _ia_signal_t*,  /** self */
		struct _ia_signal_t*   /** combiner */
	);

	/** determines optimal threshold by otsu */
	ia_int32_t (*threshold_otsu)        (
		struct _ia_signal_t*,  /** self */
		ia_uint32_t*,          /** output threshold */
		ia_uint32_t*,          /** output mean */
		ia_uint32_t*,          /** output mean black */
		ia_uint32_t*,          /** output mean white */
		ia_double_t*,          /** output area black */
		ia_double_t*,          /** output area white */
		ia_double_t*           /** output max criterion */
	);

	/** determines 2 optimal thresholds by otsu */
	ia_int32_t (*threshold_otsu_2)      (
		struct _ia_signal_t*,  /** self */
		ia_uint32_t*,          /** output threshold 1 */
		ia_uint32_t*,          /** output threshold 2 */
		ia_uint32_t*,          /** output mean */
		ia_uint32_t*,          /** output mean black */
		ia_uint32_t*,          /** output mean grey */
		ia_uint32_t*,          /** output mean white */
		ia_double_t*,          /** output area black */
		ia_double_t*,          /** output area grey */
		ia_double_t*,          /** output area white */
		ia_double_t*           /** output max criterion */
	);

	/** determines min and max colors in the signal */
	void (*get_min_max)                 (
		struct _ia_signal_t*, /** self */
		ia_int32_t  *,        /** return min color */
		ia_uint32_t *         /** return max color */
	);
	
	/** shifts the singnal values to the given direction */
	void (*shift)                       (
		struct _ia_signal_t*, /** self */
		ia_int32_t            /** direction to shift, > 0 shift right */
	);

	/** allocate memory and copy this signal */
	struct _ia_signal_t* (*copy)        (
		struct _ia_signal_t* /** self */
	);

	/** destroy image object */
	void (*destroy)                     (
		struct _ia_signal_t* /** self */
	);

} ia_signal_t, *ia_signal_p;

/** creates new signal */
IA_API ia_signal_p ia_signal_new    (
	ia_uint16_t, /** signal length          */
	ia_format_t, /** pixel format           */
	ia_bool_t    /** 1 if the signal represents gray pixels */
);

#endif
