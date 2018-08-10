#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>        /* Definition of uint64_t */

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

FILE *fp = NULL;
static void
print_elapsed_time(void)
{
    static struct timespec start;
    struct timespec curr;
    static int first_call = 1;
    int secs, nsecs;
    static int counter = 0;
    if (first_call) {
        first_call = 0;
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
            printf("clock_gettime error\n");
            exit(1);
        }
        return;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1) {
        printf("clock_gettime error\n");
        exit(1);
    }

    secs = curr.tv_sec - start.tv_sec;
    nsecs = curr.tv_nsec - start.tv_nsec;
    start = curr;
    if (nsecs < 0) {
        secs--;
        nsecs += 1000000000;
    }
    printf("(delta: %4ds %4dms) counter %4d\n", secs, (nsecs + 500000) / 1000000, counter++);
    fprintf(fp, "%lf\n", secs + nsecs/1000000000.0);
}

volatile int  max_exp = 0;

void timer_handler (int signum)
{
	max_exp--;
    print_elapsed_time();    
}

void create_setitimer(int sec, int msec, int _max_exp)
{
    struct sigaction sa;
    struct itimerval timer;

    /* Install timer_handler as the signal handler for SIGVTALRM. */
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;
    sigaction (SIGVTALRM, &sa, NULL);

    /* Configure the timer to expire after m s n ms... */
    timer.it_value.tv_sec = sec;
    timer.it_value.tv_usec = msec * 1000;
    /* ... and every m s n ms after that. */
    timer.it_interval.tv_sec = sec;
    timer.it_interval.tv_usec = msec * 1000;


    /* Start a virtual timer. It counts down whenever this process is
      executing. */
    setitimer (ITIMER_VIRTUAL, &timer, NULL);
    
    while(max_exp > 0) {
    	;
    }
}

int main (int argc, char **argv)
{
    int sec = 0;
    int msec = 55;
    max_exp = 100;

	fp = fopen("setitimer_resule.txt", "w+");
	if (fp == NULL) {
		printf("open log file failed\n");
		return 0;
	}

    printf("\033[32mdefault value: delay 0s 55ms exp 100, \nset value:  %s [interval-secs interval-msecs max-exp]\033[0m\n", argv[0]);

    if (argc == 4) {
        sec = atoi(argv[1]);			// s
        msec = atoi(argv[2]);			// ms
        max_exp = atoi(argv[3]);
    }

    create_setitimer(sec, msec, max_exp);

	fflush(fp);
	fclose(fp);
    return 0;
}
