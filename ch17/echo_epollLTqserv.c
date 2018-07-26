#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/epoll.h>

#define BUF_SIZE 4
#define EPOLL_SIZE 50

void error_handling(char *buf);

int main(int argc, char* argv[]){
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;	
	char buf[BUF_SIZE];
	
	socklen_t adr_sz;
	int str_len, i;

	struct epoll_event *ep_events;//用于保存wait的结果
	struct epoll_event event;
	int epfd, event_cnt;

	if(argc != 2){
                printf("Usage : %s <port>\n",argv[0]);
                exit(1);
        }

        serv_sock = socket(PF_INET,SOCK_STREAM,0);
        if(serv_sock == -1){
                error_handling("socket() error");
        }

        memset(&serv_adr,0,sizeof(serv_adr));
        serv_adr.sin_family=AF_INET;
        serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
        serv_adr.sin_port=htons(atoi(argv[1]));

        if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr)) == -1){
                error_handling("bind() error");
	}

	if(listen(serv_sock, 5) == -1){
		error_handling("listen() error");
	}
	epfd = epoll_create(EPOLL_SIZE);//参数其实没用
	ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
/*
    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;

        timeout.tv_sec = 1;
    timeout.tv_usec = 200;
*/	

	event.events = EPOLLIN;
	event.data.fd = serv_sock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);


	while(1){

		event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);//
		puts("return epoll_wait");
		if(event_cnt == -1){
			puts("epoll_wait error");
			break;
		}
/*
        tmps = reads;

        timeout.tv_sec = 1;
        timeout.tv_usec = 200;
		fd_num = select(fd_max + 1, &tmps, 0, 0, &timeout);
		if(fd_num == -1){
			break;
		}
		if(fd_num == 0){
			printf("timeout, continue...\n");
			continue;
		}
*/		
		for(i = 0; i < event_cnt; i++){
			if(ep_events[i].data.fd == serv_sock){//新连接请求
				adr_sz = sizeof(clnt_adr);	
				clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
				event.data.fd = clnt_sock;
				event.events = EPOLLIN;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
				printf("connect client:%d\n", clnt_sock);
					
				}
			else{
				str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);	
				if(str_len == 0)	
				{
					epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
					close(ep_events[i].data.fd);
					printf("close client:%d\n", ep_events[i].data.fd);
				}	
				else{
					write(ep_events[i].data.fd, buf, str_len);
					printf("meg from clnt %d : %s\n", ep_events[i].data.fd, buf);
				}
			}
		}
	}
	close(serv_sock);
	close(epfd);
	return 0;
}

void error_handling(char* buf){
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}
