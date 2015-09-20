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
    // Declarations
    mqd_t                   mq_fd_request;
    mqd_t                   mq_fd_response;
    MQ_REQUEST_MESSAGE      req;
    MQ_RESPONSE_MESSAGE     rsp;
    struct mq_attr attr; 

    // Gets the message queue names from the (command line) arguments
    mq_name1 = argv[1];
    mq_name2 = argv[2];
    
    // Opens the request and response queue 
    mq_fd_request = mq_open(mq_name1, O_RDONLY);
    mq_fd_response = mq_open(mq_name2, O_WRONLY);

    if (mq_fd_request < 0) perror("Worker could not open request queue");
    if (mq_fd_response < 0) perror("Worker could not open response queue"); 

    int fail_counter = 0;
    // Repeatingly:
    do { 
        // Read from a message queue the new job to do
        ssize_t bytes_read = mq_receive(mq_fd_request, (char *) &req, sizeof(req), NULL); 
        
        // Tests if a message has been read
        if (bytes_read < sizeof(req)){
            fail_counter++;
            usleep(100);
            continue;
        } else {
            fail_counter = 0;
        } 
        
        // Wait a random amount of time
        rsleep(10000);
        
        // Compute the madelbrot points and create response
        int i;
        for (i = 0; i < X_PIXEL; i++) {
            rsp.y = req.y;
            rsp.k[i] = mandelbrot_point(X_LOWERLEFT + (i * STEP), Y_LOWERLEFT + (req.y * STEP));
        } 
        
        // Wait until there is room in the response queue
        do {
            mq_getattr(mq_fd_response, &attr);
        } while (attr.mq_curmsgs >= MQ_MAX_MESSAGES);

        // Write the results to the reponse queue
        int code = mq_send(mq_fd_response, (char *) &rsp, sizeof(rsp), 0); 
        if (code < 0) perror("Worker could not send response");
        
    }  while (fail_counter < 10);

    // Close the message queues
    mq_close(mq_fd_request);
    mq_close(mq_fd_response);

    return (0);
}
