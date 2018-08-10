#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void safe_close(int &sock);
void emulate_subpackage(int sock);
void emulate_adheringpackage(int sock);

int main(int argc, char *argv[])
{
    char buf[128] = {0};
    int sockfd = -1;
    struct sockaddr_in serv_addr;

    // Create sock
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd) {
        printf("new socket failed. errno: %d, error: %s\n", errno, strerror(errno));
        exit(-1);
    }

    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(7890);

    // Connect to remote server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("connection failed. errno: %d, error: %s\n", errno, strerror(errno));
        exit(-1);
    }
    printf("\n======================================\n");
    emulate_subpackage(sockfd);
    printf("\n======================================\n");
    emulate_adheringpackage(sockfd);
	printf("\n======================================\n");
	
    const int HEAD_SIZE = 9;
    const char temp[] = "exit";
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%0.*zu", HEAD_SIZE - 1, sizeof(temp));
    
    write(sockfd, buf, HEAD_SIZE);
    write(sockfd, temp, sizeof(temp));

	printf("\n======================================\n");

    printf("send complete.\n");
    memset(buf, 0, sizeof(buf));
    read(sockfd, buf, sizeof(buf));
    printf("receive data: %s\n", buf);
    printf("client finish.\n");

    safe_close(sockfd);
    return 0;
}

void safe_close(int &sock)
{
    if (sock > 0) {
        close(sock);
        sock = -1;
    }
}

/**
 * emulate socket data write multi part.
 */
void emulate_subpackage(int sock)
{
    printf("emulate_subpackage...\n");
    char text[] = "This is a test case for client send subpackage data. data is not send complete at once.";
    const size_t TEXTSIZE = sizeof(text);
    ssize_t len = 0;
    size_t sendsize = 0, sendsum = 0;

    const int HEAD_SIZE = 9;
    char buf[64] = {0};
    snprintf(buf, HEAD_SIZE, "%08zu", TEXTSIZE);
    write(sock, buf, HEAD_SIZE);
    printf("send data size: %s\n", buf);

    do {
        sendsize = 6;
        if (sendsum + sendsize > TEXTSIZE) {
            sendsize = TEXTSIZE - sendsum;
        }
        len = write(sock, text + sendsum, sendsize);
        if (-1 == len) {
            printf("send data failed. errno: %d, error: %s\n", errno, strerror(errno));
            return;
        }
        memset(buf, 0, sizeof(buf));
        snprintf(buf, len + 1, text + sendsum);
        printf("send data: %s\n", buf);
        sendsum += len;
        //sleep(1);
    } while (sendsum < TEXTSIZE && 0 != len);
}

/**
 * emualte socket data write adhering.
 */
void emulate_adheringpackage(int sock)
{
    printf("emulate_adheringpackage...\n");
    const int HEAD_SIZE = 9;
    char buf[1024] = {0};
    char text[128] = {0};
    char *pstart = buf;

    // append text
    memset(text, 0, sizeof(text));
    snprintf(text, sizeof(text), "Hello ");
    snprintf(pstart, HEAD_SIZE, "%08zu", strlen(text) + 1);
    pstart += HEAD_SIZE;
    snprintf(pstart, strlen(text) + 1, "%s", text);
    pstart += strlen(text) + 1;

    // append text
    memset(text, 0, sizeof(text));
    snprintf(text, sizeof(text), "I'm lucky.");
    snprintf(pstart, HEAD_SIZE, "%08zu", strlen(text) + 1);
    pstart += HEAD_SIZE;
    snprintf(pstart, strlen(text) + 1, "%s", text);
    pstart += strlen(text) + 1;

    // append text
    memset(text, 0, sizeof(text));
    snprintf(text, sizeof(text), "Nice too me you");
    snprintf(pstart, HEAD_SIZE, "%08zu", strlen(text) + 1);
    pstart += HEAD_SIZE;
    snprintf(pstart, strlen(text) + 1, "%s", text);
    pstart += strlen(text) + 1;
    write(sock, buf, pstart - buf);
}
