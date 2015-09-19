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
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>    
#include <unistd.h>         // for execlp
#include <mqueue.h>         // for mq

#include "settings.h"
#include "output.h"
#include "common.h"

static char mq_name1[80];
static char mq_name2[80];

void create_workers(){
    int i;
    for (i = 0; i < NROF_WORKERS; i++){
        pid_t workerPID;
        workerPID = fork();
        if (workerPID < 0){
            perror("fork() failed");
            exit(1);
        } else if (workerPID == 0) { 
            execlp("./worker", "worker", mq_name1, mq_name2, NULL);

            perror("execlp() failed");
        } 
    }
}


int main (int argc, char * argv[])
{
    if (argc != 1)
    {
        fprintf (stderr, "%s: invalid arguments\n", argv[0]);
    }
        
    //output_init();

    //Create the message queues & the children
    mqd_t               mq_fd_request;
    mqd_t               mq_fd_response;
    MQ_REQUEST_MESSAGE  req;
    MQ_RESPONSE_MESSAGE rsp;
    struct mq_attr      attrReq;
    struct mq_attr      attrRsp;
    
    sprintf (mq_name1, "/mq_request_%s_%d", "MarkMartin", getpid());
    sprintf (mq_name2, "/mq_response_%s_%d", "MarkMartin", getpid());
    printf("mq_name1 = ");
    //printf(mq_name1);
    printf("\n");
    printf("mq_name2 = ");
    //printf(mq_name2);
    printf("\n");

    attrReq.mq_maxmsg  = MQ_MAX_MESSAGES;
    attrReq.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE); 
    mq_fd_request = mq_open (mq_name1, O_WRONLY | O_CREAT | O_EXCL, 0600, &attrReq);
    
    attrRsp.mq_maxmsg  = MQ_MAX_MESSAGES;
    attrRsp.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);
    mq_fd_response = mq_open (mq_name2, O_RDONLY | O_CREAT | O_EXCL, 0600, &attrRsp);

    if (mq_fd_request > -1) printf("Request queue opened\n");
    else perror("Request queue could not be opened");
    if (mq_fd_response > -1) printf("Response queue opened\n");
    else perror("Response queue could not be opened");
    
    create_workers();

    int total_send = 0;
    int total_received = 0;
    int j = 0;
    //while (total_received < Y_PIXEL)
    while(j < 10)
    {
        printf("doing");
        mq_getattr(mq_fd_response, &attrRsp);
        mq_getattr(mq_fd_request, &attrReq);

        //printf("Messages in queue, %d, %d\n", (int) attrReq.mq_curmsgs, (int) attrRsp.mq_curmsgs);
        
        // Gives priority to the request queue if it is emptier than the response queue is full
        if (attrReq.mq_curmsgs < (MQ_MAX_MESSAGES - attrRsp.mq_curmsgs))
        {
            // Creates request
            req.y = total_send;
           
            // Sends request
            int code = mq_send(mq_fd_request, (char *) &req, sizeof(req), 0); 

            if (code < 0) {
                perror("An error occurred while sending request");
            } else {
                total_send++;

                printf("Request send\n");
            }
        } else {
            // Receives response
            ssize_t bytes_read = mq_receive(mq_fd_response, (char *) &rsp, sizeof(rsp), NULL);

            if (bytes_read > 0) // Tests if a response has been received
            {
                // Draws pixels based on response
                int i;
                for (i = 0; i < X_PIXEL; i++) {
                    output_draw_pixel(X_LOWERLEFT + (i * STEP), Y_LOWERLEFT + (rsp.y * STEP), rsp.k[i]);
                }

                total_received++;

                printf("Response received: y=%f k=%d\n", rsp.y, rsp.k[0]);
            } else {
                //printf("No reponse received\n");
            } 
        }

        j++;

    }
    printf("ended");

    // Cleans up the message queues
    usleep(1);
    mq_close (mq_fd_response);
    mq_close (mq_fd_request);
    mq_unlink (mq_name1);
    mq_unlink (mq_name2);

    // Important notice: make sure that your message queues contain your
    // student name and the process id (to ensure uniqueness during testing)
    
    //output_end();
    
    return (0);
}

