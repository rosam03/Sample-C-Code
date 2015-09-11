/*
 * Edited by: Rosa Mohammadi
 * CPSC 261 Lab 7
 * phil-ordered
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


#define MAX_THINKING_TIME 25000 // 25000 usec
#define DO_LOCK 1 // 1 = Lock for atomicity, 0 = May execute out of order
#define VERBOSE 1 // 1 = Print movements of philosophers , 0 = Do not print anything

// initialize maximum number of locks
pthread_mutex_t lockRightFork[25000] = {PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t lockLeftFork[25000] = {PTHREAD_MUTEX_INITIALIZER};


long numTime; // Global variable for number of eating sessions

/*
 * Locks right forks. If the right fork is already locked, thread waits
 * until is becomes available.
 */
void synchronized_Right(char *message, long rForkNum)
{
    
    if (DO_LOCK) // all obtain the right fork
    {
        // If the lock is already aquired, wait for it
        pthread_mutex_lock(&lockRightFork[rForkNum]); // try to lock the right fork for that philosopher
    }
    
    if(VERBOSE){ // Print messages
        while (*message)
        {
            putchar(*message++); // Display the message when right fork is obtained
        }
        putchar('\n');
    }
    
}

/*
 * Locks left forks. If the right fork is already locked, thread waits
 * until is becomes available.
 */
void synchronized_Left(char *message, long lForkNum)
{
    if (DO_LOCK)
    {
        pthread_mutex_lock(&lockLeftFork[lForkNum]); // lock the fork for that philosopher
    }
    
    if(VERBOSE){ // Print messages
        while (*message)
        {
            putchar(*message++); // Display the message when right fork is obtained
        }
        putchar('\n');
    }
    
}

/*
 * Releases/unlocks both forks for all philosophers.
 */
void synchronized_down(char *message)
{
    int x;
    
    if (DO_LOCK) // then release the right forks and left forks
    {
        for (x = 0; x < 25000; ++x) { // release all possible forks
            pthread_mutex_unlock(&lockRightFork[x]);
            pthread_mutex_unlock(&lockLeftFork[x]);
        }
    }
    
    if(VERBOSE){ // Print messages
        while (*message)
        {
            putchar(*message++); // Display the message when right fork is obtained
        }
        putchar('\n');
    }
    
}

void thinkDeepThoughts(long phil){
    
    if(VERBOSE){ // Print messages
        printf("Philosopher %ld is thinking deep thoughts. \n", phil);
        usleep(random() % MAX_THINKING_TIME); // Think deep thoughts before eating
        
    }
    
}

/*
 * Struct that holds data passed to each thread.
 */
typedef struct thread_data
{
    int threads; // Number of threads/philosophers to create
    char *messageRight; // Message to display when picking right fork up
    char *messageLeft; // Message to display when picking left fork up
    char *messageDown; // Message to display when putting both forks down
} thread_data;

/*
 * A function thread_body() that is executed by each one of the
 * threads.
 */
void *thread_body(void *arg)
{
    long x = numTime; // Keep track of eating sessions complete
    long y = 0;
    thread_data *data = arg;
    char *buffer = malloc(256);
    long compare = (y+1)%data->threads; 
    
    while(x > 0){ // While there are eating sessions left
        
        
        if(VERBOSE){ // Print messages
            // Let user know which eating session philosophers are on
            printf("Eating for the %ld time. \n", numTime - x);
        }
        
        for(y = 0; y < data->threads; y++){
            
            thinkDeepThoughts(y); // Think deep thoughts before eating
            
            if( compare < y){ // if right fork number is smaller, obtain it first
                sprintf(buffer, "Thread %ld says \"%s\" for the %ldth time", y,          data->messageRight, numTime - x); // say who obtained the right fork
                synchronized_Right(buffer, (y+1)%data->threads); // obtain locks with defined right fork, currentPhil+1 mod totalPhils
            }
            
            else{ // if left fork number is smaller, obtain it
                sprintf(buffer, "Thread %ld says \"%s\" for the %ldth time", y, data->messageLeft, numTime - x); // say we obtained the left forks
                synchronized_Left(buffer, y); // obtain locks with defined left fork
            }
            
            thinkDeepThoughts(y); // Think deep thoughts after getting first fork
        }
        
        
        // Now that all threads have obtained smaller numbered forks,
        // begin picking up the other forks
        for(y = 0; y < data->threads; y++){ // all pick up other forks
            
            if( y%data->threads < y){ // if right fork num was smaller, get left now
                sprintf(buffer, "Thread %ld says \"%s\" for the %ldth time", y, data->messageLeft, numTime - x); // say we obtained the left fork
                synchronized_Left(buffer, y); // get the left fork now
                
            }
            
            else{ // the left fork number was smaller, so get the right now
                sprintf(buffer, "Thread %ld says \"%s\" for the %ldth time", y,          data->messageRight, numTime - x); // say who obtained the right fork
                synchronized_Right(buffer, (y+1)%data->threads); // obtain locks with defined right fork, currentPhil mod totalPhils
            }
            
            thinkDeepThoughts(y); // Think deep thoughts after having both forks
        }
        
        
        // Now that all threads have both forks, put them both down
        sprintf(buffer, "%s",data->messageDown); // Notify both forks put down
        synchronized_down(buffer); // Release all locks
        
        usleep(random() % MAX_THINKING_TIME); // Think deep thoughts after eating session
        
        x--; // Decrement number of times left to eat since we just did one session
    }
    
    return 0; // Exit when done eating number of specified times
}

/*
 * The main function: it starts the variable number of threads.
 */
int main(int argc, char **argv)
{
    
    int PHILOSOPHERS = atoi(argv[1]);  // get number of philosophers as 1st argument
    int NUM_TO_EAT = atoi(argv[2]); // get number of times to eat as second argument
    
    thread_data arr; // thread data will be passed as arguments
    thread_data dataSet[PHILOSOPHERS]; // store necessary elements of data
    pthread_t threads[PHILOSOPHERS]; // create necessary threads
    srandom(time(0));
    
    /*
     * Initialize the data used by each thread.
     */
    int x;  // Threads will be variable since user defined
    for(x = 0; x < PHILOSOPHERS; x++){
        arr.threads = PHILOSOPHERS; // Keep track of philosophers
        arr.messageRight = "Picked up RIGHT fork";  // Notify right fork is obtained
        arr.messageLeft = "Picked up LEFT fork"; // Notify left fork is obtained
        arr.messageDown = "Put down both forks"; // Notify eating session is over
        dataSet[x] = arr; // Keep track of data for each thread
    }
    
    // Now create the threads
    for(x = 0; x < PHILOSOPHERS; x++){
        
        // Check for errors in creating or entering threads
        if ( pthread_create(&threads[x], NULL, (void *) &thread_body, &(dataSet[x]) ) ){
            printf("Error in creating thread #: %d. \n", x);
        }
    }
    
    numTime = NUM_TO_EAT; // Allow other functions to know the argument passed
    
    // Now join the threads
    for( x = 0; x < PHILOSOPHERS; x++){
        while(pthread_join(threads[x], NULL)) { // Check if thread x was entered
            printf("Thread %d was NOT entered. \n", x);
        }
    }
    
    // Done
    return 0;
}
