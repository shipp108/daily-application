#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <errno.h>

#define SERVER_PORT       6997
#define BUFFER_SIZE                   1024
#define FILE_NAME_MAX_SIZE            512

struct file_send_data {
    unsigned int file_len;
    unsigned int should_op_len;
    unsigned int op_len_count;
    unsigned char buf[BUFFER_SIZE];
};

struct file_send_data_return {
    int write_len;
    int write_len_count;
    int recv_len;
};

static const unsigned long int crc32tab[] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
	0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
	0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
	0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
	0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
	0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
	0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
	0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
	0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
	0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
	0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
	0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
	0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
	0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
	0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
	0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
	0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
	0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
	0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
	0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
	0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
	0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
	0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
	0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
	0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
	0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
	0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
	0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
	0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
	0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
	0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
	0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
	0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
	0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
	0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
	0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
	0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
	0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
	0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
	0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
	0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
	0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
	0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
	0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
	0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
	0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
	0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
	0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
	0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
	0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
	0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
	0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};

int init(char *ip)
{
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr =  inet_addr(ip);
    client_addr.sin_port = htons(SERVER_PORT);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        printf("Create Socket Failed!\n");
        return -1;
    }

	/* set recv timeout */
	struct timeval timeout = {30, 0};	/* timeout: 5s */
	setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

	/* Disable the Nagle (TCP No Delay) algorithm */
	const char flag = 1;
	setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));

    if (inet_aton(ip, &client_addr.sin_addr) == 0) {
        printf("Server IP Address Error!\n");
        close(client_socket);
        return -3;
    }
    
    socklen_t client_addr_length = sizeof(client_addr);

    if (connect(client_socket, (struct sockaddr*)&client_addr, client_addr_length) < 0) {
        printf("Can Not Connect To %s!\n", ip);
        close(client_socket);
        return -4;
    }
    return client_socket;
}

int requestFile(const char *requestFileName, const int requestFileNameSize, const char *saveFilePath, const int client_socket)
{
	/* send request file name */
    struct file_send_data send_buffer;
    struct file_send_data_return send_ret;
    memset(send_buffer.buf, 0, sizeof(send_buffer.buf));
    strncpy(send_buffer.buf, requestFileName, requestFileNameSize > BUFFER_SIZE ? BUFFER_SIZE : requestFileNameSize);
    send(client_socket, &send_buffer, sizeof(struct file_send_data) - 1, 0);

	/* save file */
	char save_file[FILE_NAME_MAX_SIZE];
	memset(save_file, 0, sizeof(save_file));
    strcat(save_file, saveFilePath);
    char *pSaveFile = strcat(save_file, requestFileName);
	umask(0);
	printf("[%s-%d] save file to: %s\n", __func__, __LINE__, pSaveFile);
    int fileopen = open(pSaveFile, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IROTH );
    if(fileopen < 0) {
        printf("[%s-%d] %m (%s)\n", __func__, __LINE__, pSaveFile);
        return -1;
    }
	
	int recvlen 		= 0;
	int writelen		= 0;
	int writelencount 	= 0;
	int opnumber		= 0;

	do {
		memset(&send_buffer, 0, sizeof(struct file_send_data));
    	memset(&send_ret, 0, sizeof(struct file_send_data_return));
        recvlen = recv(client_socket, &send_buffer, sizeof(struct file_send_data) - 1, 0);
        //printf("[%5d] recv length: %d\n", opnumber++, recvlen);
        if(recvlen < 0) {
        	if (recvlen == -1 && errno == EAGAIN) {
        		printf("[%s-%d] recv timeout\n", __func__, __LINE__);
        	}
            printf("[%s-%d] %m\n", __func__, __LINE__);
            break;
        } 

        if((writelen = write(fileopen, send_buffer.buf, send_buffer.should_op_len)) < 0) {
            printf("[%s-%d] write error!\n", __func__, __LINE__);
            break;
        }
        //printf("%5d <> %5d\n", writelen, send_buffer.should_op_len);
        writelencount += writelen;
        
        send_ret.write_len 			= writelen;
        send_ret.write_len_count 	= writelencount;
        send_ret.recv_len 			= recvlen;

        write(client_socket, &send_ret, sizeof(struct file_send_data_return));
        //printf("[%s-%d] len: %d len_count: %d recv_len: %d\n", __func__, __LINE__, send_ret.write_len, send_ret.write_len_count, send_ret.recv_len);
    } while( writelencount != send_buffer.file_len);
    sync();
	close(fileopen);
	return 0;
}

int filecrc32(const char *path, unsigned long int *crc);

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: ./%s ServerIPAddress\n", argv[0]);
        exit(1);
    }

	/* init socket */
    int client_socket = init(argv[1]);
    if (client_socket < 0) {
    	printf("init socket failed.\n");
    	return 0;
    }

	/* get request file name */
    char file_name[FILE_NAME_MAX_SIZE + 1];
    bzero(file_name, sizeof(file_name));
    
    sprintf(file_name, "%s", "Firmware.img");
    printf("request [%s] start ...\n", file_name);
	requestFile(file_name, strlen(file_name), "./recv/", client_socket);
	printf("request [%s] done  ...\n", file_name);
	
	bzero(file_name, sizeof(file_name));
	sprintf(file_name, "%s", "crc");
	printf("request [%s] start ...\n", file_name);
	requestFile(file_name, strlen(file_name), "./recv/", client_socket);
	printf("request [%s] done  ...\n", file_name);
	
	int ret = checkFileCRC32("./recv/Firmware.img", "./recv/crc");
	
	if (ret == 0) {
		printf("crc32 check success...\n");
	} else {
		printf("crc32 check failed...\n");
	}
	
    close(client_socket);

    return 0;
}

int checkFileCRC32(const char *file, const char *crcFile)
{
	unsigned long int crcTmp = 0;
	unsigned long int crc = 0;
	
	if (access(file, F_OK) != 0 || access(crcFile, F_OK) != 0) {
		printf("Please check the file exists.\n");
		return -2;
	}
	
	FILE *fp = NULL;
	fp = fopen(crcFile, "r");
	if (fp != NULL) {
		fscanf(fp, "%ld", &crc);
		fclose(fp);
		fp = NULL;
		
		filecrc32(file, &crcTmp);
		printf("crc Code: %ld <> %ld\n", crc, crcTmp);
		if (crcTmp == crc) {
			return 0;
		} else {
			return -1;
		}
	} else {
		printf("cannot get a crc code file.\n");
	}
	return -1;
}

int filecrc32(const char *path, unsigned long int *crc)
{
	unsigned char buf[BUFFER_SIZE] = {0};
	unsigned long int size;
	unsigned long int i;
	unsigned long int count = 0;
	unsigned long int tmp;
	FILE *fp = NULL;

	fp = fopen(path, "rb");
	if (fp == NULL) {
		printf("crc open file fault %s\n", path);
		return -1;
	}

	tmp = 0xFFFFFFFF;
	while ((size = fread(buf, 1, BUFFER_SIZE, fp)) > 0) {
		for (i = 0; i < size; i++) {
			tmp = crc32tab[(tmp ^ buf[i]) & 0xff] ^ (tmp >> 8);
		}
		count += size;
	}

	fclose(fp);
	*crc = tmp ^ 0xFFFFFFFF;
	return 0;
}
