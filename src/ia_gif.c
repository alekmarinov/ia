/*********************************************************************/
/*                                                                   */
/* Copyright (C) 2005, Alexander Marinov, Nadezhda Zlateva           */
/*                                                                   */
/* Project:       ia                                                 */
/* Filename:      ia_gif.c                                           */
/* Description:   Load/Save GIF image format                         */
/*                                                                   */
/*********************************************************************/

#define MaxBufferExtent  8192

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <ia/ia_image.h>

static ia_bool_t ia_gif_read_byte(FILE* fp, ia_int8_t *b)
{
	return (fread(b, 1, sizeof(ia_int8_t), fp) == sizeof(ia_int8_t));
}

static ia_bool_t ia_gif_read_short(FILE* fp, ia_int16_t *s)
{
	return (fread(s, 1, sizeof(ia_int16_t), fp) == sizeof(ia_int16_t));
}

static ia_bool_t ia_gif_read_long(FILE* fp, ia_uint32_t *l)
{
	return (fread(l, 1, sizeof(ia_uint32_t), fp) == sizeof(ia_uint32_t));
}

static ia_uint32_t ia_gif_read_buffer(FILE* fp, ia_uint8_t *buf, ia_uint32_t count)
{
	return (ia_uint32_t)fread(buf, 1, count, fp);
}

static ia_uint32_t ia_gif_read_block(FILE* fp, ia_uint8_t *buf)
{
	ia_uint8_t block_count;
	if (!ia_gif_read_byte(fp, &block_count)) return 0;
	return (ia_uint32_t)fread(buf, 1, block_count, fp);
}

typedef struct 
{
	ia_uint8_t red, green, blue;
} ia_color_rgb_t;

typedef struct 
{
	ia_int16_t rows, columns;
	ia_bool_t interlaced;
	ia_int32_t colors;
	ia_uint16_t page_width,
		page_height,
		page_x,
		page_y;

	ia_int32_t delay;
	ia_int32_t dispose;
	ia_int32_t iterations;
	ia_bool_t matte;

	ia_color_rgb_t* colormap;
	ia_color_rgb_t background_color;
} image_info_t;


