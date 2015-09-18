/* 
 * Operating Systems (2INC0) Practical 
 * 2009/2015 (c) Joris Geurts
 *
 * This program contains some C constructs which might be useful for
 * various assignments of 2IN0C
 *
 *
 *      I M P O R T A N T    M E S S A G E :
 *      ====================================
 *
 * For readability reasons, this program does not check the return value of 
 * the POSIX calls.
 * This is not a good habit.
 * Always check the return value of a system call (you never know if the disk is
 * is full, or if we run out of other system resources)!
 * Possible construction:
 *
 *      rtnval = <posix-call>();
 *      if (rtnval == <error-value-according-documentation>)
 *      {
 *          perror ("<your-message>");
 *          exit (1);
 *      }
 *
 */

/* this pragma is added to avoid getting compiler warnings of unused functions 
 * (which would appear when you commented some various test-functions in main())
 */
#pragma GCC diagnostic ignored "-Wunused-function"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/wait.h>   /* for waitpid() */

typedef unsigned long long  MY_TYPE;

// create a bitmask where bit at position n is set
#define BITMASK(n)          (((MY_TYPE) 1) << (n))

// check if bit n in v is set
#define BIT_IS_SET(v,n)     (((v) & BITMASK(n)) == BITMASK(n))

// set bit n in v
#define BIT_SET(v,n)        ((v) =  (v) |  BITMASK(n))

// clear bit n in v
#define BIT_CLEAR(v,n)      ((v) =  (v) & ~BITMASK(n))

// create a bitmask of n bits
#define BITS_MASK(n)        ((1 << (n)) - 1)

static pthread_mutex_t      mutex          = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t       condition      = PTHREAD_COND_INITIALIZER;

#define STUDENT_NAME        "your_name_here"

static char                 mq_name1[80];
static char                 mq_name2[80];

typedef struct
{
    // a data structure with 3 members
    int                     a;
    int                     b;
    int                     c;
} MQ_REQUEST_MESSAGE;

typedef struct
{
    // a data structure with 3 members, where the last member is an array with 3 elements
    int                     d;
    int                     e;
    int                     f[3];
} MQ_RESPONSE_MESSAGE;

/*-------------------------------------------------------------------------*/


static void
bit_test (void)
{
    MY_TYPE     v;
    
    // set all bits to 1
    v = ~((MY_TYPE) 0);
    
    printf ("v (all 1's): %llx\n", v);
    BIT_CLEAR (v, 4);
    printf ("v (one 0)  : %llx\n", v);
    if (BIT_IS_SET (v, 4))
    {
        printf ("bit %d is set\n", 4);
    }
    if (BIT_IS_SET (v, 63))
    {
        printf ("bit %d is set\n", 63);
    }
    BIT_SET (v, 4);
    printf ("v (all 1's): %llx\n", v);
    printf ("\n");
}    
    
    
static void *
my_mutex_thread (void * arg)
{
    int     i = (int) arg;  // see calling of pthread_create() below  
    void *  rtnval;         // see calling of pthread_join() below
    
    printf ("        %lx: thread-%d start; wanting to enter CS...\n", pthread_self(), i);
    pthread_mutex_lock (&mutex);
    
    // Here we are in the critical section (protected by a mutex)
    printf ("        %lx: thread-%d entered CS\n", pthread_self(), i);
    sleep (6);
    printf ("        %lx: thread-%d leaves CS\n", pthread_self(), i);
    
    pthread_mutex_unlock (&mutex);
    
    rtnval = (void *) i;        // an arbitrary value...
    pthread_exit (rtnval);
}


