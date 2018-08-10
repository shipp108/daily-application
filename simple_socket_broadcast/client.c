#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define IP_FOUND 		"IP_FOUND"
#define IP_FOUND_ACK 	"IP_FOUND_ACK"
#define IFNAME 			"eth0"
#define MCAST_PORT 		9999

int main(int argc,char*argv[])
{
	int ret 	= -1;
    int sock 	= -1;
    int count 	= -1;
    int so_broadcast = 1;
    
    char buffer[1024];
    
    struct ifreq ifr;
    struct sockaddr_in broadcast_addr;	/* broadcast addr */
    struct sockaddr_in from_addr;		/* server addr */
    int struct_sockaddr_in_len = sizeof(from_addr);

    /* select read fd set */
    fd_set readfd;
    
    /* select timeout param */
    struct timeval timeout;
    timeout.tv_sec 	= 2;
    timeout.tv_usec = 0;
    
    /* construction socket */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("HandleIPFound: sock init error\n");
        return;
    }
    /* copy net interface to ifr.ifr_name,  */
    strncpy(ifr.ifr_name, IFNAME, strlen(IFNAME) + 1);
    
	/* send command, get broadcast address */
    if(ioctl(sock, SIOCGIFBRDADDR, &ifr) == -1) {
        perror("ioctl error");
        return;
    }

	/* copy the broadcast address to broadcast_addr */
    memcpy(&broadcast_addr, &ifr.ifr_broadaddr, sizeof(struct sockaddr_in));
	
	/* set broadcast port */
    printf("broadcast IP is:%s\n", inet_ntoa(broadcast_addr.sin_addr));
    broadcast_addr.sin_family 	= AF_INET;
    broadcast_addr.sin_port 	= htons(MCAST_PORT);
    
    /* set socket attr to support broadcast */
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof(so_broadcast));
	
	/* send broadcast to detect server */
    int i = 0;
    int times = 10;
    for(i = 0; i < times; i++) {
        timeout.tv_sec 	= 2;
        timeout.tv_usec = 0;
        
        /* send broadcast request */
        ret = sendto(sock, IP_FOUND, strlen(IP_FOUND) + 1, 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
        if(ret == -1) {
            continue;
        }

		/* clear select fd set */
        FD_ZERO(&readfd);
        FD_SET(sock,&readfd);
        ret = select(sock + 1, &readfd, NULL, NULL, &timeout);
        
        /* handler select return value */
        switch(ret) {
        case -1:
        	/* select error */
            break;
        case 0:
        	/* select timeout */
            printf("timeout\n");
            break;
        default:
            if(FD_ISSET(sock,&readfd)) {
                count = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&from_addr, &struct_sockaddr_in_len);
                printf("recv server msg: %s\n", buffer);
                if(strstr(buffer, IP_FOUND_ACK)) {
                	/* print server IP */
                    printf("found server IP is:%s\n", inet_ntoa(from_addr.sin_addr));
                    /* print server port */
                    printf("Server Port:%d\n", htons(from_addr.sin_port));
                }
                return;
            }
            break;
        }
    }
    return;
}
