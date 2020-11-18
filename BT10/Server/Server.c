/*
 * HoTen: Nguyen The Nam
 * MSSV: 17020916
 * Mota: Server mở cổng 3000 chờ kết nối từ client.
 * 		sau khi kết nối client gửi yêu cầu download bằng tên file
 * 		nếu có file, server xuất tên file và kích thước file ra màn hình
 * 		gửi kích thước file và file cho client nếu không server gửi -1 cho client
 */

#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

#define MAX_SIZE 256
#define USERNAME 20

struct list_client
{
    int sockfd;
    char username[USERNAME];
    struct list_client *next_node;
};

struct list_client *_client_list = NULL;

struct list_client *add_node(int sockfd)
{
    struct list_client *__client_list = _client_list;
    struct list_client *new_node = malloc(sizeof(struct list_client));

    new_node->sockfd = sockfd;
    new_node->username[0] = '\0';
    new_node->next_node = NULL;
    
    while (__client_list && __client_list->next_node)
    {
        __client_list = __client_list->next_node;
    }
    if(__client_list == NULL) {
        _client_list = new_node;
    }else {
        __client_list->next_node = new_node;
    }
    return new_node;
};


struct list_client *search_node(char *receiver) {
    struct list_client *__client_list = _client_list;
    if(receiver == NULL || receiver[0] == '\0') {
        return NULL;
    }
    while (__client_list != NULL)
    {
        if(strcmp(__client_list->username, receiver) == 0) {
            return __client_list;
        }
        __client_list = __client_list->next_node;
    }
    return NULL;
    
}

void remove_node(struct list_client *remove_node)
{
    struct list_client *__client_list, *prev;
	__client_list = _client_list;
	prev = NULL;
	if(__client_list == remove_node) {
		_client_list = __client_list->next_node;
		return;
	}
	while(__client_list != remove_node && __client_list != NULL) {
		prev = __client_list;
		__client_list = __client_list->next_node;
	}
	if(prev && __client_list) {
		prev->next_node = __client_list->next_node;
    }
};

char *get_username(struct list_client *node)
{
	char *str = malloc(29);
	read(node->sockfd, str, 29);
	return 	strrchr(str, ' ') + 1;
}

void send_list_user_online(struct list_client *node) {
    struct list_client *tmp = _client_list;
    char buffer[MAX_SIZE] = {0};
    strcat(buffer, "\n");
    strcat(buffer, "User online:");
    while (tmp)
    {
        if(strcmp(tmp->username, "") != 0) {
            strcat(buffer, "\n");
            strcat(buffer, tmp->username);
        }
        tmp = tmp->next_node;
    }
    write(node->sockfd, buffer, strlen(buffer));
}

void update_list_user_online() {
    struct list_client *tmp = _client_list;
    char buffer[MAX_SIZE];
    memset(buffer, 0, sizeof(buffer));
    strcat(buffer, "\n");
    strcat(buffer, "User online:");
    while (tmp)
    {
        if(strcmp(tmp->username, "") != 0) {
            strcat(buffer, "\n");
            strcat(buffer, tmp->username);
        }
        tmp = tmp->next_node;
    }
    tmp = _client_list;
    while (tmp)
    {
        write(tmp->sockfd, buffer, strlen(buffer));
        tmp = tmp->next_node;
    }
}

void *request(void * args) {
    char buffer[MAX_SIZE] = {0};
	struct list_client *current_client_node, *target_node, *tmpnode;
	char *receiver, *msg, *_tmp, *_msg;
	current_client_node = (struct list_client *)args;
	strcpy(current_client_node->username, get_username(current_client_node));
	printf("New user: %s \n", current_client_node->username);
    send_list_user_online(current_client_node);
    int readlen;
	while(1) {
		memset(buffer, 0, sizeof(buffer));
		readlen = read(current_client_node->sockfd, buffer, sizeof buffer);
        
		if(readlen <= 0 || strncmp(buffer, "EXIT", 4) == 0 || strncmp(buffer, "exit", 4) == 0) {
			remove_node(current_client_node);
            update_list_user_online();
			close(current_client_node->sockfd);
			free(current_client_node);
		}
		if(strncmp(buffer, current_client_node->username, strlen(current_client_node->username)) == 0) {
			_tmp = strchr(buffer, ' ');
			if(_tmp == NULL) {
				continue;
            }
			receiver = _tmp + 1;
			_tmp = strchr(receiver, ' ');
			if(_tmp == NULL) {
				continue;
            }
			*_tmp = '\0';
			msg = _tmp + 1;
			target_node = search_node(receiver);
			if(target_node == NULL) {
                write(current_client_node->sockfd, "USER NOT FOUND", strlen("USER NOT FOUND"));
				continue;
            }
			_msg = malloc(MAX_SIZE);
			if(MAX_SIZE < strlen(current_client_node->username) + strlen(msg) + 2)
				continue;
			sprintf(_msg, "%s: %s", current_client_node->username, msg);
			printf("%s sent message to %s\n", current_client_node->username, target_node->username);
			write(target_node->sockfd, _msg, strlen(_msg) + 1);
			free(_msg);
		}
	}
	close(current_client_node->sockfd);
	return NULL;
}

int main() {
	int sockfd, newfd;
	int reuseaddr = 1;
	pthread_t pthid;
    struct sockaddr_in cliaddr, sockaddr;
	unsigned int cliaddr_len = sizeof(cliaddr);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuseaddr, sizeof(int)) < 0) {
		perror("setsockopt reuseaddr");
    }
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(3000);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) {
		perror("bind");
	}
	if (listen(sockfd, 10) < 0) {
		perror("listen");
	}
	while(1) {
		struct list_client *node;
        newfd = accept(sockfd, (struct sockaddr*) &cliaddr, &cliaddr_len);
        node = add_node(newfd);
        pthread_create(&pthid, NULL, request, (void *)node);
    }
	return 0;
}
