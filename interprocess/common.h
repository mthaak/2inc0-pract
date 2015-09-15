/* 
 * Operating Systems  (2INC0)  Practical Assignment
 * Interprocess Communication
 *
 * Contains definitions which are commonly used by the farmer and the workers
 *
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include "settings.h"


// TODO: put your definitions of the datastructures here

typedef struct
{
    // a data structure with x and y coordinates
    double                     x;
    double                     y;
} PIXEL_COORDINATE;

typedef struct
{
    // a data structure with x and y coordinates for one row
    PIXEL_COORDINATE           coordinates[X_PIXEL];
} MQ_REQUEST_MESSAGE;

typedef struct
{
    // a data structure with the k value of the mandelbrot point for one row of pixels
    int                        colors[X_PIXEL];
} MQ_RESPONSE_MESSAGE;


#endif

