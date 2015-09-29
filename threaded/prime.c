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

//For bookkeeping about threads
typedef struct {
    pthread_t                   thread_id;  
    int                         parameter; 
    bool                        finished;
    int                         array_index; //index of array when put in thread_collection
    bool			slot_used; //is the slot used (running or waiting to be joined)
} THREAD_CONSTRUCT;

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

static pthread_mutex_t          mutex[(NROF_SIEVE/64) + 1]; //an array of mutexes (1 per 64bit number)

THREAD_CONSTRUCT                thread_collection[NROF_THREADS]; //structure with bookkeeping of all threads

int main (void)
{
    ints_in_buffer = (NROF_SIEVE/64) + 1;

    //initialise mutexes
    int mutex_index;
    for (mutex_index = 0; mutex_index < ints_in_buffer; mutex_index++) {
        pthread_mutex_init(&mutex[mutex_index], NULL);
    }

    //set all bits in buffer to 1 (all prime)
    int i;
    for (i = 0; i < ints_in_buffer; i++) {
        buffer[i] = ~0;
    }
 
    //set all thread slots as not used
    for (i = 0; i < NROF_THREADS; i++) {
        thread_collection[i].slot_used = false;
    }

    int k; //number of threads started
    k = 0;
    int current_base;
    current_base = 2;
    while (k < NROF_THREADS && current_base <= NROF_SIEVE) { //start up all threads
        if (BIT_IS_SET(buffer[(current_base / 64)], current_base % 64)) {
            strike_out_multiples(current_base, k);
            k++;
        }
        current_base++;
    }

    bool found_base;
    while (current_base <= NROF_SIEVE) { //until finished starting up threads...
        for (i = 0; i < NROF_THREADS; i++) { //...find a thread that has finished and start a new one
            if (thread_collection[i].finished == true) { //found finished thread
                pthread_join (thread_collection[i].thread_id, NULL);
                thread_collection[i].slot_used = false;
                
                //search for next suitable (prime) base
                found_base = false;
                while (found_base == false && current_base <= NROF_SIEVE) {
                    if (BIT_IS_SET(buffer[(current_base / 64)], current_base % 64)) {
                        strike_out_multiples(current_base, i); //start a new thread
                        found_base = true;
                    }
                    current_base++;
                }
            }
        }
    }

    for (i = 0; i < NROF_THREADS; i++) { //join (and wait for) all the threads that are used
        if (thread_collection[i].slot_used == true) {
            pthread_join (thread_collection[i].thread_id, NULL);
        }
    }
 
    //output all primes
    int primes_found = 0;
    int j;
    for (j = 2; j <= NROF_SIEVE; j++) {
        if (BIT_IS_SET(buffer[(j / 64)], j % 64)) {
            printf("%d\n", j);
            primes_found++;
        }
    }

    //printf("Found %d primes\n", primes_found);

    return (0);
}

//sets up bookkeeping and starts a new thread
void strike_out_multiples(int base, int index) {
    thread_collection[index].parameter = base;
    thread_collection[index].finished = false;
    thread_collection[index].array_index = index;
    thread_collection[index].slot_used = true;
    pthread_create (&thread_collection[index].thread_id, NULL, strike_out_thread, &thread_collection[index].array_index);
}

//The code executed in the thread
void strike_out_thread(void * arg) {
    //bookkeeping
    int *argi;
    int base;
    int multiple;
    argi = (int *) arg;  
    int thread_index = *argi;
    base = thread_collection[thread_index].parameter;

    //strike out all multiples
    multiple = 2 * base;
    while (multiple <= NROF_SIEVE) {
        pthread_mutex_lock (&mutex[multiple/64]); //entering critical section
        BIT_CLEAR(buffer[(multiple / 64)], multiple % 64);
        pthread_mutex_unlock (&mutex[multiple/64]); //leaving critical section
        multiple += base;
        rsleep(100);
    }
    thread_collection[thread_index].finished = true;
}

void print_buffer() //for testing purposes, prints all the 64 bit ints in buffer
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


