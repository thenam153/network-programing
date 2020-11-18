/*
 * HoTen: Nguyen The Nam
 * MSSV: 17020916
 * Mota: Client nhập địa chỉ IP server rồi kết nối tới server
 * 		sau khi kết nối client gửi tên file tới server và nhận thông báo
 * 		thông báo -1 với không có file và khác là kích thước file
 */
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

#define MAX_SIZE 256
#define USERNAME 20
static char username[USERNAME];

void recvMsg(void *sockfd) {
	char buffer[MAX_SIZE];
	int sfd = *(int*)sockfd;
	int readlen;
	while(1) {
		memset(buffer, 0, sizeof buffer);
		readlen = read(sfd, buffer, sizeof buffer);
		if(readlen < 1) {
            printf("SERVER OUT!");
			exit(1);
        }
		printf("%s\n", buffer);
	}
}

void sendMsg(int sockfd) {
    char buffer[MAX_SIZE];
	char *receiver, *msg, *tmp;

	memset(buffer, 0, sizeof buffer);
	printf("Wellcome to Cchat!");
	while(1) {
        printf("[%s]# ", username);
		fgets(buffer, sizeof buffer, stdin);
		buffer[strlen(buffer) - 1] = '\0';
        if(strncmp(buffer, "exit", 4) == 0) {
			write(sockfd, "exit", 4);
			exit(0);
		}
        write(sockfd, buffer, strlen(buffer));
	}
}

void login(int sockfd) {
    char *message = malloc(29);
    sprintf(message, "USERNAME %s", username);
    write(sockfd, message, strlen(message));
    free(message);
}

int main() {
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

    char* ip_input = "127.0.0.1";
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip_input);
	servaddr.sin_port = htons(3000);

	if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
		perror("connect");
		return 1; 	
	}
    fgets(username, sizeof(username), stdin);
    username[strlen(username) - 1] = '\0';
    login(sockfd);
    pthread_t pthid;
    int thread = pthread_create(&pthid , NULL , (void *)recvMsg , (void *)&sockfd);
    sendMsg(sockfd);
	return 0;
}
