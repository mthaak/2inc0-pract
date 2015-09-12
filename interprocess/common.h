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
} MQ_REQUEST_MESSAGE;

typedef struct
{
    // a data structure with the k value of the mandelbrot point
    int                     k;
} MQ_RESPONSE_MESSAGE;


#endif

