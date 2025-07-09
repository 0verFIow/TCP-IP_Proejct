#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>   
#include <sys/wait.h>
#include <pthread.h>

#define MAX 10

typedef struct{
	int fd;
	char name[128];
	struct sockaddr_in addr;
	int active;
	pthread_t tid;
} client_info_t;

client_info_t clients[MAX];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * server_function (void * args) {
	client_info_t *client = (client_info_t *)args;
	int fd = client->fd;
	int bytes_recv;
	char tx_buf[128], rx_buf[128];

	//entry_alarm
	pthread_mutex_lock(&mutex);
	snprintf(tx_buf, sizeof(tx_buf), "[%s] has joined the chat!\n", client->name);
	for (int i = 0; i < MAX; i++) {
		if (clients[i].active && clients[i].fd != fd) {
			send(clients[i].fd, tx_buf, strlen(tx_buf), 0);
		}
	}
	pthread_mutex_unlock(&mutex);

	for(int i=1; ; i++) {
		memset(tx_buf, 0, sizeof(tx_buf));
		memset(rx_buf, 0, sizeof(rx_buf));
        // recv = 소켓을 통해 데이터를 수신하는 함수
        // fd = 클라이언트와 연결된 소켓
		if((bytes_recv = recv(fd, rx_buf, sizeof(rx_buf), 0)) == -1){
			perror("Recv");
			break;
		}

		// client가 접속 종료했을 때
   if(bytes_recv == 0)
     {
			pthread_mutex_lock(&mutex);
			client->active = 0;
			//exit_alarm
			snprintf(tx_buf, sizeof(tx_buf), "[%s] has left the chat!\n", client->name);
      for (int i = 0; i < MAX; i++) {
          if (clients[i].active && clients[i].fd != fd) {
              send(clients[i].fd, tx_buf, strlen(tx_buf), 0);
          }
      }
      pthread_mutex_unlock(&mutex);
			close(fd);
            break;
      }
		
		char rx_buf_with_name[256];
		sprintf(rx_buf_with_name,"%s >> %s",client->name,rx_buf);
        pthread_mutex_lock(&mutex);
		for (int i = 0; i < MAX; i++) {
        if (clients[i].active && clients[i].fd != fd) {
            send(clients[i].fd, rx_buf_with_name, strlen(rx_buf_with_name), 0);
        }
			  }
			  pthread_mutex_unlock(&mutex);

				printf("%s(%s) : %s", client->name,inet_ntoa(client->addr.sin_addr),rx_buf);
		}
	return NULL;
}

int main(void){
	struct sockaddr_in server_addr;
	int sock_size;
	int server_sfd;
	int yes = 1;
	pthread_t tid;

	if((server_sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket() error");
		exit(1);
	}

	if(setsockopt(server_sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
		perror("setsockopt() error");
		exit(1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(12000); //server port number setting
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&(server_addr.sin_zero), '\0', 8);

	// bind = 소켓에 주소(IP와 포트)를 할당 해주는 함수 
	if(bind(server_sfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1){
		perror("bind() error");
		exit(1);
	}

	//client backlog setting
	if(listen(server_sfd, 5) == -1){
		perror("listen() error");
		exit(1);
	}

	printf("start server\n");
	while(1){
		sock_size = sizeof(struct sockaddr_in);

		int idx = -1;
		for(int i=0;i<MAX;i++)
		{
			if(clients[i].active == 0)
			{
				idx=i;
				break;
			}
		}
		if (idx == -1)
		{
			perror("full\n");
			exit(1);
		}
		clients[idx].fd = accept(server_sfd, (struct sockaddr *)&clients[idx].addr, &sock_size);
		clients[idx].active = 1;

		recv(clients[idx].fd,clients[idx].name,128,0);
		pthread_create(&clients[idx].tid, NULL, server_function, &clients[idx]);
	}
	return 0;
}
