/* Konstantinos Lamprakis 16/05/2018 */

#include "prodcons.h"


int main(int argc, char **argv){

    // create new clean files and deletes old if exist.
    FILE *f = fopen("prod_in.txt", "w");
    fclose(f);
    f = fopen("cons_out.txt", "w");
    fclose(f);


    cb = (circular_buffer*)malloc(sizeof (struct circular_buffer)); // circular_buffer allocation.

    // parameters validation and initilization.
    read_parameters(argc, argv, &producer_number, &consumer_number, &capacity, &random_numbers, &seed);

    cb_init(cb, capacity, sizeof(int)); // circular_buffer initialization.

    // attribute initialization to makes threads joinable.
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_mutex_init(&mutex, NULL); // mutex initialization.
    pthread_cond_init (&empty_cond, NULL); // condition variables initialization.
    pthread_cond_init (&full_cond, NULL);

    // producer threads creation.

    struct prod_data prod_data_array[producer_number]; // used to parse arguments to each producer thread.
    pthread_t producers[producer_number]; // used to take a unique thread value.


    for(int i=0; i<producer_number; i++){
        prod_data_array[i].prod_id = i+1;
        prod_data_array[i].random_numbers = random_numbers;
        prod_data_array[i].seed = seed;

        returned_code = pthread_create(&producers[i], &attr, producer, (void *)&prod_data_array[i]);

        if (returned_code) {
            fprintf(stderr, "error: an error occured during producer thread creation");
            exit(-1);
        }
    }


    // consumer threads creation.
    int id[consumer_number]; // used to parse id to each consumer thread.
    pthread_t consumers[consumer_number]; // used to take a unique thread value.

    for(int i=0; i<consumer_number; i++){
        id[i] = i+1;

        returned_code = pthread_create(&consumers[i], &attr, consumer, (void *)id[i]);
        if (returned_code) {
            fprintf(stderr, "error: an error occured during consumer thread creation");
            exit(-1);
        }
    }

    pthread_attr_destroy(&attr); // delete attr and release resources.


    // wait for all threads to return.
    for(int i=0; i<producer_number; i++){
        returned_code = pthread_join(producers[i], &status);
        if (returned_code) {
            fprintf(stderr, "error: an error occured during waiting for producer threads to return.");
            exit(-1);
        }
    }

    for(int i=0; i<consumer_number; i++){
        returned_code = pthread_join(consumers[i], &status);
        if (returned_code) {
            fprintf(stderr, "error: an error occured during waiting for consumer threads to return.");
            exit(-1);
        }
    }

    // release resources.
    pthread_mutex_destroy(&mutex); // destroy mutex.
    pthread_cond_destroy(&empty_cond); // destroy condition variables.
    pthread_cond_destroy(&full_cond);
    free(cb);

}
