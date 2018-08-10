
#include <pthread.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
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

volatile int max_exp = 0;
/*
 * Thread start routine to notify the application when the
 * timer expires. This routine is run "as if" it were a new
 * thread, each time the timer expires.
 *
 * When the timer has expired 5 times, the main thread will
 * be awakened, and will terminate the program.
 */
void timer_thread(union sigval arg)
{
    max_exp--;
    print_elapsed_time();
}
timer_t timer_id;
void create_timer(int sec, int msec, int _max_exp)
{

    int status;
    struct itimerspec ts;
    struct sigevent se;

    /*
     * Set the sigevent structure to cause the signal to be
     * delivered by creating a new thread.
     */
    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = &timer_id;
    se.sigev_notify_function = timer_thread;
    se.sigev_notify_attributes = NULL;

    ts.it_value.tv_sec = sec;
    ts.it_value.tv_nsec = msec * 1000000;
    ts.it_interval.tv_sec = sec;
    ts.it_interval.tv_nsec = msec * 1000000;

    status = timer_create(CLOCK_REALTIME, &se, &timer_id);
    if (status == -1) {
        perror("Create timer");
        exit(EXIT_FAILURE);
    }

    status = timer_settime(timer_id, 0, &ts, 0);
    if (status == -1) {
        perror("Set timer");
        exit(EXIT_FAILURE);
    }
    
       /* Do busy work. */
    while (max_exp > 0) {
 		;
    }
    timer_delete(timer_id);
}

int main(int argc, char **argv)
{
    int sec = 0;
    int msec = 55;
    max_exp = 100;


	fp = fopen("timer_resule.txt", "w+");
	if (fp == NULL) {
		printf("open log file failed\n");
		return 0;
	}
	
    printf("\033[32mdefault value: delay 0s 55ms exp 100, \nset value:  %s [interval-secs interval-msecs max-exp]\033[0m\n", argv[0]);

    if (argc == 4) {
        sec = atoi(argv[1]);
        msec = atoi(argv[2]);
        max_exp= atoi(argv[3]);
    }

    create_timer(sec, msec, max_exp);

 

	fflush(fp);
	fclose(fp);

    return 0;
}