static ia_bool_t ia_gif_decode(FILE* fp, image_info_t* image_info, ia_image_p img, ia_int32_t opacity)
{
#define MaxStackSize  4096
#define NullCode  (~0UL)

	ia_int32_t index;
	ia_int32_t offset, y;
	ia_int32_t x;
	ia_color_rgb_t q;
	ia_uint8_t *c;
	ia_uint32_t datum;
	ia_int16_t *prefix;
	ia_uint32_t count;
	ia_uint8_t
		*packet,
		*pixel_stack,
		*suffix,
		*top_stack;
	ia_uint32_t
		available,
		bits,
		clear,
		code,
		code_mask,
		code_size,
		data_size,
		first,
		end_of_information,
		in_code,
		old_code,
		pass;

	/* Allocate decoder tables. */

	packet=(ia_uint8_t*)malloc(256);
	prefix=(ia_int16_t*)malloc(MaxStackSize*sizeof(*prefix));
	suffix=(ia_uint8_t*)malloc(MaxStackSize);
	pixel_stack=(ia_uint8_t*)malloc(MaxStackSize+1);
	if ((!packet) ||
		(!prefix) ||
		(!suffix) ||
		(!pixel_stack))
	{
		if (packet) free(packet);
		if (prefix) free(prefix);
		if (suffix) free(suffix);
		if (pixel_stack) free(pixel_stack);
		/* not enough memory */
		return 0;
	}
	/* Initialize GIF data stream decoder.*/
	data_size=0;
	if (!ia_gif_read_byte(fp, (ia_uint8_t*)&data_size))
	{
		/* invalid file size */
		return 0;
	}
	if (data_size > 8)
	{
		/* corrupt image format */
		return 0;
	}
	clear=1UL << data_size;
	end_of_information=clear+1;
	available=clear+2;
	old_code=NullCode;
	code_size=data_size+1;
	code_mask=(1 << code_size)-1;
	for (code=0; code < clear; code++)
	{
		prefix[code]=0;
		suffix[code]=(ia_uint8_t) code;
	}

	/* Decode GIF pixel stream. */
	datum=0;
	bits=0;
	c=0;
	count=0;
	first=0;
	offset=0;
	pass=0;
	top_stack=pixel_stack;
	for (y=0; y < (ia_int32_t) image_info->rows; y++)
	{
		if ((ia_uint32_t)(y*image_info->columns) >= img->pixels.size)
		{
			break;
		}

		//indexes=GetIndexes(image); // ????
		for (x=0; x < (ia_int32_t) image_info->columns; )
		{
			if (top_stack == pixel_stack)
			{
				if (bits < code_size)
				{
					/* Load bytes until there is enough bits for a code. */
					if (count == 0)
					{
						/* Read a new data block. */
						count=ia_gif_read_block(fp, packet);
						if (count == 0) break;
						c=packet;
					}
					datum+=(ia_uint32_t) (*c) << bits;
					bits+=8;
					c++;
					count--;
					continue;
				}
				/* Get the next code. */
				code = datum & code_mask;
				datum >>= code_size;
				bits -= code_size;
				/* Interpret the code */
				if ((code > available) || (code == end_of_information)) break;
				if (code == clear)
				{
					/* Reset decoder. */
					code_size=data_size+1;
					code_mask=(1 << code_size)-1;
					available=clear+2;
					old_code=NullCode;
					continue;
				}
				if (old_code == NullCode)
				{
					*top_stack++=suffix[code];
					old_code=code;
					first=code;
					continue;
				}
				in_code=code;
				if (code >= available)
				{
					*top_stack++=(ia_uint8_t) first;
					code=old_code;
				}
				while (code >= clear)
				{
					if ((top_stack-pixel_stack) >= MaxStackSize) break;
					*top_stack++ = suffix[code];
					code=(ia_uint32_t) prefix[code];
				}
				first=(ia_uint32_t) suffix[code];

				/* Add a new string to the string table, */
				if ((top_stack-pixel_stack) >= MaxStackSize) break;
				if (available >= MaxStackSize) break;
				*top_stack++=(ia_uint8_t) first;
				prefix[available]=(ia_int16_t) old_code;
				suffix[available]=(ia_uint8_t) first;
				available++;
				if (((available & code_mask) == 0) && (available < MaxStackSize))
				{
					code_size++;
					code_mask+=available;
				}
				old_code=in_code;
			}
			/* Pop a pixel off the pixel stack. */
			top_stack--;
			index=*top_stack; //???
			//index=ConstrainColormapIndex(image, *top_stack); // ???
			//indexes[x]=index;
			q=image_info->colormap[index];
			img->set_pixel(img, x, y, IA_RGB(q.red, q.green, q.blue));
			//q->opacity=(Quantum)(index == opacity ? TransparentOpacity : OpaqueOpacity);
			x++;
			//q++;
		}
		if (!image_info->interlaced) offset++;
		else
		{
			switch (pass)
			{
				case 0:
				default:
				{
					offset+=8;
					if (offset >= (ia_int32_t) image_info->rows)
					{
						pass++;
						offset=4;
					}
					break;
				}
				case 1:
				{
					offset+=8;
					if (offset >= (ia_int32_t) image_info->rows)
					{
						pass++;
						offset=2;
					}
					break;
				}
				case 2:
				{
					offset+=4;
					if (offset >= (ia_int32_t) image_info->rows)
					{
						pass++;
						offset=1;
					}
					break;
				}
				case 3:
				{
					offset+=2;
					break;
				}
			}
		}
		//if (SyncImagePixels(image) == MagickFalse)
		//  break;
		if (x < (ia_int32_t) image_info->columns) break;
	}
	free(pixel_stack);
	free(suffix);
	free(prefix);
	free(packet);
	if (y < (long) image_info->rows)
	{
		/* corrupt image format */
		return 0;
	}
	return 1;
}


