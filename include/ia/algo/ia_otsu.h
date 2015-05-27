/*********************************************************************/
/*                                                                   */
/* Copyright (C) 2007, Alexander Marinov, Nadezhda Zlateva           */
/*                                                                   */
/* Project:       ia                                                 */
/* Filename:      ia_otsu.h                                          */
/* Description:   Histogram Multi Thresholding                       */
/*                based on the Otsu's method for Binarization        */
/*                                                                   */
/*********************************************************************/

#ifndef __IA_OTSU_H
#define __IA_OTSU_H

#include <ia/ia_signal.h>

/*********************************************************************/
/*                 Otsu API interface                                */
/*********************************************************************/

/* 
 * Determines a threshold in a histogram
 * 1 - success, < 0  - error
 */
ia_int32_t IA_API ia_otsu(
						ia_signal_p,  /* input signal         */
						ia_uint32_t*, /* output treshold      */
						ia_uint32_t*, /* output mean          */
						ia_uint32_t*, /* output mean black    */
						ia_uint32_t*, /* output mean white    */
						ia_double_t*, /* output area black    */
						ia_double_t*, /* output area white    */
						ia_double_t*  /* output max criterion */
);

/* 
 * Determines 2 thresholds in a histogram
 * 1 - success, < 0  - error
 */
ia_int32_t IA_API ia_otsu_2(
						ia_signal_p,  /* input signal         */
						ia_uint32_t*, /* output treshold 1    */
						ia_uint32_t*, /* output treshold 2    */
						ia_uint32_t*, /* output mean          */
						ia_uint32_t*, /* output mean black    */
						ia_uint32_t*, /* output mean grey     */
						ia_uint32_t*, /* output mean white    */
						ia_double_t*, /* output area black    */
						ia_double_t*, /* output area grey     */
						ia_double_t*, /* output area white    */
						ia_double_t*  /* output max criterion */
);

#endif
