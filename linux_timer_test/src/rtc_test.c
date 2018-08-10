#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#define FREQ 256
#define USEC_PER_SECOND 1000000

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

static int g_fd = 0;


void rtc_open()
{
    g_fd = open ("/dev/rtc0", O_RDONLY);
    if(g_fd < 0) {
        perror("open");
        exit(errno);
    }
    printf("opened.\n");
}

void set_freq()
{
    /* The frequencies 128Hz, 256Hz, ... 8192Hz are only allowed for root. */
    if(ioctl(g_fd, RTC_IRQP_SET, FREQ ) < 0) {
        perror("ioctl(RTC_IRQP_SET)");
        close(g_fd);
        exit(errno);
    }
    /* Enable periodic interrupts */
    if(ioctl(g_fd, RTC_PIE_ON, 0) < 0) {
        perror("ioctl(RTC_PIE_ON)");
        close(g_fd);
        exit(errno);
    }
}

void rtc_task(int msec, int max_exp)
{
    unsigned long i = 0;
    unsigned long data = 0;
    int time_to_wait = msec;

    if(time_to_wait < 0)
        return;

    //calc how many times to loop
    unsigned long cnt = (unsigned long)(time_to_wait * 1000.0 / USEC_PER_SECOND * FREQ + 0.5); //add 0.5 to meet precision in common

    while(max_exp--) {
        for(i = 0; i < cnt; i++) {
            if(read(g_fd, &data, sizeof(unsigned long)) < 0) {
                perror("read");
                ioctl(g_fd, RTC_PIE_OFF, 0);
                close(g_fd);
                exit(errno);
            }
        }
        print_elapsed_time();
    }
    /* Disable periodic interrupts */
    ioctl(g_fd, RTC_PIE_OFF, 0);
    close(g_fd);
}

void create_rtc_timer(int sec, int msec, int max_exp)
{
    rtc_open();
    set_freq();
    rtc_task(sec*1000 + msec, max_exp);
}

int main(int argc, char* argv[])
{
    int sec = 0;
    int msec = 55;
    int max_exp = 100;

	fp = fopen("rtc_resule.txt", "w+");
	if (fp == NULL) {
		printf("open log file failed\n");
		return 0;
	}
	
    printf("\033[32mdefault value: delay 0s 55ms exp 100, \nset value:  %s [interval-secs interval-msecs max-exp]\033[0m\n", argv[0]);

    if (argc == 4) {
        sec = atoi(argv[1]);			// s
        msec = atoi(argv[2]);		// ms
        max_exp = atoi(argv[3]);
    }

    create_rtc_timer(sec, msec, max_exp);

	fflush(fp);
	fclose(fp);

    return 0;
}
