
// Include
#include <sys/time.h>
#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
// !!!!!! This value is CPU-dependent !!!!!!

//#define LOOP_ITERATIONS_PER_MILLISEC 155000

#define LOOP_ITERATIONS_PER_MILLISEC 178250
//#define LOOP_ITERATIONS_PER_MILLISEC 193750


#define MILLISEC    1000
#define MICROSEC    1000000
#define NANOSEC    1000000000



/*   This is how CASIO tasks are specified 

In the file linux-2.6.24-casio/include/linux/sched.h

also in the file /usr/include/bits/sched.h (the user space scheduler file)

struct sched_param {
	int sched_priority;
#ifdef CONFIG_SCHED_SMS_POLICY
	int casio_pid;
	unsigned long long deadline;
#endif
};
*/

/// The minimum and maximum offset
double min_offset, max_offset; //seconds
/// The minimum and maximum execution time
double min_exec_time, max_exec_time; //seconds
/// The minimum and maximum interval for the arrival time in seconds
double min_inter_arrival_time, max_inter_arrival_time; //seconds

/// The ID's
unsigned int casio_id, jid = 1;
/// Interval timer. More can be found on https://man7.org/linux/man-pages/man2/getitimer.2.html
struct itimerval inter_arrival_time;


/**
 * Pass through one millisecond, going through as many iterations at it takes for it.
 * @return N/A

 * */
void burn_1millisecs() {
    unsigned long long i;
    for (i = 0; i < LOOP_ITERATIONS_PER_MILLISEC; i++);
}


/**
 * Burn through as many milliseconds as it is wished
 * @param milliseconds How many milliseconds are to be passed
 * @return N/A
 * */
void burn_cpu(long milliseconds) {
    long i;
    for (i = 0; i < milliseconds; i++)
        burn_1millisecs();
}

/**
 * @brief Clear a sched param, setting both the deadline and the id to default values
 * @param param The pointer to the parameter
 * @return N/A
 * */
void clear_sched_param(struct sched_param *param) {
    param->casio_id = -1;
    param->deadline = 0;
}

/**
 * @brief Prints the information of a parameter.
 * @param param Parameter whose information we need
 * @return N/A
 * */
void print_task_param(struct sched_param *param) {
    printf("\npid[%d]\n", param->casio_id);
    printf("deadline[%llu]\n", param->deadline);
}

/**
 * Clear the timer used in a certain address
 * @param t The Time Interval variable that we want to reset
 * */
void clear_signal_timer(struct itimerval *t) {
    t->it_interval.tv_sec = 0;
    t->it_interval.tv_usec = 0;
    t->it_value.tv_sec = 0;
    t->it_value.tv_usec = 0;
}

/**
 * Set the timer in a certain address.
 * @param t The timer
 * @param secs The amount (in seconds) that we want to set it as
 * */
void set_signal_timer(struct itimerval *t, double secs) {
    t->it_interval.tv_sec = 0;
    t->it_interval.tv_usec = 0;
    t->it_value.tv_sec = (int) secs; // Set the value in here
    t->it_value.tv_usec = (secs - t->it_value.tv_sec) * MICROSEC; // And try to convert the difference to microseconds

}

/**
 * Get the information of a timer
 * @param t The pointer to the timer whose information we need
 * */
void print_signal_timer(struct itimerval *t) {
    printf("Interval: secs [%ld] usecs [%ld] Value: secs [%ld] usecs [%ld]\n",
           t->it_interval.tv_sec,
           t->it_interval.tv_usec,
           t->it_value.tv_sec,
           t->it_value.tv_usec);
}

/**
 * Get a random time value. If the min and the max are equal, just return that value.
 * @param min The minimum time needed
 * @param max The max time allowed
 * @return A number between the minimum and the maximum
 * */
double get_time_value(double min, double max) {
    if (min == max)
        return min;
    return (min + (((double) rand() / RAND_MAX) * (max - min)));
}


