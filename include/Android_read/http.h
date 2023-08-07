#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef __cplusplus
extern "C"{
#endif

char* strstrstr(char* str, char* text, char* rear);//变态级的函数,看了可能会崩溃
char* httppost(char* hostname, char* url, char* cs);//POST方法
char* httpget(const char* hostname, char* url);//GET方法
char* getip(char* hostname);//域名转换IP
int hextoint(char* hex);//16进制字符串转整数

#ifdef __cplusplus
}
#endif


char* httppost(char* hostname, char* url, char* cs){
	// sock句柄
	int sockfd;
	struct sockaddr_in serveraddr;
	//两个是同一个类型，可混用，但是会警告
	//int addrlen = sizeof(serveraddr);
    socklen_t addrlen = sizeof(serveraddr);
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		//printf("创建网络连接失败---socket error!\n");
		return NULL;
	}
	//C4droid的GCC没有这个函数，编译会报错
	//bzero(&serveraddr, addrlen);
	//可以使用这个函数
	memset(&serveraddr,0,addrlen);
	serveraddr.sin_family = AF_INET;
	//端口80
	serveraddr.sin_port = htons(80);
	struct hostent* host;
	host = gethostbyname(hostname);
	if (host == NULL){
		printf("无法解析域名\n");
		close(sockfd);
		return NULL;
	}
	struct in_addr ip = *((struct in_addr *)host->h_addr);
	//printf("ip:%s\n",inet_ntoa(ip));
	serveraddr.sin_addr = ip;
	if (connect(sockfd, (struct sockaddr*) & serveraddr, addrlen) < 0){
		//printf("连接到服务器失败,connect error!\n");
		close(sockfd);
		return NULL;
	}
	//因为懒，所以char[2048]
	//可以通过strlen获取参数长度来动态申请内存
	char postxyt[2048];
	char postsjlen[5];
	// 发送数据
	//32位要用 "%u" 经测试发现还是long最稳定
	//顺利通过 GCC，arm-linux-gcc，arm-linux-gnueabi-gcc，安卓jni 编译 无警告
#if __SIZEOF_LONG__ == 8
	sprintf(postsjlen, "%lu", strlen(cs));
#else
	sprintf(postsjlen, "%u", strlen(cs));
#endif
	memset(postxyt, 0, 2048);
	// 追加字符串（请求协议头）
	//个人感觉strcat函数比较低效
	//用char指针和strcpy函数会比较快
	sprintf(postxyt,"POST /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nUser-Agent: Mozilla/4.0(compatible)\r\nContent-Length: %ld\r\n\r\n%s\r\n\r\n",url,hostname,strlen(cs),cs);
	//别问，问就是 “一切皆文件”
	/*
	if (write(sockfd, postxyt, strlen(postxyt)) == -1){
		//printf("发送失败！错误代码:%d，错误信息:%s\n", errno, strerror(errno));
		close(sockfd);
		return NULL;
	}*/
	if (send(sockfd, postxyt, strlen(postxyt),0) == -1){
		printf("%d，%s\n", errno, strerror(errno));
		close(sockfd);
		return NULL;
	}
	fd_set fds;
	struct timeval tv = { 3,0 }; //select等待3秒，3秒轮询，要非阻塞就置0
	//非阻塞需要循环
	FD_ZERO(&fds); //每次循环都要清空集合，否则不能检测描述符变化 
	FD_SET(sockfd, &fds); //添加描述符

	if (select(sockfd + 1, &fds, NULL, NULL, &tv) < 1){
		//非阻塞时使用continue;
		//-1错误
		//0无写入数据
		close(sockfd);
		return NULL;
	}
	
	if (FD_ISSET(sockfd, &fds)) { //测试sock是否可读，即是否网络上有数据
		char xyt[1024];
		char* xytzz = xyt;
		char* xytmaxlen = xyt + 1023;
		int readlen;
		while (readlen = read(sockfd, xytzz, 1)){
			if(*xytzz == '\n'){
				//printf("匹配到一个\\n\n");
				if(strncmp(xytzz - 3,"\r\n\r",3) == 0){
					*++xytzz = '\0';
					//printf("嗨呀，完全匹配，好开心\n");
					break;
				}
			}
			xytzz++;
			if(xytmaxlen == xytzz){
				//printf("什么吊毛协议头这么长\n");
				close(sockfd);
				return NULL;
			}
		}
		if(!readlen){
			//printf("服务器断开了连接，并未发送任何数据。\n");
			close(sockfd);
			return NULL;
		}
		//printf("%s\n--\n",xyt);
		char* xylen = strstrstr(xyt,"Content-Length: ","\n");
		char* xyzw;
		if(xylen == NULL){
			//如果读取正文长度失败则进入分段读取模式
			char hexlen[8];
			char* hex = hexlen;
			do{
				read(sockfd, hex, 1);
			}while(*hex++ != '\n');
			*hex = '\0';
			int rdlen = hextoint(hexlen);
			if(rdlen == 0){
				close(sockfd);
				return NULL;
			}
			//rdlen++;
			xyzw = (char*)malloc(rdlen + 2);//把\r\n读出来
			read(sockfd, xyzw, rdlen + 2);//方便下面读取区块信息
			//xyzw[rdlen] = '\0';
			//读取下一块内容
			while(1){
				hex = hexlen;
				do{
					read(sockfd, hex, 1);
				}while(*hex++ != '\n');
				*hex = '\0';
				int chlen = hextoint(hexlen);
				//判断当前区块是否为0
				if(chlen == 0)break;
				rdlen += chlen;
				xyzw = (char*)realloc(xyzw,rdlen + 2);
				char* xrzz = xyzw + rdlen - chlen;
				read(sockfd, xrzz, chlen + 2);
				//xyzw[rdlen] = '\0';
			}
			xyzw[rdlen] = '\0';
		}else{
			//得到正文长度，一次性读取 申请内存
			int xyzwlen = atoi(xylen);
			free(xylen);
			//printf("正文共%d字节\n",xyzwlen);
			xyzw = (char*)malloc(xyzwlen + 1);
			readlen = read(sockfd, xyzw, xyzwlen);//接受网络数据
			xyzw[readlen] = '\0';
			//printf("读取了%d字节\n",readlen);
		}
		close(sockfd);
		return xyzw;
	}
	close(sockfd);
	return NULL;
}

