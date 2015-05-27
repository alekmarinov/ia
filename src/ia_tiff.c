/*********************************************************************/
/*                                                                   */
/* Copyright (C) 2005, Alexander Marinov, Nadezhda Zlateva           */
/*                                                                   */
/* Project:       ia                                                 */
/* Filename:      ia_tiff.c                                          */
/* Description:   Load/Save TIFF image format                        */
/*                                                                   */
/*********************************************************************/

#ifdef HAVE_TIFFLIB

#include <ia/ia_image.h>
#include <tiffio.h>
#include <string.h>
#include <malloc.h>

ia_image_p ia_image_load_tiff(const ia_string_t src)
{
	ia_image_p img;
	uint32 rowsperstrip = (uint32) -1;
	TIFF *in;
	uint32 w, h;
	uint16 samplesperpixel;
	uint16 bitspersample;
	uint16 config;
	uint16 photometric;
	uint32 *inbuf;
	void* imgbuf;
	unsigned char *pbuf;
	uint32 outlinesize;
	ia_format_t format = IAT_UINT_32;
	int imgtype = IA_IMAGE_RGB;
	int row;

	in = TIFFOpen(src, "r");
	if (!in)
	{
		return 0;
	}
	photometric = 0;
	TIFFGetField(in, TIFFTAG_PHOTOMETRIC, &photometric);
	/*
	if (photometric != PHOTOMETRIC_RGB && photometric != PHOTOMETRIC_PALETTE ) {
		fprintf(stderr, "Bad photometric; can only handle RGB and Palette images, got %d\n", photometric);
		return 0;
	}
	*/
	TIFFGetField(in, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
	if (samplesperpixel != 1 && samplesperpixel != 3) {
		fprintf(stderr, "Bad samples/pixel %u.\n", samplesperpixel);
		return 0;
	}
	TIFFGetField(in, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	if (bitspersample != 8 && bitspersample != 16 && bitspersample != 32) {
		fprintf(stderr, "Sorry, only handle 8, 16 or 32-bit samples, got %d\n", bitspersample);
		return 0;
	}
	
	if (samplesperpixel == 1)
	{
		imgtype = IA_IMAGE_GRAY;
		switch (bitspersample)
		{
		case 8:
			format = IAT_UINT_8;
		break;
		case 16:
			format = IAT_UINT_16;
		break;
		case 32:
			format = IAT_UINT_32;
		break;
		}
	}

	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &h);

	outlinesize = w * ia_format_size(format) >> 3;
	imgbuf = malloc(h * outlinesize);
	pbuf = (unsigned char *)imgbuf;

	inbuf = (uint32*) _TIFFmalloc(TIFFScanlineSize(in));
	if (!inbuf)
	{
		TIFFClose(in);
		return NULL;
	}
	printf("%d %d %d\n", format, imgtype, photometric);
	for (row=0; row<h; row++)
	{
		if (TIFFReadScanline(in, inbuf, row, 0) < 0)
		{
			TIFFClose(in);
			return NULL;
		}
		/*printf("reading line %d of %d from size %d to %d\n", row, h, TIFFScanlineSize(in), outlinesize);*/
		memcpy(pbuf, inbuf, outlinesize);
		pbuf += outlinesize;
	}

	TIFFClose(in);
	img = ia_image_from_data(w, h, format, imgtype, imgbuf, h * outlinesize);
	img->is_user_data = 0;
	return img;
}

void ia_image_save_tiff(ia_image_p image, const ia_string_t dst)
{
	uint16 bitspersample = ia_format_size(image->format);
	uint16 samplesperpixel = image->is_gray?1:3;
	unsigned char *outbuf = (unsigned char *)image->pixels.data;
	uint32 bufsize = image->width * bitspersample >> 3;
	int row;

	TIFF *out = TIFFOpen(dst, "w");
	if (!out)
	{
		return ;
	}
	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, image->width);           /* set the width of the image */
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, image->height);         /* set the height of the image */
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);   /* set number of channels per pixel */
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, bitspersample);       /* set the size of the channels */
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);   /* set the origin of the image. */
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);       /* RGB image */

	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, bufsize));
	printf("writing bitspersample=%d, samplesperpixel=%d\n", bitspersample, samplesperpixel);
	for (row=0; row < image->height; row++)
	{
		if (TIFFWriteScanline(out, outbuf, row, 0) < 0)
			break;
		outbuf += bufsize;
	}

	TIFFClose(out);
}

#endif /* HAVE_TIFFLIB */