/**
 * Start a task.
 * @param s Deprecated. The seed
 * @return N/A
 * */
void start_task(int s) {
    printf("\nTask(%d) has just started\n", casio_id);
    set_signal_timer(&inter_arrival_time, get_time_value(min_offset, max_offset)); // Set the timer's signal
    setitimer(ITIMER_REAL, &inter_arrival_time, NULL); // Start the timer
}

/**
 * Try to complete the task.
 * @param s Deprecated. The seed
 * @return N/A
 * */
void do_work(int s) {
    signal(SIGALRM, do_work); // Try to create a signal
    set_signal_timer(&inter_arrival_time,
                     get_time_value(min_inter_arrival_time, max_inter_arrival_time)); // Set the signal to the timer

    setitimer(ITIMER_REAL, &inter_arrival_time, NULL); // Set the timer

    clock_t start, end;
    double elapsed = 0;
    start = clock(); // Set the starting time to the current time
    printf("Job(%d,%d) starts\n", casio_id, jid);
    burn_cpu(get_time_value(min_exec_time, max_exec_time) * MILLISEC); // Simulate the CPU taking time to do something
    end = clock(); // After done, set the end variable as the current time
    elapsed = ((double) (end - start)) / CLOCKS_PER_SEC; // Set the elapsed time as the difference between end and start
    printf("Job(%d,%d) ends (%f)\n", casio_id, jid, elapsed);
    jid++; // Change the job number

}

/**
 * End a task
 * @param s Deprecated. The seed
 * @return N/A
 * */
void end_task(int s) {
    printf("\nTask(%d) has finished\n", casio_id);
    exit(0);
}


/**
 * Main method
 * @param casio_id Arg 1. The id of the task to be created.
 * @param min_exec_time Arg 2. The minimum execution time.
 * @param max_exec_time Arg 3. The maximum execution time.
 * @param min_inter_arrival_time Arg 4. The minimum interval arrival time.
 * @param max_inter_arrival_time Arg 5. The maximum interval arrival time.
 * @param deadline Arg 6. The chosen deadline.
 * @param min_offset Arg 7. The minimum offset from last.
 * @param max_offset Arg 8. The maximum offset from last.
 * @param seed Arg 9. A seed used to generate values.
 * @return N/A
 * */
int main(int argc, char **argv) {

    struct sched_param param; /// < Create the parameter

    unsigned long long seed;
    int i;

    clear_signal_timer(&inter_arrival_time); // Clear the timer's signal

    clear_sched_param(&param); // Clear the parameter

    param.sched_priority = 1; // Set the priority in the schedule

    casio_id = param.casio_id = atoi(argv[1]); // Set the id
    min_exec_time = atof(argv[2]); // Grab the minimum execution time
    max_exec_time = atof(argv[3]); // Grab the maximum execution time
    min_inter_arrival_time = atof(argv[4]); // Grab the minimum arrival time
    max_inter_arrival_time = atof(argv[5]); // Grab the maximum arrival time
    param.deadline = atof(argv[6]) * NANOSEC; // Set a deadline for the parameter
    min_offset = atof(argv[7]); // Set the minimum offset
    max_offset = atof(argv[8]); // Set the maximum offset
    seed = atol(argv[9]); // Generate a seed
    srand(seed); // Based on a seed, generate a random number
    signal(SIGUSR1, start_task); //Signal to start a task
    signal(SIGALRM, do_work); // Signal to work on a task
    signal(SIGUSR2, end_task); // Signal to end a task

    print_task_param(&param); // Print the param
    printf("Before sched_setscheduler:priority %d\n", sched_getscheduler(0));

    if (sched_setscheduler(0, SCHED_CASIO, &param) == -1) {
        perror("ERROR");
    }

    printf("After sched_setscheduler:priority %d\n", sched_getscheduler(0));

    //Wait for signals while the task is idle
    while (1) {
        pause();
    }
    return 0;
}
	
	
	

	
	
	
	