char* httpget(const char* hostname, char* url){
	// sock句柄
	int sockfd;
	struct sockaddr_in serveraddr;
	//两个是同一个类型，可混用，但是会警告
	//int addrlen = sizeof(serveraddr);
    socklen_t addrlen = sizeof(serveraddr);
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		//printf("创建网络连接失败---socket error!\n");
		return NULL;
	}
	//C4droid的GCC没有这个函数，编译会报错
	//bzero(&serveraddr, addrlen);
	//可以使用这个函数
	memset(&serveraddr,0,addrlen);
	serveraddr.sin_family = AF_INET;
	//端口80
	serveraddr.sin_port = htons(80);
	struct hostent* host;
	host = gethostbyname(hostname);
	if (host == NULL){
		//printf("无法解析域名\n");
		close(sockfd);
		return NULL;
	}
	struct in_addr ip = *((struct in_addr *)host->h_addr);
	//printf("ip:%s\n",inet_ntoa(ip));
	serveraddr.sin_addr = ip;
	if (connect(sockfd, (struct sockaddr*) & serveraddr, addrlen) < 0){
		//printf("连接到服务器失败,connect error!\n");
		close(sockfd);
		return NULL;
	}
	//因为懒，所以char[2048]
	//可以通过strlen获取参数长度来动态申请内存
	char postxyt[2048];
	memset(postxyt, 0, 2048);
	// 追加字符串（请求协议头）
	//个人感觉strcat函数比较低效
	//用char指针和strcpy函数会比较快
	sprintf(postxyt,"GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n",url,hostname);
	//别问，问就是 “一切皆文件”
	/*
	if (write(sockfd, postxyt, strlen(postxyt)) == -1){
		//printf("发送失败！错误代码:%d，错误信息:%s\n", errno, strerror(errno));
		close(sockfd);
		return NULL;
	}*/
	if (send(sockfd, postxyt, strlen(postxyt),0) == -1){
		printf("%d%s\n", errno, strerror(errno));
		close(sockfd);
		return NULL;
	}
	fd_set fds;
	struct timeval tv = { 3,0 }; //select等待3秒，3秒轮询，要非阻塞就置0
	//非阻塞需要循环
	FD_ZERO(&fds); //每次循环都要清空集合，否则不能检测描述符变化 
	FD_SET(sockfd, &fds); //添加描述符

	if (select(sockfd + 1, &fds, NULL, NULL, &tv) < 1){
		//非阻塞时使用continue;
		//-1错误
		//0无写入数据
		close(sockfd);
		return NULL;
	}
	
	if (FD_ISSET(sockfd, &fds)) { //测试sock是否可读，即是否网络上有数据
		char xyt[1024];
		char* xytzz = xyt;
		char* xytmaxlen = xyt + 1023;
		int readlen;
		while (readlen = read(sockfd, xytzz, 1)){
			if(*xytzz == '\n'){
				//printf("匹配到一个\\n\n");
				if(strncmp(xytzz - 3,"\r\n\r",3) == 0){
					*++xytzz = '\0';
					//printf("嗨呀，完全匹配，好开心\n");
					break;
				}
			}
			xytzz++;
			if(xytmaxlen == xytzz){
				//printf("什么吊毛协议头这么长\n%s\n",xyt);
				close(sockfd);
				return NULL;
			}
		}

		if(!readlen){
			//printf("服务器断开了连接，并未发送任何数据。\n");
			close(sockfd);
			return NULL;
		}
		
		//printf("%s\n--\n",xyt);
		char* xylen = strstrstr(xyt,"Content-Length: ","\n");
		char* xyzw;
		if(xylen == NULL){
			//如果读取正文长度失败则进入分段读取模式
			char hexlen[8];
			char* hex = hexlen;
			do{
				read(sockfd, hex, 1);
			}while(*hex++ != '\n');
			*hex = '\0';
			int rdlen = hextoint(hexlen);
			if(rdlen == 0){
				close(sockfd);
				return NULL;
			}
			//rdlen++;
			xyzw = (char*)malloc(rdlen + 2);//把\r\n读出来
			read(sockfd, xyzw, rdlen + 2);//方便下面读取区块信息
			//xyzw[rdlen] = '\0';
			//读取下一块内容
			while(1){
				hex = hexlen;
				do{
					read(sockfd, hex, 1);
				}while(*hex++ != '\n');
				*hex = '\0';
				int chlen = hextoint(hexlen);
				//判断当前区块是否为0
				if(chlen == 0)break;
				rdlen += chlen;
				xyzw = (char*)realloc(xyzw,rdlen + 2);
				char* xrzz = xyzw + rdlen - chlen;
				read(sockfd, xrzz, chlen + 2);
				//xyzw[rdlen] = '\0';
			}
			xyzw[rdlen] = '\0';
		}else{
			//得到正文长度，一次性读取 申请内存
			int xyzwlen = atoi(xylen);
			free(xylen);
			//printf("正文共%d字节\n",xyzwlen);
			xyzw = (char*)malloc(xyzwlen + 1);
			readlen = read(sockfd, xyzw, xyzwlen);//接受网络数据
			xyzw[readlen] = '\0';
			//printf("读取了%d字节\n",readlen);
		}
		close(sockfd);
		return xyzw;
	}
	close(sockfd);
	return NULL;
}

