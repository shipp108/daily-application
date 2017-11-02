#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/wireless.h>

/*
获取无线网卡的连接状态
(/proc/net/wireless的status)
return 0: 未连接
return 1: 已连接
*/

/* gcc getWifiStatus.c -o getWifiStatus */

int get_wireless_if_status(char *ath)
{
    int sock_fd;
    int ret = 0;
    struct iwreq iwr;
    struct iw_statistics stats;

    if (NULL == ath)
    {
        printf("ath is NULL/n");
        return -1;
    }

    /* make socket fd */
    if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("socket err/n");
        return -1;
    }

    /* make iwreq */
    memset(&iwr, 0, sizeof(iwr));
    memset(&stats, 0, sizeof(stats));
    iwr.u.data.pointer = (caddr_t) &stats; /* result value */
    iwr.u.data.length = sizeof(stats);
    iwr.u.data.flags = 1; /* clear updated flag */

    /* ifname is reqired */
    strncpy(iwr.ifr_ifrn.ifrn_name, ath, IFNAMSIZ - 1);

    /* get SIOCGIWSTATS */
    if (ioctl(sock_fd, SIOCGIWSTATS, &iwr) < 0)
    {
        printf("No Such Device %s/n",ath);
        close(sock_fd);
        return -1;
    }

    ret = stats.status;

    close(sock_fd);
    return ret;
}

int main()
{
	if (get_wireless_if_status("wlan0") != 0) {
		printf("wlan0 connect failed!\n");
	} else {
		printf("wlan0 connect success!\n");
	}
	return 0;
}
