# lib/sci/ia/src/ia.mak

TARGET=ia
VERSION=1.1
OBJS=ia_bezier.o ia_common.o ia_gif.o ia_image.o ia_signal.o \
     ia_jpeg.o ia_tiff.o ia_line.o ia_vector.o \
     algo/ia_binarize.o algo/ia_contours.o \
     algo/ia_convolution.o algo/ia_distance_transform.o \
     algo/ia_fft.o algo/ia_morphology.o algo/ia_otsu.o
EXTRA_INCS=-I../include
EXTRA_DEFS=-DHAVE_JPEGLIB -DHAVE_TIFFLIB
EXTRA_LIBS=-ljpeg -ltiff
