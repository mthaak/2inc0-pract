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
        
    output_init();

    // Declarations
    mqd_t               mq_fd_request;
    mqd_t               mq_fd_response;
    MQ_REQUEST_MESSAGE  req;
    MQ_RESPONSE_MESSAGE rsp;
    struct mq_attr      attrReq;
    struct mq_attr      attrRsp;
    
    // Sets message queue names
    sprintf (mq_name1, "/mq_request_%s_%d", "MarkMartin", getpid());
    sprintf (mq_name2, "/mq_response_%s_%d", "MarkMartin", getpid());

    // Opens the request queue
    attrReq.mq_maxmsg  = MQ_MAX_MESSAGES;
    attrReq.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE); 
    mq_fd_request = mq_open (mq_name1, O_WRONLY | O_CREAT | O_EXCL, 0600, &attrReq);
    
    // Opens the response queue
    attrRsp.mq_maxmsg  = MQ_MAX_MESSAGES;
    attrRsp.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);
    mq_fd_response = mq_open (mq_name2, O_RDONLY | O_CREAT | O_EXCL, 0600, &attrRsp);

    if (mq_fd_request < 0) perror("Farmer could not open request queue");
    if (mq_fd_response < 0) perror("Farmer could not open response queue");
    
    // Creates NROF_WORKERS by forking
    create_workers();

    int total_send = 0;
    int total_received = 0;
    while (total_received < Y_PIXEL)  // loop until all rows have been received
    {
        // Gets attributes of the request and response queue        
        mq_getattr(mq_fd_response, &attrRsp);
        mq_getattr(mq_fd_request, &attrReq);

        // Gives priority to the request queue if it is emptier than the response queue is full
        if (attrReq.mq_curmsgs < (MQ_MAX_MESSAGES - attrRsp.mq_curmsgs))
        {
            // Creates request
            req.y = total_send;
           
            // Sends request
            int code = mq_send(mq_fd_request, (char *) &req, sizeof(req), 0); 
            if (code < 0) perror("Farmer could not send request");

            total_send++; 
        } else if (attrRsp.mq_curmsgs > 0)
        {
            // Receives response
            ssize_t bytes_read = mq_receive(mq_fd_response, (char *) &rsp, sizeof(rsp), NULL);
            if (bytes_read < 1) perror("Farmer could not receive response");

            // Draws pixels based on response
            int i;
            for (i = 0; i < X_PIXEL; i++) {
                output_draw_pixel(i, rsp.y, rsp.k[i]);
            }

            total_received++;
        } 
        
    }
    
    // Cleans up the message queues
    mq_close (mq_fd_response);
    mq_close (mq_fd_request);
    mq_unlink (mq_name1);
    mq_unlink (mq_name2);

    output_end();
    
    return (0);
}
