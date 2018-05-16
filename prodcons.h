/* Konstantinos Lamprakis 16/05/2018 */

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

 int producer_number, consumer_number, capacity, random_numbers, seed;

//------------ Structs declaraton ------------//

typedef struct circular_buffer
{
    void *buffer;     // data buffer
    void *buffer_end; // end of data buffer
    size_t capacity;  // maximum number of items in the buffer
    size_t count;     // number of items in the buffer
    size_t sz;        // size of each item in the buffer
    void *head;       // pointer to head
    void *tail;       // pointer to tail
} circular_buffer;

// used to parse arguments to each producer thread.
struct prod_data{
   int  prod_id; // unique id for each producer thread.
   int  random_numbers; // how many numbers to produce.
   int seed; // seed for rand_r.
};

//------------ Variables declaration ------------//

circular_buffer *cb;

pthread_mutex_t mutex; // mutex used for mutual exclusion among threads.
pthread_cond_t empty_cond, full_cond; // condition variables for empty or full circular buffer declaration.
pthread_attr_t attr;

int returned_code; // returned code from pthread_create(), pthred_join() and pthread_cond_timedwait() functions.
void *status; // status used by pthread_join function to store returned_code from each thread.

// deal with time.
struct timespec time_s;
struct timeval time_v;


//------------ Functions declaration ------------//

void read_parameters(int argc, char **argv, int *producer_number, int *consumer_number, int *capacity, int *random_numbers, int *seed);

void * producer(void *arg);

void * consumer(void *arg);
