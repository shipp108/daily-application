#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

void newclient(int sock);
bool readPack(int sock, char* buf, size_t len);
void safe_close(int &sock);

int main(int argc, char *argv[])
{
    int sockfd = -1, newsockfd = -1;
    socklen_t c = 0;
    struct sockaddr_in serv_addr, cli_addr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd) {
        printf("new socket failed. errno: %d, error: %s\n", errno, strerror(errno));
        exit(-1);
    }

    // Prepare the sockaddr_in structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(7890);

    // bind
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("bind failed. errno: %d, error: %s\n", errno, strerror(errno));
        exit(-1);
    }

    // listen
    listen(sockfd, 5);

    printf("listening...\n");
    // accept new connection.
    c = sizeof(struct sockaddr_in);
    int i = 0;
    while (1) {
        printf("waiting for new socket accept.\n");
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, (socklen_t*)&c);
        if (newsockfd < 0) {
            printf("accept connect failed. errno: %d, error: %s\n", errno, strerror(errno));
            safe_close(sockfd);
            exit(-1);
        }
        pid_t pid = fork();
        if (0 == pid) {
            newclient(newsockfd);
            safe_close(sockfd);
            break;
        } else if (pid > 0) {
            safe_close(newsockfd);
        }
    }
    
    safe_close(sockfd);
    return 0;
}

void newclient(int sock)
{
    printf("newclient sock fd: %d\n", sock);
    int datasize = 0;
    const int HEAD_SIZE = 9;
    char buf[512] = {0};
    while (true) {
    	printf("\n=============================================\n");
        memset(buf, 0, sizeof(buf));
        if (! readPack(sock, buf, HEAD_SIZE)) {
            printf("read head buffer failed.\n");
            safe_close(sock);
            return;
        }

        datasize = atoi(buf);
        printf("data size: %s, value:%d\n", buf, datasize);
        
        memset(buf, 0, sizeof(buf));
        if (! readPack(sock, buf, datasize)) {
            printf("read data buffer failed\n");
            safe_close(sock);
            return;
        }
        printf("data size: %d, text: %s\n", datasize, buf);
        if (0 == strcmp(buf, "exit")) {
            break;
        }
    }
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "from server read complete.");
    write(sock, buf, strlen(buf) + 1);
    printf("newclient sockfd: %d, finish.\n", sock);
    safe_close(sock);
}

void safe_close(int &sock)
{
    if (sock > 0) {
        close(sock);
        sock = -1;
    }
}

/**
 * read size of len from sock into buf.
 */
bool readPack(int sock, char* buf, size_t len)
{
    if (NULL == buf || len < 1) {
        return false;
    }
    memset(buf, 0, len); // only reset buffer len.
    ssize_t read_len = 0, readsum = 0;
    do {
        read_len = read(sock, buf + readsum, len - readsum);
        if (-1 == read_len) { // ignore error case
            return false;
        }
        printf("receive data: %s\n", buf + readsum);
        readsum += read_len;
    } while (readsum < len && 0 != read_len);
    return true;
}
