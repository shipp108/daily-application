#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IP_FOUND 		"IP_FOUND"
#define IP_FOUND_ACK 	"IP_FOUND_ACK"
#define PORT 			9999

int main(int argc,char*argv[])
{
    int ret = -1;
    int sock = -1;
    int count = -1;
    char buffer[1024];
    
    struct sockaddr_in server_addr;	/* server ip */
    struct sockaddr_in from_addr;	/* client ip */
    int struct_sockaddr_in_len = sizeof(struct sockaddr_in);
    
    /* select read fd set */
    fd_set readfd;
    
    /* select timeout param */
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    
    /* construction socket */
    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0) {
        perror("sock error");
        return;
    }

    memset((void*)&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family 		= AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port 		= htons(PORT);
	
    ret = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0) {
        perror("bind error");
        return;
    }

    while(1) {
    	/* set select timeout param */
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        
		/* set slect readfd */
        FD_ZERO(&readfd);
        FD_SET(sock, &readfd);
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
            if(FD_ISSET(sock, &readfd)) {
            	/* recv client data */
                count = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&from_addr, &struct_sockaddr_in_len);
                printf("recv client msg: %s\n", buffer);
                /* from "from_addr" struct to save client info  */
                if(strstr(buffer, IP_FOUND)) {
                    /* print client IP */
                    printf("Client IP is %s\n", inet_ntoa(from_addr.sin_addr));
                    /* print client port */
                    printf("Client Send Port: %d\n", ntohs(from_addr.sin_port));
                    memcpy(buffer, IP_FOUND_ACK, strlen(IP_FOUND_ACK) + 1);
                    /* send data to client */
                    count = sendto(sock, buffer, strlen(buffer) + 1, 0, (struct sockaddr*)&from_addr, struct_sockaddr_in_len);
                }
                return;
            }
            break;
        }
    }
    return;
}
