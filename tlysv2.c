#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#define BUFFSIZE 8092
struct sockaddr_in     servaddr;
int create_socket(int tcp){
	int sock = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1){
		printf("invalid socket !");
		return -1;
	}
	return sock;
}

int bind_socket(int server_sock,int port){
//	struct sockaddr_in     servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
    	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   	servaddr.sin_port = htons(port);
	int rc=-1;
	if( (rc=bind(server_sock, (struct sockaddr*)&servaddr, sizeof(servaddr))) == -1){
    		printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
    		return rc;
    	}else{
		return rc;
	}
}
int tcp_listen(int server_sock){
	int rc = listen(server_sock, 5);
	return rc;
}

int  getfile(int sock,const char *cmd){
	tcp_send(sock,cmd,strlen(cmd));
	
	char buff[256]={0};
	char file[256]={0};
	tcp_recv(sock,buff,sizeof(buff));
	if(strcmp(buff,"fail") == 0){
		printf("%s\n",buff);
		return -1;	
	}
	int len = 0;
	sscanf(buff, "%s %d", file, &len);
	FILE *p = fopen(file, "wb");
	
	if (p&&len){
		char *buf = (char*)malloc(len);
		memset(buf, 0, len);
		tcp_send(sock, "OK", 2);
		int rc = 0;
		while (rc < len){
			int aa = tcp_recv(sock, &buf[rc], len - rc);
			rc += aa;
		}
		fwrite(buf, len, 1, p);
		free(buf);
		fclose(p);
		tcp_recv(sock,buff,strlen(buff));
		printf("%s\n",buff);
		return 0;
		
	}
	else{
		memset(buff, 0, sizeof(buff));
		tcp_recv(sock,buff,sizeof(buff));
			
		printf("%s\n",buff);	
		return -1;
		
	}		
}
int putfile(int sock ,const char *buf){
	char srcfile[256] = {0};
	char desfile[256] = {0};
	sscanf(buf,"put %s %s",srcfile,desfile);
	FILE *p = fopen(srcfile,"rb");
	if(p){
		char cmd[256]={0};
		int len = fseek(p,0,SEEK_END);
		len = ftell(p);
		if(len>0){
			sprintf(cmd,"put %s %d",desfile,len);
			tcp_send(sock,cmd,strlen(cmd));
			memset(cmd,0,sizeof(cmd));
			tcp_recv(sock,cmd,sizeof(cmd));
			if(strcmp(cmd,"OK") == 0){
				char *content=(char*)malloc(len);
				fseek(p,0,SEEK_SET);
				fread(content,len,1,p);
				int rc=0;
				while(rc<len){
					int aa=tcp_send(sock,&content[rc],len-rc);
					rc+=aa;
				}
				free(content);
			}
		}
		fclose(p);
		memset(cmd,0,sizeof(cmd));
		tcp_recv(sock,cmd,sizeof(cmd));
		if(strncmp(cmd,"fail",4)==0){
			printf("%s\n",cmd);
			return -1;	
		}
		printf("%s\n",cmd);
		return 0;
		
		
	}else{
		printf("%s\n","no this file");
		return -1;
	}
}
int tcp_accept(int server_sock,char *ip){

	int len = sizeof(struct sockaddr);	
        int sock  = accept(server_sock, (struct sockaddr*)&servaddr,&len);
	char *p = (char *)inet_ntoa(servaddr.sin_addr);
	strcpy(ip,p);
	return sock;
		
}
int  tcp_send(int sock,char *buf,int len){
	int sd = send(sock, buf, len, 0);
	if (sd < 0){
		return -1;
	}
	else{
		printf("sendsd=%d\n",sd);
		return sd;
	}

}
int tcp_recv(int sock,char *buf,int len){
	int rc = recv(sock, buf, len, 0);
	if (rc < 0){
		return -1;
	}
	else{
		printf("rcv=%d\n",rc);
		return rc;
	}
}

int main(int argc,char *argv[]){

	//建立一个tcp socket
	int server_sock = create_socket(1);
	if(server_sock == -1){

		printf("create error\n");
		return 0;
	}
	//将socket绑定到8080端口
	int rc = bind_socket(server_sock,8080);
	if(rc == -1){
		printf("bind error\n");
		return 0;
	}
	//开始listren
	rc = tcp_listen(server_sock);
	if(rc ==-1){
		printf("listen error\n");
		return 0;
	}
	char ip[100] = {0};
	int sock = tcp_accept(server_sock,ip);
	if(sock<=0){
		printf("accept error\n");
	}
		
	printf("from %s\n",ip);
	char *buf = (char *)malloc(BUFFSIZE);
	
	while(1){


		memset(buf,0,BUFFSIZE);
		fgets(buf,BUFFSIZE,stdin);
		buf[strlen(buf)-1]=0;
		//如果用户输入ls或者exec等代码，执行下面的代码
		if((strncmp(buf,"ls ",3) ==0)||strncmp(buf,"exec ",5)==0){
			
			tcp_send(sock,buf,strlen(buf));
			memset(buf,0,BUFFSIZE);
			tcp_recv(sock,buf,BUFFSIZE);
			printf("buf=%s\n",buf);
		}else if((strncmp(buf,"get ",4)==0)){
			if(getfile(sock,buf) == -1){
				continue;
			}
			
		}else if(strncmp(buf,"put ",4) == 0){
			if(putfile(sock,buf) ==-1){
				continue;
			}
		}else{
			
			continue;
		}
	}
	close(server_sock);

}