ia_image_p ia_image_load_gif(const ia_string_t src)
{
#define BitSet(byte,bit)  (((byte) & (bit)) == (bit))
#define LSBFirstOrder(x,y)  (((y) << 8) | (x))

	FILE *fp;
	ia_image_p img;
	ia_uint8_t magick[12],
		flag, 
		background, 
		c;
	ia_int32_t nRead;
	ia_int16_t page_width, 
		page_height, 
		page_x, 
		page_y;
	ia_uint8_t *global_colormap;
	ia_uint32_t count;
	ia_int32_t opacity;
	ia_uint32_t delay,
		dispose,
		global_colors,
		image_count,
		iterations;
	ia_bool_t error;
	ia_uint8_t header[256];
	ia_int32_t depth;
	ia_uint8_t* p;
	ia_int32_t i;
	
	image_info_t image_info;

	fp=fopen(src, "rb");
	if (!fp)
	{
		return 0;
	}

	/* check if this is a GIF image */

	nRead=(ia_int32_t)ia_gif_read_buffer(fp, magick, 6);
	if (nRead != 6 || (strncmp(magick, "GIF87", 5) != 0 && strncmp(magick, "GIF89", 5) != 0))
	{
		/* invalid magick string */
		return 0;
	}

	if (!ia_gif_read_short(fp, &page_width) || (!ia_gif_read_short(fp, &page_height)))
	{
		/* invalid file size */
		return 0;
	}

	if (!ia_gif_read_byte(fp, &flag) || (!ia_gif_read_byte(fp, &background)) 
		|| (!ia_gif_read_byte(fp, &c)) /* reserved */ )
	{
		/* invalid file size */
		return 0;
	}

	global_colors=1 << (((ia_uint32_t) flag & 0x07)+1);
	global_colormap=(ia_uint8_t *)malloc((size_t) (3*MAX(global_colors, 256)));
	if (!global_colormap)
	{
		/* not enough memory */
		return 0;
	}

	if (BitSet((ia_int32_t) flag, 0x80) != 0)
	{
		count=ia_gif_read_buffer(fp, global_colormap, (ia_uint32_t) (3*global_colors));
	}

	delay=0;
	dispose=0;
	iterations=1;
	opacity=(-1);
	image_count=0;
	error=0;

	for ( ; ; )
	{
		if (!ia_gif_read_byte(fp,&c)) break; /* eof */
		if (c == (ia_uint8_t) ';') break;  /* terminator */
		if (c == (ia_uint8_t) '!')
		{
			/*
			  GIF Extension block.
			*/
			if (!ia_gif_read_byte(fp,&c)) { error = 1; break; } /* invalid image format */
			switch (c)
			{
				case 0xF9:
				{
					/* Read graphics control extension. */
					while (ia_gif_read_block(fp, header) != 0);
					dispose=(ia_uint32_t) (header[0] >> 2);
					delay=(ia_uint32_t) ((header[2] << 8) | header[1]);
					if ((ia_int32_t) (header[0] & 0x01) == 0x01)
						opacity=(ia_int32_t) header[3];
					break;
				}
				case 0xFE:
				{
					/* Skip comment extension. */
					for ( ; ; )
					{
					  count=(ia_uint32_t) ia_gif_read_block(fp, header);
					  if (count == 0) break;
					}
					break;
				}
				case 0xFF:
				{
					ia_bool_t loop;

					/* Skip Netscape Loop extension. */
					loop=0;
					if (ia_gif_read_block(fp, header) != 0)
					  loop=strncmp(header, "NETSCAPE2.0", 11) == 0;
					while (ia_gif_read_block(fp, header) != 0)
						if (loop != 0)
							iterations=(ia_uint32_t) ((header[2] << 8) | header[1]);
					break;
				}
				default:
				{
					while (ia_gif_read_block(fp, header) != 0);
					break;
				}
			}
		}
		if (c != (ia_uint8_t) ',') continue;
		image_count++;
		/* Read image attributes. */
		if (!ia_gif_read_short(fp, &page_x) 
			|| !ia_gif_read_short(fp, &page_y) 
			|| !ia_gif_read_short(fp, &image_info.columns) 
			|| !ia_gif_read_short(fp, &image_info.rows)
			|| !ia_gif_read_byte(fp, &flag))
		{
			/* invalid file size */
			return 0;
		}
		depth=8;

		image_info.interlaced=BitSet((ia_int32_t) flag,0x40) != 0;
		image_info.colors=BitSet((ia_int32_t) flag, 0x80) == 0 ? global_colors : 1UL << ((ia_uint32_t) (flag & 0x07)+1);
		if (opacity >= (ia_int32_t) image_info.colors)
			image_info.colors=(ia_uint32_t) opacity+1;
		image_info.page_width=page_width;
		image_info.page_height=page_height;
		image_info.page_y=page_y;
		image_info.page_x=page_x;
		image_info.delay=delay;
		image_info.dispose=dispose;
		image_info.iterations=iterations;
		image_info.matte=(opacity >= 0);
		delay=0;
		dispose=0;
		iterations=1;
		if ((image_info.columns == 0) || (image_info.rows == 0))
		{
			/* zero image size */
			return 0;
		}

		/* allocate 32-bit image object */
		img = ia_image_new(image_info.columns, image_info.rows, IAT_UINT_32, IA_IMAGE_RGB);

		/* Inititialize colormap. */
		image_info.colormap=(ia_color_rgb_t*)malloc(image_info.colors * sizeof(ia_color_rgb_t));
		if (!image_info.colormap)
		{
			/* not enough memory */
			return 0;
		}

		if (BitSet((ia_int32_t) flag, 0x80) == 0)
		{
			/* Use global colormap. */
			p=global_colormap;
			for (i=0; i < (ia_int32_t) image_info.colors; i++)
			{
				image_info.colormap[i].red=*p++;
				image_info.colormap[i].green=*p++;
				image_info.colormap[i].blue=*p++;
			}
			image_info.background_color=image_info.colormap[MIN((ia_uint32_t)background, (ia_uint32_t)(image_info.colors-1))];
		}
		else
		{
			ia_uint8_t *colormap;

			/* Read local colormap. */
			colormap=(ia_uint8_t *)malloc(3*image_info.colors);
			if (!colormap)
			{
				/* not enough memory */
				return 0;
			}
			count=ia_gif_read_buffer(fp, colormap, 3*image_info.colors);
			if (count != 3*image_info.colors)
			{
				/* invalid file size */
				return 0;
			}
			p=colormap;
			for (i=0; i < (ia_int32_t) image_info.colors; i++)
			{
				image_info.colormap[i].red=*p++;
				image_info.colormap[i].green=*p++;
				image_info.colormap[i].blue=*p++;
			}
			free(colormap);
		}

		/* Decode image. */
		if (!ia_gif_decode(fp, &image_info, img, opacity))
		{
			/* corrupt image format */
			free(global_colormap);
			return 0;
		}
	}
	free(global_colormap);
	if ((image_info.columns == 0) || (image_info.rows == 0))
	{
		/* zero image size */
		return 0;
	}
	fclose(fp);
	return img;
}
