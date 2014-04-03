/*
 *
 */

#include "../utils/debug.h"
#include "defaults.h"

#include <stdlib.h> 
#include <stdio.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h> 

int newSocket(const char *ip, const unsigned short port, const int q_size, const int type);
int buildAddr(struct sockaddr_in *addr, const char *ip, const unsigned short port);
void closeSocket(const int socketFD);
int getNewSocket(const int domain, int type);
int waitConnection(int socket, char *clientIP, unsigned int *clientPort);
int readfromSocket(const int socket, char *buffer, const unsigned int maxRead);
int sendtoSocket(const int socket, char *buffer, const unsigned int size);

int newSocket(const char *ip, const unsigned short port, const int q_size, const int type) 
{
	int socketFD;
	int status;
	int localerror;
	int protocol;
	struct sockaddr_in addr;

	debug(5,"Type: %i", type);

	switch(type)
	{
	    case TCPLISTENER:
	    case TCPSENDER:
			protocol = SOCK_STREAM;
			break; /* optional */
	  
	    case UDPLISTENER:
	    case UDPSENDER:
			protocol = SOCK_DGRAM;
			break; /* optional */
	    default:
	    	protocol = 0;
	    	break;
	}

	if(protocol == SOCK_STREAM)
	{
		debug(5,"TCP Socket");
	}
	else if(protocol == SOCK_DGRAM)
	{
		debug(5,"UDP Socket");
	}
	else
	{
		debug(5,"Unknown Socket");
	}

	if(!buildAddr(&addr,ip,port)) return -1;
	
	if((socketFD = getNewSocket(PF_INET,protocol))==-1) return -1;
	
	if(type == TCPLISTENER || type == UDPLISTENER)
	{
		status = bind(socketFD, (struct sockaddr*)&addr, sizeof(addr));
		
		if(status != 0) 
		{
			localerror = errno;
			fprintf(stderr,"Error: Can't bind port %s:%i (%s)\n",ip,port,strerror(localerror));
			return -1;
		}

		if(type == TCPLISTENER)
		{
			status = listen(socketFD,q_size);

			if(status != 0) 
			{
				localerror = errno;
				fprintf(stderr,"Error: Can't change socket mode to listen (%s)\n",strerror(localerror));
				return -1;
			}
		}

		debug(4,"Socket on %s:%u created",ip,port);
	}
	else if(type == TCPSENDER)
	{
		status = connect(socketFD, (struct sockaddr*)&addr, sizeof(addr));

		if(status == -1) 
		{
			localerror = errno;
			fprintf(stderr,"Can't connect to %s:%i (%s)",ip,port,strerror(localerror));
			return -1;
		}
		
		debug(4,"Connected to %s:%i",ip,port);
	}
	
	return socketFD;
}

int buildAddr(struct sockaddr_in *addr, const char *ip, const unsigned short port) 
{
	int status;
	int localerror;

	memset(addr,0,sizeof(sockaddr_in));

	addr->sin_family = AF_INET;

	status = inet_pton(AF_INET,ip,&(addr->sin_addr.s_addr));

	if(status == 0) {
		fprintf(stderr,"Invalid IPv4 Address\n");
		return -1;
	} else if(status == -1) {
		localerror = errno;
		fprintf(stderr,"Error on IP Address (%s)\n",strerror(localerror));
		return -1;
	}
	
	addr->sin_port = htons(port);
	
	return -1;	
}

void closeSocket(const int socketFD) 
{
	close(socketFD);
	debug(4,"Socket(%i) closed",socketFD);
	return;
}

int getNewSocket(const int domain, int type) 
{
	int socketFD;
	int localerror;
	
	socketFD = socket(domain,type,0);
	if(socketFD == -1) {
		localerror = errno;
		fprintf(stderr,"Can't create socket (%s)\n",strerror(localerror));
		return -1;
	}
	
	return socketFD;
}

int waitConnection(int socket, char *clientIP, unsigned int *clientPort) 
{
	int client;
	struct sockaddr_in addrClient;
	socklen_t addrLen;
	char ip[INET_ADDRSTRLEN]={0};
	int localerror;
	
	addrLen = sizeof(addrClient);
	
	memset(&addrClient,0,sizeof(addrClient));

	client = accept(socket, (struct sockaddr *)&addrClient,&addrLen);

	if(client == -1) {
		localerror = errno;
		fprintf(stderr,"Can't retrive client Socket (%s)\n",strerror(localerror));
		return -1;
	}
	
	if(clientIP!=NULL) {
		inet_ntop(AF_INET,&(addrClient.sin_addr),ip,INET_ADDRSTRLEN);
		strcpy(clientIP,ip);		
	}
	
	if(clientPort!=NULL) {
		*clientPort = ntohs(addrClient.sin_port);
	}
	
	return client;
}


