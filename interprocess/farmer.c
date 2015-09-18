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

static char         mq_name1[80];
static char         mq_name2[80];

// ADDED
pid_t worker_pids[NROF_WORKERS];

void create_workers(){
    int i;
    for (i = 0; i < NROF_WORKERS; i++){
        pid_t workerPID;
        workerPID = fork();
        if (workerPID < 0){
            perror("fork() failed");
            exit(1);
        } else {
            worker_pids[i] = workerPID;
            if (workerPID == 0){
                execlp("./worker", "worker", mq_name1, mq_name2, NULL);

                perror("execlp() failed");
            }
        } 
    }
}


int main (int argc, char * argv[])
{
    if (argc != 1)
    {
        fprintf (stderr, "%s: invalid arguments\n", argv[0]);
    }
        
    output_init ();

    //Create the message queues & the children
    mqd_t               mq_fd_request;
    mqd_t               mq_fd_response;
    MQ_REQUEST_MESSAGE  req;
    MQ_RESPONSE_MESSAGE rsp;
    struct mq_attr      attrReq;
    struct mq_attr      attrRsp;
    
    sprintf (mq_name1, "/mq_request_%s_%d", "MarkMartin", getpid());
    sprintf (mq_name2, "/mq_response_%s_%d", "MarkMartin", getpid());

    attrReq.mq_maxmsg  = MQ_MAX_MESSAGES;
    attrReq.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE); 
    int e = mq_fd_request = mq_open (mq_name1, O_WRONLY | O_CREAT | O_EXCL, 0600, &attrReq);
    
    attrRsp.mq_maxmsg  = MQ_MAX_MESSAGES;
    attrRsp.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);
    int f = mq_fd_response = mq_open (mq_name2, O_RDONLY | O_CREAT | O_EXCL, 0600, &attrRsp);

    if (e > -1) printf("Request queue opened\n");
    if (f > -1) printf("Response queue opened\n");
    
    //create_workers();

    //Do the farming
    int num_sent = 0;
    int num_received = 0;
    while (num_received < Y_PIXEL)
    {
        mq_getattr(mq_fd_response, &attrRsp);
        mq_getattr(mq_fd_request, &attrReq);
        //printf("Messages in queue, %d, %d\n", (int) attrReq.mq_curmsgs, (int) attrRsp.mq_curmsgs);
        if(false)//attrRsp.mq_curmsgs > 0)
        {
            //receive response
            ssize_t bytes_read = mq_receive(mq_fd_response, (char *) &rsp, sizeof(rsp), NULL);
            int i;
            for (i = 0; i < X_PIXEL; i++) {
                output_draw_pixel(i, num_received, rsp.colors[i]);
            }
            num_received++;
        }
        else if (num_sent < 5)
        {
            //send request
            int j;
            printf("test");
            /*for (j = 0; j < 3; j++){
                req.x[j] = 5;
                printf("%d", j);
                //req.y[j] = Y_LOWERLEFT+(num_sent*STEP);
            }
            req.x[0] = 1;
            req.x[1] = 2;
            req.x[2] = 3;*/
            int code;
            code = mq_send(mq_fd_request, (char *) &req, sizeof(req), 0); 
            if (code < 0) {
                printf("An error occurred while sending request:");
                printf("%d", errno);
                perror("");
            } else {
                printf("Request send");
            }
            num_sent++;
            //printf("Number of requests sent: %d\n", num_sent);
        }
    }

    //Wait for children to finish
    /*int status;
    int i;
    for (i = 0; i < NROF_WORKERS; i++) 
    {
        status = pthread_join (worker_pids[i], NULL);
        if (status != 0)
        {
            printf("An error occurred trying to join the workers");
        }
    }*/

    //Clean up the message queues
    mq_close (mq_fd_response);
    mq_close (mq_fd_request);
    mq_unlink (mq_name1);
    mq_unlink (mq_name2);

    // Important notice: make sure that your message queues contain your
    // student name and the process id (to ensure uniqueness during testing)
    
    output_end();
    
    return (0);
}

