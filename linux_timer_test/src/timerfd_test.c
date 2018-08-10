#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>        /* Definition of uint64_t */

FILE *fp = NULL;
int max_exp = 0;

static void
print_elapsed_time(void)
{
    static struct timespec start;
    struct timespec curr;
    static int first_call = 1;
    int secs, nsecs;

    if (first_call) {
        first_call = 0;
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
            perror("clock_gettime");
            exit(EXIT_FAILURE);
        }
        return;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    secs = curr.tv_sec - start.tv_sec;
    nsecs = curr.tv_nsec - start.tv_nsec;
    start = curr;
    if (nsecs < 0) {
        secs--;
        nsecs += 1000000000;
    }
    printf("(delta: %4ds %4dms): ", secs, (nsecs + 500000) / 1000000);
    fprintf(fp, "%lf\n", secs + nsecs/1000000000.0);
}

void create_timerfd_timer(int sec, int msec, int max_exp)
{
    struct itimerspec new_value;
    int fd;
    struct timespec now;
    uint64_t exp, tot_exp;
    ssize_t s;


    if (clock_gettime(CLOCK_REALTIME, &now) == -1) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    /* Create a CLOCK_REALTIME absolute timer with initial
      expiration and interval as specified in command line */

    new_value.it_value.tv_sec = now.tv_sec;
    new_value.it_value.tv_nsec = now.tv_nsec;

    new_value.it_interval.tv_sec = sec;
    new_value.it_interval.tv_nsec = msec * 1000000;

    fd = timerfd_create(CLOCK_REALTIME, 0);
    if (fd == -1) {
        perror("timerfd_create");
        exit(EXIT_FAILURE);
    }


    if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1) {
        perror("timerfd_settime");
        exit(EXIT_FAILURE);
    }

    print_elapsed_time();
    printf("timer started\n");

    for (tot_exp = 0; tot_exp < (uint64_t)max_exp;) {
        s = read(fd, &exp, sizeof(uint64_t));
        if (s != sizeof(uint64_t)) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        tot_exp += exp;
        print_elapsed_time();
        printf("read: %llu; total=%llu\n",
               (unsigned long long) exp,
               (unsigned long long) tot_exp);
    }
}

int
main(int argc, char *argv[])
{

    int sec = 0;
    int msec = 55;
    int max_exp = 100;

	fp = fopen("timerfd_resule.txt", "w+");
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

    create_timerfd_timer(sec, msec, max_exp);

	fflush(fp);
	fclose(fp);
    return 0;
}
