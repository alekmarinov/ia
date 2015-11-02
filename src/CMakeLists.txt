# src/CMakeLists.txt
SET(${PROJECT_NAME}_SOURCES
	ia_bezier.c
	ia_common.c
	ia_gif.c
	ia_image.c
	ia_jpeg.c
	ia_line.c
	ia_signal.c
	ia_tiff.c
	ia_vector.c
	algo/ia_binarize.c
	algo/ia_contours.c
	algo/ia_convolution.c
	algo/ia_distance_transform.c
	algo/ia_fft.c
	algo/ia_morphology.c
	algo/ia_otsu.c
)

ADD_LIBRARY(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})
INSTALL_TARGETS(/lib ${PROJECT_NAME})
