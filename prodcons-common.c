/* Konstantinos Lamprakis 16/05/2018 */

#include "prodcons.h"
#include <errno.h>
#include <stdint.h>
#include <regex.h>
#include <time.h>

int prev_prod_id = 0, prev_cons_id = 0;

// Returns 1 if string match with pattern, 0 otherwise.
int match(const char *string, char *pattern)
{
    int status;
    regex_t re;

    if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
        return(0); // error occured.
    }
    status = regexec(&re, string, (size_t) 0, NULL, 0);
    regfree(&re);
    if (status != 0) {
        return(0); // error occured.
    }
    return(1);
}

// read parameters, make input control and initialization.
void read_parameters(int argc, char **argv, int *producer_number, int *consumer_number, int *capacity, int *random_numbers, int *seed){

    // check if there are exactly 6 parameters.
	if(argc != 6){ // remember: first argument is application name.
		fprintf(stderr, "Invalid number of parameters.\nTerminating.");
		exit(-1);
	}

    // check if all parameters(except first) are integers.
    for (int i=1; i<6; i++){
        if(!match(argv[i], "^[0-9]+$")){
            fprintf(stderr, "error: parameter number %d (with value %s) is invalid. Terminating.", i, argv[i]);
            exit(-1);
        }
    }

    // variables initialization.
    *producer_number = strtol(argv[1], NULL, 10);
    *consumer_number = strtol(argv[2], NULL, 10);
    *capacity = strtol(argv[3], NULL, 10);
    *random_numbers = strtol(argv[4], NULL, 10);
    *seed = strtol(argv[5], NULL, 10);
}

void * producer(void *arg){

    struct prod_data * data = (struct prod_data *) arg;
    unsigned int seed = data->seed * data->prod_id; // initialization of seed.
    int prod_numbers[random_numbers];// prod_numbers holds all numbers produced by this thread.l
    int index = 0; // index for prod_numbers.

    // Produce and push a random number in each iteration.
	for (int i=0; i<data->random_numbers; i++){

		int random = rand_r(&seed)%256; // random number production in range 0-256.
        prod_numbers[index++]=random;
		pthread_mutex_lock(&mutex); //lock mutex.

        while(cb->count == cb->capacity){
            pthread_cond_wait(&full_cond, &mutex);
        }
		cb_push_back(cb, (void*)&random);

		FILE *f = fopen("prod_in.txt", "a");
		if (f == NULL){
		    fprintf(stderr, "error: an error occured during opening file \"prod_in.txt\" \n");
		    exit(-1);
		}
		fprintf(f, "Producer %d: %d\n", data->prod_id, random);
		fclose(f);

		pthread_cond_broadcast(&empty_cond);
		pthread_mutex_unlock (&mutex);
		usleep(100);
	}

	// print numbers to stdout.
	while(data->prod_id != prev_prod_id + 1){
        usleep(1000);
	}
	prev_prod_id++;
    pthread_mutex_lock(&mutex); //lock mutex.
    printf("\nProducer %d: ", data->prod_id);
    for(int i=0; i<index-1; i++){
        printf("%d, ", prod_numbers[i]);
    }
    printf("%d\n", prod_numbers[index-1]);
    pthread_mutex_unlock (&mutex);

	pthread_exit(NULL);
}

void * consumer(void * arg){

    int cons_numbers[((producer_number*random_numbers)/consumer_number)*2]; // holds number readed from each trhead.
    int index =0; // index for cons_numbers.
    int id = ((int *) arg);
    int *tmp_read = (int *) malloc(sizeof(int));

	while(1){
		pthread_mutex_lock(&mutex);

		while (cb->count == 0) {

		    //implementation of time.
            gettimeofday(&time_v, NULL);

            // Convert from timeval to timespec
            time_s.tv_sec  = time_v.tv_sec;
            time_s.tv_nsec = time_v.tv_usec * 1000;
            time_s.tv_sec += 3; // 3 is numbers of seconds to wait.

            returned_code = pthread_cond_timedwait(&empty_cond, &mutex,&time_s);

            if (returned_code == ETIMEDOUT) {

                pthread_mutex_unlock(&mutex);
                free(tmp_read);

                // print numbers to stdout.
                while(id != prev_cons_id + 1){
                    usleep(1000);
                }
                prev_cons_id++;
                pthread_mutex_lock(&mutex); //lock mutex.
                printf("\nConsumer %d: ", id);
                for(int i=0; i<index-1; i++){
                    printf("%d, ", cons_numbers[i]);
                }
                printf("%d\n", cons_numbers[index-1]);
                pthread_mutex_unlock (&mutex);


                pthread_exit(NULL);

            }else if (returned_code!=0){
                fprintf(stderr, "error: an error occured during consumer waiting. Terminatng.");
                free(tmp_read);
                pthread_mutex_unlock(&mutex);
                exit(-1);
            }
		}

        cb_pop_front(cb, (void*)tmp_read);
        cons_numbers[index++] = *tmp_read;

		FILE *f = fopen("cons_out.txt", "a");

		if (f == NULL){
            fprintf(stderr, "error: an error occured during opening file \"cons_out.txt\" \n");
		    exit(-1);
		}

		fprintf(f, "Consumer %d: %d\n", id, *tmp_read);
		fclose(f);

        pthread_cond_broadcast(&full_cond);
		pthread_mutex_unlock (&mutex);
        usleep(100);
	}

}

//------------ circular_buffer implementation ------------//

//initialize circular buffer
//capacity: maximum number of elements in the buffer
//sz: size of each element
void cb_init(circular_buffer *cb, size_t capacity, size_t sz)
{
    cb->buffer = malloc(capacity * sz);
    if(cb->buffer == NULL){
		printf("Could not allocate memory..Exiting! \n");
		exit(1);
    }
    // handle error
    cb->buffer_end = (char *)cb->buffer + capacity * sz;
    cb->capacity = capacity;
    cb->count = 0;
    cb->sz = sz;
    cb->head = cb->buffer;
    cb->tail = cb->buffer;
}

//destroy circular buffer
void cb_free(circular_buffer *cb)
{
    free(cb->buffer);
    // clear out other fields too, just to be safe
}

//add item to circular buffer
void cb_push_back(circular_buffer *cb, const void *item)
{
    if(cb->count == cb->capacity)
    {
        printf("Access violation. Buffer is full\n");
        exit(1);
    }
    memcpy(cb->head, item, cb->sz);
    cb->head = (char*)cb->head + cb->sz;
    if(cb->head == cb->buffer_end)
        cb->head = cb->buffer;
    cb->count++;
}

//remove first item from circular item
void cb_pop_front(circular_buffer *cb, void *item)
{
    if(cb->count == 0)
    {
        printf("Access violation. Buffer is empy\n");
        exit(1);
	}
    memcpy(item, cb->tail, cb->sz);
    cb->tail = (char*)cb->tail + cb->sz;
    if(cb->tail == cb->buffer_end)
        cb->tail = cb->buffer;
    cb->count--;
}