char* strstrstr(char* str, char* front, char* rear){
	if(!str || !front || !rear)return NULL;//如果你不传NULL,我至于吗
	char* s;
	char* t;
	while(*str) {
		s = str;
		t = front;
		while (*s == *t) {
			s++;
			t++;
			if (!*t) {
				str = s;
				char* old = str;
				do{
					s = str;
					t = rear;
					while (*s == *t) {
						s++;
						t++;
						if (!*t) {
							int charlen = str - old;
							char* newstr = (char*)malloc(charlen + 1);
							strncpy(newstr, old, charlen);
							//使用Visual studio编程时会警告strncpy函数存在风险，使用strncpy_s替换之
							//strncpy_s(newstr, charlen + 1,old, charlen);
							newstr[charlen] = '\0';
							return newstr;
						}
					}
					str++;
				}while(*str);
				return NULL;
			}
		}
		str++;
	}
	return NULL;
}

char* getip(char* hostname) {
	//不多bb，面向百度编程
	struct hostent* host;
	host = gethostbyname(hostname);
	if (host == NULL){
		perror("cannot get host by hostname");
		return NULL;
	}
	return inet_ntoa(*((struct in_addr *)host->h_addr));
}

int hextoint(char * hex){
    int value = 0;
    while (*hex){
        if (*hex >= 'A' && *hex <= 'F')
            value = (*hex - 55) + 16 * value;
        else if (*hex >= 'a' && *hex <= 'f')
            value = (*hex - 87) + 16 * value;
        else if (*hex >= '0' && *hex <= '9')
            value = (*hex - 48) + 16 * value;
        else{
            return value;
        }
        hex++;
    }
    return value;
}
