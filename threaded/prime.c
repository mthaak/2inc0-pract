/* 
 * Operating Systems  (2INC0)   Practical Assignment
 * Threaded Application
 *
 * Mark Bouwman (0868533)
 * Martin ter Haak (0846351)
 *
 * Grading:
 * Students who hand in clean code that fully satisfies the minimum requirements will get an 8. 
 * â€Extraâ€ steps can lead to higher marks because we want students to take the initiative. 
 * Extra steps can be, for example, in the form of measurements added to your code, a formal 
 * analysis of deadlock freeness etc.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>     // for usleep()
#include <time.h>       // for time()
#include <errno.h>
#include <pthread.h>
#include "prime.h"
typedef unsigned long long  MY_TYPE;

// create a bitmask where bit at position n is set
#define BITMASK(n)          (((MY_TYPE) 1) << (n))

// check if bit n in v is set
#define BIT_IS_SET(v,n)     (((v) & BITMASK(n)) == BITMASK(n))

// set bit n in v
#define BIT_SET(v,n)        ((v) =  (v) |  BITMASK(n))

// clear bit n in v
#define BIT_CLEAR(v,n)      ((v) =  (v) & ~BITMASK(n))

// declare a mutex, and it is initialized as well
static pthread_mutex_t      mutex          = PTHREAD_MUTEX_INITIALIZER;

void print_buffer();
static void rsleep (int t);

int ints_in_buffer;

int main (void)
{
    // TODO: start threads generate all primes between 2 and NROF_SIEVE and output the results
    // (see thread_malloc_free_test() and thread_mutex_test() how to use threads and mutexes,
    //  see bit_test() how to manipulate bits in a large integer)
    ints_in_buffer = (NROF_SIEVE/64) + 1;

    //set all bits to 1
    int i;
    for (i = 0; i < ints_in_buffer; i++) {
        buffer[i] = ~0;
    }

    print_buffer();

    int k;
    for (k = 2; k <= NROF_SIEVE; k++) {
        printf("evaluating multiples of %d\n", k);
        int multiple;
        multiple = 2 * k;
        while (multiple <= NROF_SIEVE) {
            
            
            printf("setting %d to 0\n", multiple);
            BIT_CLEAR(buffer[(multiple / 64)], multiple % 64);
            multiple += k;
        }
    }

    //output all primes
    int j;
    for (j = 2; j <= NROF_SIEVE; j++) {
        if (BIT_IS_SET(buffer[(j / 64)], j % 64))
        {
            printf("%d\n", j);
        }
    }

    return (0);
}

void print_buffer() 
{
    int i;
    for (i = 0; i < ints_in_buffer; i++) {
        printf ("%llx", buffer[i]);
    }
    printf("\n");
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid());
        first_call = false;
    }
    usleep (random () % t);
}


