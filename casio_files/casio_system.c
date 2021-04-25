// Import the libraries
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

// Constants
#define  BUF_LEN            200 /// < The max buf length
#define  CASIO_TASKS_NUM        20    /// < The allowed CASIO tasks

/// Struct that allows us to see the id, the arrival, the deadline, and the offset
struct casio_tasks_config {
    int pid;
    double min_exec;
    double max_exec;
    double min_inter_arrival;
    double max_inter_arrival;
    double deadline;
    double min_offset;
    double max_offset;
};

/// The id of the task
pid_t casio_tasks_pid[CASIO_TASKS_NUM];
int casio_tasks_num = 0; /// < Set the number to 0

/**
 * @brief Converts a string to an integer
 * @param str The string to be converted
 * @return The converted value
 * */
int get_int_val(char *str) {
    char *s = str; // Grab the pointer from the string
    int val; // Allocate the number memory
    for (s = str; *s != '\t'; s++); // Find the horizontal tab escape value of the string and point to it
    *s = '\0'; // Set the tab value as a null escape sequence
    val = atoi(str); // Convert the string to an int
    return val;
}

/**
 * @brief Print the config of the tasks
 * @param tasks The tasks whose configuration will be printed
 * @param num The number of tasks
 * @return N/A
 * */
void print_casio_tasks_config(struct casio_tasks_config *tasks, int num) {
    int i; // Declaration of iterable
    printf("\nCASIO TASKS CONFIG\n");
    printf("pid\tmin_c\tmax_c\tmin_t\tmax_t\tdeadl\tmin_o\tmax_o\n");
    for (i = 0; i < num; i++) { // Print each task's information
        printf("%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
               tasks[i].pid,
               tasks[i].min_exec,
               tasks[i].max_exec,
               tasks[i].min_inter_arrival,
               tasks[i].max_inter_arrival,
               tasks[i].deadline,
               tasks[i].min_offset,
               tasks[i].max_offset
        );
    }
}

/**
 * Clear the task's configuration. Can take many tasks.
 * @param tasks The tasks
 * @param num The number of tasks
 * @return N/A
 * */
void clear_casio_tasks_config_info(struct casio_tasks_config *tasks, int num) {
    int i;

    for (i = 0; i < num; i++) {
        tasks[i].pid = 0;
        tasks[i].min_exec = 0;
        tasks[i].max_exec = 0;
        tasks[i].min_inter_arrival = 0;
        tasks[i].max_inter_arrival = 0;
        tasks[i].deadline = 0;
        tasks[i].min_offset = 0;
        tasks[i].max_offset = 0;
    }
}


/**
 * @brief Get the specific config of a task
 * @param str String with the numbers depicting the properties
 * @param tasks The tasks
 * @param n The pointer to the task needed from the previous list
 * */
void get_casio_task_config_info(char *str, struct casio_tasks_config *tasks, int *n) {
    char *s, *s1; /// < Iterables
    int i = 0; /// String length iterable
    s = s1 = str;
    while (i < 7) {
        if (*s == '\t') {
            *s = '\0'; /// < Assign a null value to the current string position
            switch (i) { /// < Iterate through Cases
                case 0:
                    tasks[*n].pid = atoi(s1);
                    s1 = s + 1;
                    i++;
                    break;
                case 1:
                    tasks[*n].min_exec = atof(s1);
                    s1 = s + 1;
                    i++;
                    break;
                case 2:
                    tasks[*n].max_exec = atof(s1);
                    s1 = s + 1;
                    i++;
                    break;
                case 3:
                    tasks[*n].min_inter_arrival = atof(s1);
                    s1 = s + 1;
                    i++;
                    break;
                case 4:
                    tasks[*n].max_inter_arrival = atof(s1);
                    s1 = s + 1;
                    i++;
                    break;
                case 5:
                    tasks[*n].deadline = atof(s1);
                    s1 = s + 1;
                    i++;
                    break;
                case 6:
                    tasks[*n].min_offset = atof(s1);
                    s1 = s + 1;
                    i++;
                    tasks[*n].max_offset = atof(s1);
                    break;

            }

        }
        s++;
    }
    (*n)++;
}

/**
 * @brief Grab the information of many tasks at the same time
 * @param file Where the tasks are stored
 * @param duration Total duration permited
 * @param tasks The Tasks
 * @param n Offset. Redundant.
 * */
