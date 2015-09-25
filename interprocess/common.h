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
    // a data structure for one row
    int                         y;
} MQ_REQUEST_MESSAGE;

typedef struct
{
    // a data structure for the color of pixels of one row
    int                         y;
    int                         k[X_PIXEL];
} MQ_RESPONSE_MESSAGE;

#endif
