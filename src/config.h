#ifndef BRT_CONFIG_H
#define BRT_CONFIG_H

#ifdef _MSC_VER
#  pragma warning(disable: 4996)  /* C4996: This function or variable may be unsafe. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. */
#endif

/**
 * Tell arxlib to use Qt for image io
 */
#define ARX_USE_QT_IMAGE_IO

/**
 * Minimal number of keypoints for a valid image.
 */
#define MIN_KEYPOINTS_PER_IMAGE 4

/**
 * Minimal number of matched keypoints for a valid match.
 */
#define MIN_MATCHES 8

/**
 * Maximal number of matches for Levenberg-Marquardt.
 */
#define MAX_MATCHES 50

/**
 * Initial smoothness of an input image in terms of standard deviation of gaussian filter.
 */
#define INITIAL_SMOOTHNESS 0.5

/**
 * Default maximal size parameter for kpextract and kpmatch.
 */
#define DEFAULT_MAX_SIZE_X 1024
#define DEFAULT_MAX_SIZE_Y 1024

/**
 * Default maximal and minimal number of iterations for barcode recognition.
 */
#define DEFAULT_MIN_ITERATIONS 16
#define DEFAULT_MAX_ITERATIONS 512

/**
 * Barcode recognition toolset version
 */
#define BRT_VERSION "0.1.6"

#endif // BRT_CONFIG_H