static void
thread_mutex_test (void)
{
    void *      parameter;
    void *      rtnval;
    int         i;
    pthread_t   my_threads[2];

    // the 4th parameter of pthread_create() is the parameter when 
    // the thread-function my_thread() will be called.
    // it's a bit dirty to put an integer in a void* 
    // inside my_thread() we have to do the opposite: treat the void* as an integer
    // better solution would be: malloc() and free() (see thread_malloc_free_test() below)
    parameter = (void *) 3; // we want to pass integer value 3 to the new thread
    printf ("%lx: starting thread-3 ...\n", pthread_self());
    pthread_create (&my_threads[0], NULL, my_mutex_thread, parameter);
    sleep (3);
    parameter = (void *) 5; // we want to pass integer value 5 to the new thread
    printf ("%lx: starting thread-5 ...\n", pthread_self());
    pthread_create (&my_threads[1], NULL, my_mutex_thread, parameter);
    
    pthread_join (my_threads[0], NULL);
    // retrieve a return value from the finished thread 
    // (again with the dirty casting between pointers and integers)
    pthread_join (my_threads[1], &rtnval);  // do not use "(void **) &i" !!!
    i = (int) rtnval;    
    
    printf ("%lx: threads ready; return value=%d\n", pthread_self(), i);
    printf ("\n");
}


static void *
my_malloc_free_thread (void * arg)
{
    int *   argi; 
    int     i;      
    int *   rtnval;
    
    argi = (int *) arg;     // proper casting before dereferencing (could also be done in one statement)
    i = *argi;              // get the integer value of the pointer
    free (arg);             // we retrieved the integer value, so now the pointer can be deleted
    
    printf ("        %lx: thread started; parameter=%d\n", pthread_self(), i);
    
    sleep (1);
    
    rtnval = malloc (sizeof (int));
    *rtnval = 42;           // assign an arbitrary value...
    pthread_exit (rtnval);
}


static void
thread_malloc_free_test (void)
{
    int *       parameter;
    int *       rtnval;
    pthread_t   thread_id;
    
    parameter = malloc (sizeof (int));
    *parameter = 73;
    printf ("%lx: starting thread ...\n", pthread_self());
    pthread_create (&thread_id, NULL, my_malloc_free_thread, parameter);
    sleep (3);
    pthread_join (thread_id, (void **) &rtnval);
    
    printf ("%lx: thread ready; return value=%d\n", pthread_self(), *rtnval);
    free (rtnval);          // free the memory thas has been allocated by the thread
    printf ("\n");
}


static void
process_test (void)
{
    pid_t           processID;      /* Process ID from fork() */

    printf ("parent pid:%d\n", getpid());
    processID = fork();
    if (processID < 0)
    {
        perror("fork() failed");
        exit (1);
    }
    else
    {
        if (processID == 0)
        {
            printf ("child  pid:%d\n", getpid());
            execlp ("ps", "ps", "-l", NULL);
            //execlp ("./c_program", "my_own_name_for_argv0", "first_argument", NULL);
            
            // we should never arrive here...
            perror ("execlp() failed");
        }
        // else: we are still the parent (which continues this program)
        
        waitpid (processID, NULL, 0);   // wait for the child
    }
}