void get_casio_tasks_config_info(char *file, int *duration, struct casio_tasks_config *tasks, int *n) {
    char buffer[BUF_LEN]; // Create the buffer
    int count = 0; // Set the count to 0
    FILE *fd = fopen(file, "r"); // Open the file
    *n = 0; // Reason as to why it is redundant: Assigned anyways.
    buffer[0] = '\0'; // Set the first character in the buffer as null.
    while ((fgets(buffer, BUF_LEN, fd)) != NULL) { // As long as it is not null, continue
        if (strlen(buffer) > 1) { // If the buffer is larger than 1, try to read
            switch (count) {
                case 0:
                    *duration = get_int_val(buffer);
                    count++;
                    break;
                default:
                    get_casio_task_config_info(buffer, tasks,
                                               n); // Given the fact that the count iis bigger than one, we just need to grab the information of the current task
            }
        }
        buffer[0] = '\0'; // Set the buffer to a null string
    }
    fclose(fd); // Close the file buffer

}

/**
 * @brief Starts a simulation of the scheduling
 * @return N/A
 * */
void start_simulation() {
    int i; // Iterable
    printf("I will send a SIGUSR1 signal to start all tasks\n");
    for (i = 0; i < casio_tasks_num; i++) {
        kill(casio_tasks_pid[i],
             SIGUSR1); // Do exactly what we were told not to do: KILL the process based on what the number of casio tasks is.
    }

}

/**
 * @brief Ends the simulation
 * @param signal Deprecated. Signal type.
 * @return N/A
 * */
void end_simulation(int signal) {
    int i;
    printf("I will send a SIGUSR2 signal to finish all tasks\n");
    for (i = 0; i < casio_tasks_num; i++) {
        kill(casio_tasks_pid[i], SIGUSR2); // Kill
    }

}

/**
 * @brief Print help
 * @param name The name of the command being used
 * @return 0
 * */
void help(char *name) {
    fprintf(stderr, "Usage: %s file_name (system configuration)\n", name);
    exit(0);
}


/**
 * The main function of the code.
 * Runs a simulation, using certain intervals and grabbing the
 * information of the CASIO tasks. Prints their information and
 * then ends the simulation back down.
 * @return 0
 * */
int main(int argc, char *argv[]) {

    int duration, i, j, k, n;
    struct casio_tasks_config casio_tasks_config[CASIO_TASKS_NUM];
    struct itimerval sim_time;
    char arg[CASIO_TASKS_NUM][BUF_LEN], *parg[CASIO_TASKS_NUM];

    srand(time(NULL));

    if (argc != 2) {
        help(argv[0]);
    }

    clear_casio_tasks_config_info(casio_tasks_config, CASIO_TASKS_NUM);


    get_casio_tasks_config_info(argv[1], &duration, casio_tasks_config, &casio_tasks_num);

    sim_time.it_interval.tv_sec = 0;
    sim_time.it_interval.tv_usec = 0;
    sim_time.it_value.tv_sec = duration;
    sim_time.it_value.tv_usec = 0;

    signal(SIGALRM, end_simulation);
    setitimer(ITIMER_REAL, &sim_time, NULL);


    for (i = 0; i < casio_tasks_num; i++) {
        strcpy(arg[0], "casio_task");

        sprintf(arg[1], "%d", casio_tasks_config[i].pid);
        sprintf(arg[2], "%f", casio_tasks_config[i].min_exec);
        sprintf(arg[3], "%f", casio_tasks_config[i].max_exec);
        sprintf(arg[4], "%f", casio_tasks_config[i].min_inter_arrival);
        sprintf(arg[5], "%f", casio_tasks_config[i].max_inter_arrival);
        sprintf(arg[6], "%f", casio_tasks_config[i].deadline);
        sprintf(arg[7], "%f", casio_tasks_config[i].min_offset);
        sprintf(arg[8], "%f", casio_tasks_config[i].max_offset);
        sprintf(arg[9], "%ld", rand());
        n = 10;
        for (k = 0; k < n; k++) {
            //printf("%s  ",arg[k]);
            parg[k] = arg[k];
        }
        parg[k] = NULL;

        casio_tasks_pid[i] = fork();
        if (casio_tasks_pid[i] == 0) {
            execv("./casio_task", parg);
            perror("Error: execv\n");
            exit(0);
        }
        sleep(1);
    }

    start_simulation();  //time zero of the execution
    //waiting for the end of the simulation
    pause();

    for (i = 0; i < casio_tasks_num; i++) {
        wait(NULL);
    }

    printf("All tasks have finished properly!!!\n");
    return 0;

}
