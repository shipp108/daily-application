#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
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


void create_select_timer(int sec, int msec, int max_exp)
{
    struct timeval tv;
    while(max_exp--) {
        tv.tv_sec = sec;
        tv.tv_usec = msec * 1000;

        // Wait
        select(0,NULL,NULL,NULL,&tv);

        // do some code
        print_elapsed_time();
    }
}

int main(int argc, char **argv)
{


    int sec = 0;
    int msec = 55;
    int max_exp = 100;

	fp = fopen("select_resule.txt", "w+");
	if (fp == NULL) {
		printf("open log file failed\n");
		return 0;
	}

    printf("\033[32mdefault value: delay 0s 55ms exp 100, \nset value:  %s [interval-secs interval-msecs max-exp]\033[0m\n", argv[0]);

    if (argc == 4) {
        sec = atoi(argv[1]);
        msec = atoi(argv[2]);
        max_exp = atoi(argv[3]);
    }

    create_select_timer(sec, msec, max_exp);

	fflush(fp);
	fclose(fp);
    return 0;
}