static void 
getattr (mqd_t mq_fd)
{
    struct mq_attr      attr;
    int                 rtnval;
    
    rtnval = mq_getattr (mq_fd, &attr);
    if (rtnval == -1)
    {
        perror ("mq_getattr() failed");
        exit (1);
    }
    fprintf (stderr, "%d: mqdes=%d max=%ld size=%ld nrof=%ld\n",
                getpid(), 
                mq_fd, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
}


static void
message_queue_child (void)
{
    mqd_t               mq_fd_request;
    mqd_t               mq_fd_response;
    MQ_REQUEST_MESSAGE  req;
    MQ_RESPONSE_MESSAGE rsp;

    mq_fd_request = mq_open (mq_name1, O_RDONLY);
    mq_fd_response = mq_open (mq_name2, O_WRONLY);

    // read the message queue and store it in the request message
    printf ("                                   child: receiving...\n");
    mq_receive (mq_fd_request, (char *) &req, sizeof (req), NULL);

    printf ("                                   child: received: %d, %d, %d\n",
            req.a, req.b, req.c);
    
    // fill response message
    rsp.d = -7;
    rsp.e = -77;
    rsp.f[0] = -777;    // array-index runs from 0 to n-1
    rsp.f[1] = -7777;
    rsp.f[2] = -77777;

    sleep (3);
    // send the response
    printf ("                                   child: sending...\n");
    mq_send (mq_fd_response, (char *) &rsp, sizeof (rsp), 0);

    mq_close (mq_fd_response);
    mq_close (mq_fd_request);
}

static void
message_queue_test (void)
{
    pid_t               processID;      /* Process ID from fork() */
    mqd_t               mq_fd_request;
    mqd_t               mq_fd_response;
    MQ_REQUEST_MESSAGE  req;
    MQ_RESPONSE_MESSAGE rsp;
    struct mq_attr      attr;

    sprintf (mq_name1, "/mq_request_%s_%d", "Mark", getpid());
    sprintf (mq_name2, "/mq_response_%s_%d", "Mark", getpid());

    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE);
    mq_fd_request = mq_open (mq_name1, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);

    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);
    mq_fd_response = mq_open (mq_name2, O_RDONLY | O_CREAT | O_EXCL, 0600, &attr);

    getattr(mq_fd_request);
    getattr(mq_fd_response);

    processID = fork();
    if (processID < 0)
    {
        perror("fork() failed");
        exit (1);
    }
    else
    {
        if (processID == 0)
        {
            // child-stuff
            message_queue_child ();
            exit (0);
        }
        else
        {
            // remaining of the parent stuff
            
            // fill request message
            req.a = 88;
            req.b = 888;
            req.c = 8888;

            sleep (3);
            // send the request
            printf ("parent: sending...\n");
            mq_send (mq_fd_request, (char *) &req, sizeof (req), 0);

            sleep (3);
            // read the result and store it in the response message
            printf ("parent: receiving...\n");
            mq_receive (mq_fd_response, (char *) &rsp, sizeof (rsp), NULL);

            printf ("parent: received: %d, %d, [%d,%d,%d]\n",
                    rsp.d, rsp.e, rsp.f[0], rsp.f[1], rsp.f[2]);
    
            sleep (1);
    
            waitpid (processID, NULL, 0);   // wait for the child
            mq_close (mq_fd_response);
            mq_close (mq_fd_request);
            mq_unlink (mq_name1);
            mq_unlink (mq_name2);   
        }
    }
}


static void
mask_test (void)
{
    printf ("bitmask of nineteen 1's: %04x\n", BITS_MASK (19));
}


static void *
my_condition_thread (void * arg)
{
    pthread_mutex_lock (&mutex);
    printf ("                    thread: enter CS\n");
    sleep (4);
       
    printf ("                    thread: wait...\n");
    pthread_cond_wait (&condition, &mutex);
    printf ("                    thread: signalled\n");
    
    sleep (2);
    printf ("                    thread: leave CS\n");
    pthread_mutex_unlock (&mutex);
    return (arg);
}

static void
condition_test (void)
{
    /* note: this test case is only to validate that condition variables really work
     * it doesn't give the proper usage for the assignments
     */
    pthread_t   my_thread;

    pthread_create (&my_thread, NULL, my_condition_thread, NULL);
    sleep (2);
    
    printf ("willing to enter...\n");
    pthread_mutex_lock (&mutex);
    printf ("enter CS\n");
    sleep (2);
       
    printf ("signal\n");
    pthread_cond_signal (&condition);
    
    sleep (2);
    pthread_mutex_unlock (&mutex);
    printf ("leave CS\n");
    
    pthread_join (my_thread, NULL);
}


/*-------------------------------------------------------------------------*/

int main (int argc, char * argv[])
{
    int	i;

    // command-line arguments are available in argv[0] .. argv[argc-1]
    // argv[0] always contains the name of the program
    
    // check if the user has started this program with valid arguments
    if (argc != 1)
    {
        fprintf (stderr, "%s: %d arguments:\n", argv[0], argc);
        for (i = 1; i < argc; i++)
        {
            fprintf (stderr, "     '%s'\n", argv[i]);
        }
        exit (1);
    }
    // else: parse the arguments...
    
    // for 'Interprocess Communication':
    process_test();
    message_queue_test();
    
    // for 'Threaded Application':
    //bit_test();
    //thread_malloc_free_test();
    //thread_mutex_test();
    
    // for 'Condition Variables':
    //condition_test();
    //mask_test();
    
    return (0);
}

