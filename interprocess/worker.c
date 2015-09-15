/* 
 * Operating Systems  (2INC0)  Practical Assignment
 * Interprocess Communication
 *
 * Mark Bouwman (0868533)
 * Martin ter Haak (0846351)
 *
 * Grading:
 * Students who hand in clean code that fully satisfies the minimum requirements will get an 8. 
 * ”Extra” steps can lead to higher marks because we want students to take the initiative. 
 * Extra steps can be, for example, in the form of measurements added to your code, a formal 
 * analysis of deadlock freeness etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>          // for perror()
#include <unistd.h>         // for getpid()
#include <mqueue.h>         // for mq-stuff
#include <time.h>           // for time()
#include <complex.h>

#include "settings.h"
#include "common.h"

static char *mq_name1;
static char *mq_name2;
    
static double 
complex_dist (complex a)
{
    // distance of vector 'a'
    // (in fact the square of the distance is computed...)
    double re, im;
    
    re = __real__ a;
    im = __imag__ a;
    return ((re * re) + (im * im));
}

static int 
mandelbrot_point (double x, double y)
{
    int     k;
    complex z;
    complex c;

    z = x + y * I;     // create a complex number 'z' from 'x' and 'y'
    c = z;

    for (k = 0; (k < MAX_ITER) && (complex_dist (z) < INFINITY); k++)
    {
        z = z * z + c;
    }
    
    // k >= MAX_ITER or | z | >= INFINITY
    
    return (k);
}


/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time(NULL) % getpid());
        first_call = false;
    }
    usleep (random () % t);
}


int main (int argc, char * argv[])
{
    mqd_t                   mq_fd_request;
    mqd_t                   mq_fd_response;
    MQ_REQUEST_MESSAGE      req;
    MQ_RESPONSE_MESSAGE     rsp;
    
    mq_name1 = argv[1];
    mq_name2 = argv[2];
    
    printf("mq_name1 = %s", mq_name1);
    printf("mq_name2 = %s", mq_name2);
    
    //Open the two message queues
    mq_fd_request = mq_open(mq_name1, O_RDONLY);
    mq_fd_response = mq_open(mq_name2, O_WRONLY);

    //Repeatingly:
    while (1){
        //Read from a message queue the new job to do
        ssize_t bytes_read = mq_receive(mq_fd_request, (char *) &req, sizeof(req), NULL);
        
        //  until there are no more jobs to do
        if (bytes_read < 1) {
            printf("No more jobs\n");
            break;
        }

        printf("Received: x=%f, y=%f\n", req[0].x, req[0].y);

        //Wait a random amount of time
        rsleep(500000);

        //Compute the madelbrot points
        for (i = 0; i < X_PIXEL; i++) {
            rsp[i].k = mandelbrot_point(req[i].x, req[i].y);
        } 
        
        //Write the results to a message queue
        mq_send(mq_fd_response, (char *) &rsp, sizeof(rsp), 0); 
        
        printf("Send: k=%d\n", rsp[0].k);
    } 
    
    //Close the message queues
    mq_close(mq_fd_request);
    mq_close(mq_fd_response);

    return (0);
}


