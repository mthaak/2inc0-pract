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

void strike_out_multiples(int base, int index);
void strike_out_thread(void * arg);
void print_buffer();
static void rsleep (int t);

int ints_in_buffer; //the number of 64 bit numbers in the buffer

static pthread_mutex_t          mutex[(NROF_SIEVE/64) + 1]; //an array of mutexes
static pthread_mutex_t          supermutex = PTHREAD_MUTEX_INITIALIZER;

THREAD_CONSTRUCT                thread_collection[NROF_THREADS];

int main (void)
{
    // TODO: start threads generate all primes between 2 and NROF_SIEVE and output the results
    // (see thread_malloc_free_test() and thread_mutex_test() how to use threads and mutexes,
    //  see bit_test() how to manipulate bits in a large integer)
    ints_in_buffer = (NROF_SIEVE/64) + 1;

    //initialise mutexes
    int mutex_index;
    for (mutex_index = 0; mutex_index < ints_in_buffer; mutex_index++) {
        pthread_mutex_init(&mutex[mutex_index], NULL);
    }

    //set all bits to 1
    int i;
    for (i = 0; i < ints_in_buffer; i++) {
        buffer[i] = ~0;
    }

    //print_buffer();

    int k; //number of threads started
    k = 0;
    int current_base;
    current_base = 2;
    while (k < NROF_THREADS && current_base <= NROF_SIEVE) {
        pthread_mutex_lock(&supermutex);
        if (BIT_IS_SET(buffer[(current_base / 64)], current_base % 64)) {
            strike_out_multiples(current_base, k);
            k++;
        }
        current_base++;
        pthread_mutex_unlock(&supermutex);
    }

    bool found_base;
    while (current_base <= NROF_SIEVE) {
        for (i = 0; i < NROF_THREADS; i++) {
            if (thread_collection[i].finished == true) {
                pthread_join (thread_collection[i].thread_id, NULL);
                found_base = false;
                while (found_base == false && current_base <= NROF_SIEVE) {
                    if (BIT_IS_SET(buffer[(current_base / 64)], current_base % 64)) {
                        strike_out_multiples(current_base, k);
                        found_base = true;
                    }
                    current_base++;
                }
            }
        }
    }

    for (i = 0; i < NROF_THREADS; i++) {
        pthread_join (thread_collection[i].thread_id, NULL);
    }

    //output all primes
    int primes_found = 0;
    int j;
    for (j = 2; j <= NROF_SIEVE; j++) {
        if (BIT_IS_SET(buffer[(j / 64)], j % 64)) {
            //printf("%d\n", j);
            primes_found++;
        }
    }
    printf("Found %d primes\n", primes_found);

    //print_buffer();

    return (0);
}

void strike_out_multiples(int base, int index) {
    //printf("evaluating multiples of %d\n", base);
    thread_collection[index].parameter = base;
    thread_collection[index].finished = false;
    thread_collection[index].array_index = index;
    pthread_create (&thread_collection[index].thread_id, NULL, strike_out_thread, &thread_collection[index].array_index);
}

void strike_out_thread(void * arg) {
    int *argi;
    int base;
    int multiple;
    argi = (int *) arg;  
    int thread_index = *argi;
    base = thread_collection[thread_index].parameter;
    multiple = 2 * base;
    while (multiple <= NROF_SIEVE) {
        //printf("setting %d to 0 as multiple of %d\n", multiple, base);
        pthread_mutex_lock (&mutex[multiple/64]); //entering critical section
        BIT_CLEAR(buffer[(multiple / 64)], multiple % 64);
        pthread_mutex_unlock (&mutex[multiple/64]); //leaving critical section
        multiple += base;
        //rsleep(10);
    }
    thread_collection[thread_index].finished = true;
}

void print_buffer() //for testing purposes
{
    int i;
    for (i = 0; i < ints_in_buffer; i++) {
        printf ("buffer: %llx", buffer[i]);
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


