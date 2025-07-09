#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

volatile int running = 1;

void * read_function(void * args)
{
    int bytes_recv;
    int sockfd = *((int*)args); // 소켓 번호의 주소가 sockfd에 등록
	printf("client_read_소켓값 : %d\n",sockfd);
    char rx_buf[128];
    for(;;)
    {
        memset(rx_buf,0,sizeof(rx_buf));
        if((bytes_recv = recv(sockfd, rx_buf, sizeof(rx_buf), 0)) == -1){
			perror("recv");
			exit(1);
		}

        if(bytes_recv <= 0)
        {
            perror("close\n");
			running = 0;
            break;
        }
		rx_buf[bytes_recv] = '\0';
        printf("%s",rx_buf);
    }
	return NULL;
}

void * write_function(void * args)
{
	int sockfd = *((int*)args); // 소켓 번호의 주소가 sockfd에 등록
	char tx_buf[128];
	printf("client_write_소켓값 : %d\n",sockfd);
	while (running && fgets(tx_buf,sizeof(tx_buf),stdin))
	{
		send(sockfd,tx_buf,strlen(tx_buf),0);
	}
	return NULL;
}


int main(int argc, char *argv[]){
	int sockfd, bytes_recv; 
	struct sockaddr_in sockaddr;
	pthread_t tid1,tid2;
	char tx_buf[128], rx_buf[128],name[128];

	if(argc != 2){
		fprintf(stderr, "usage : client serverip \n");
		exit(1);
	}

	//socket open
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket() error");
		exit(1);
	}
	printf("sockfd = %d\n",sockfd);

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(12000);
    //s_addr -> 실제 IP 주소값을 담는 멤버
	sockaddr.sin_addr.s_addr = inet_addr(argv[1]);    
	/*  sockaddr.sin_addr.s_addr = inet_addr("70.12.117.90");  */
	memset(&(sockaddr.sin_zero), '\0',8);

    // inet_ntoa = 숫자형 IPv4 주소를 점으로 구분된 문자열로 바꿔주는 함수
	printf("[ %s ]\n", inet_ntoa(sockaddr.sin_addr));

	//connection request to server
    // connect -> 클라이언트 소켓이 서버 소켓에 연결을 요청하는 함수
	if(connect(sockfd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr)) == -1){
		perror("connect() error");
		exit(1);
	}
	else
	{
		//inet_ntoa -> ipv4 주소를 문자열로 변환해주는 함수, ntohs -> 네트워크에서 받은 숫자를 컴퓨터에서 읽을 수 있는 데이터로 변환
		printf("server ip : %s, port : %d\n",inet_ntoa(sockaddr.sin_addr), ntohs(sockaddr.sin_port));
		
		// client가 사용할 닉네임을 서버로 전송
		printf("사용할 이름을 입력하세요 : ");
		fgets(name,sizeof(name),stdin);
		name[strlen(name)-1] = '\0';
		send(sockfd,name,strlen(name),0);
	}

	pthread_create(&tid1, NULL, read_function,&sockfd);
	pthread_create(&tid2, NULL, write_function,&sockfd);
	pthread_join(tid1,NULL);
	printf("read 스레드 종료\n");
	pthread_join(tid2,NULL);
	printf("write 스레드 종료\n");
	close(sockfd);
	printf("모든 소켓 종료\n");
	return 0;
}
