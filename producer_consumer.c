#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// global variables to be syncrhonized via mutex when used in pthreads
#define MAX_BUFFER_SIZE 50
static int buffer[MAX_BUFFER_SIZE]; //needed to provide a max buffer size of 50 for init
static int buffer_curr_spot = 0;
static int buffer_num_items = 0;
static int thread_no = 0;

// create mutext to provide synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


//NOTE: main primarily just handles arguments and delegates to run() for functionality
void main(int argc, char* argv[]) {
  if (argc == 3) { // check for correct # or args
    // convert args to int and run; 
    // ASSUME: methods are passed in correct format
    run(atoi(argv[1]), atoi(argv[2]));
  } else {
    // inform user they entered wrong # or args
    puts("2 Arguments Required: int num_threads, int buffer_size");
  }
}


// intent: a synchronized function that produces buffer items
void* produce(void* data) {
  int rc; //mutex lock results
  int rnd = get_rand(100);

  // lock the mutex for exclusive buffer access
  rc = pthread_mutex_lock(&mutex);

    buffer[buffer_curr_spot] = rnd; //100 is arbitrary limit
    buffer_num_items++;
    thread_no++;
    printf("Thread %d produce %d in buffer %d, current number of items is %d!\n", thread_no, rnd, buffer_curr_spot, buffer_num_items);
    buffer_curr_spot++;

  //unlock the mutex so other threads can access buffer
  rc = pthread_mutex_unlock(&mutex);

  /* terminate the thread */
  pthread_exit(NULL);
}


// intent: a synchronized function that consumes items from the buffer
void* consume(void* data) {
  int rc; //mutex lock results

  // lock the mutex for exclusive buffer access
  rc = pthread_mutex_lock(&mutex);

    buffer_curr_spot--;
    int out = buffer[buffer_curr_spot];
    buffer[buffer_curr_spot] = -1; //-1 just means item removed
    buffer_num_items--;
    thread_no++;
    printf("Thread %d consume %d in buffer %d, current number of items is %d!\n", thread_no, out, buffer_curr_spot, buffer_num_items);

  //unlock the mutex so other threads can access buffer
  rc = pthread_mutex_unlock(&mutex);

  /* terminate the thread */
  pthread_exit(NULL);
}


// intent: this handles the bulk of the program so main can just handle args
int run(int num_threads, int buffer_size) {
  int i, num_producers = 0, num_consumers = 0;
  
  //randomize: http://faq.cprogramming.com/cgi-bin/smartfaq.cgi?answer=1042005782&id=1043284385
  srand48(time(NULL));

  // randomly generate the number of producers and consumers
  gen_producer_consumer_totals(&num_producers, &num_consumers, num_threads);
  
  // print program status
  printf("Total worker thread: %d, buffer size: %d\n", num_threads, buffer_size);
  printf("Number of producer: %d Number of consumer: %d\n", num_producers, num_consumers);
 
  // for each thread, create a new pthread and run functions in parallel 
  for (i=0; i < num_threads; i++) {
    pthread_t  p_thread; //new instance for each thread
    // launch the producer execution thread
    if (num_producers > 0) {
      pthread_create(&p_thread, NULL, produce, &i);
      num_producers--;
    } else {
      pthread_create(&p_thread, NULL, consume, &i);
      num_consumers--;
    }
  }

  for(i = 0; i < 5000000; i++)
    ; // delay loop so we can see the output

  return 0;
}


// intent: generate a random # that is appropriate for this application (max_size)
int get_rand(int max_size) {
  return (int)(drand48()*100) % max_size;
}


/** intent: randomly generate the number of producers and consumers
 *  *prod and *con are pointers to the vars holding producer and consumer totals
 *  max_combined is the total number of producers and consumers, combined
 */
void gen_producer_consumer_totals(int* prod, int* con, int max_combined) {
  // max_combined enforces: num_producers + num_consumers = num_threads
  int tmp = get_rand(max_combined); 

  // enforce: num_consumer <= num_producer
  if (tmp > (max_combined/2)) {
    *prod = tmp; 
    *con = max_combined - tmp;
  } else {
    *con = tmp; 
    *prod = max_combined - tmp;
  }
}


// intent: my attempt at creating a function that returns a function point to pass to thread
// NOTE: I couldn't get this to work but left it in for future reference
/*
void* chooseThreadFunc() {
  void* func;
  int rc;

  // lock the mutex for exclusive buffer access
  rc = pthread_mutex_lock(&mutex);

    if (thread_no %2 == 1 && num_consumers > 0) {
      func = &consume;
      num_consumers--;
    } else {
      func = &produce;
      num_producers--;
    }

  //unlock the mutex so other threads can access buffer
  rc = pthread_mutex_unlock(&mutex);

  return ((void*)func);
}*/
