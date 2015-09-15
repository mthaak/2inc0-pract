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
                printf("worker pid:%d\n", getpid());
                execlp("worker", "worker", "", NULL);

                perror("execlp() failed");
            }
            
            waitpid(workerPID, NULL, 0);
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

    // TODO:
    //  * create the message queues & the children
    mqd_t               mq_fd_request;
    mqd_t               mq_fd_response;
    MQ_REQUEST_MESSAGE  req;
    MQ_RESPONSE_MESSAGE rsp;
    
    sprintf (mq_name1, "/mq_request_%s_%d", "MarkMartin", getpid());
    sprintf (mq_name2, "/mq_response_%s_%d", "MarkMartin", getpid());

    attr.mq_maxmsg  = MQ_MAX_MESSAGES;
    attr.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE);
    mq_fd_request = mq_open (mq_name1, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);

    attr.mq_maxmsg  = MQ_MAX_MESSAGES;
    attr.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);
    mq_fd_response = mq_open (mq_name2, O_RDONLY | O_CREAT | O_EXCL, 0600, &attr);

    for(i = 0; i < 64; i++) 
    {
        //create worker
    }

    //  * do the farming (use output_draw_pixel() for the coloring

    //  * wait until the chilren have been stopped

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

