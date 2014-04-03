#include "netlib/net.h"
#include "netlib/defaults.h"
#include "utils/debug.h"
#include "utils/utils.h"
#include "server.h"

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

void *attendClient(void *ptr);
void *attendCallings(void *ptr);
char *createResponse(char* command);

char *serverName;
int tport;

int start_server(const char *iface, const unsigned short tcpport, const unsigned short udpport, const unsigned short maxClients, const char *name) 
{
	int serverTCPSocket;
	int serverUDPSocket;
	int clientTCPSocket;
	pthread_t thread;
	
	char clientIP[18];
	unsigned int clientPort;

	tport = tcpport;
	serverName = (char *) calloc(1,255);
	strcpy(serverName,name);

	serverTCPSocket = newSocket(iface,tcpport,maxClients,TCPLISTENER);
	serverUDPSocket = newSocket(iface,udpport,0,UDPLISTENER);
	
	if(serverTCPSocket == -1 || serverUDPSocket == -1) 
	{
		debug(4,"Sockets can't be created");
		return false;
	}

	pthread_create(&thread, NULL, attendCallings, (void *)&serverUDPSocket);
	
	while(1)
	{
		memset(clientIP,0,sizeof(clientIP));
		clientPort = 0;
			
		debug(5,"%s","Waiting for a Client...");
		clientTCPSocket = waitConnection(serverTCPSocket,clientIP,&clientPort);
		debug(4,"Connected Client %s:%u",clientIP,clientPort);

		pthread_create(&thread, NULL, attendClient, (void *)&clientTCPSocket);
	}


	closeSocket(serverTCPSocket);	
	return true;
	
}

void *attendClient(void *ptr)
{
	int clientSocket;
	char *buffer;
	char *temp;
	int end;

	end = 1;

	clientSocket = *((int *)ptr);
	buffer = (char*) calloc (1,BUFFERSIZE);
	temp = (char*) calloc (1,BUFFERSIZE);

	while(end)
	{
		memset(buffer,0,BUFFERSIZE);
		memset(temp,0,BUFFERSIZE);
		recvfrom(clientSocket, buffer, BUFFERSIZE, 0, NULL, 0);
		debug(4,"Received: %s",buffer);
		buffer = createResponse(buffer);
		sendto(clientSocket, buffer, strlen(buffer), 0, NULL, 0);
		debug(4,"Sent: %s",buffer);
	}

	return NULL;
}

void *attendCallings(void *ptr)
{
	int serverSocket;
	int clientPort;
	char *buffer;
	char clientIp[18];
	struct sockaddr_in clientAddr;
	socklen_t addrlen = sizeof(clientAddr);

	serverSocket = *((int *)ptr);
	buffer = (char*) calloc (1,BUFFERSIZE);

	while(true) 
	{
		memset(buffer,0,BUFFERSIZE);
		debug(4,"Waiting for host discovers");
		recvfrom(serverSocket, buffer, BUFFERSIZE, 0, (struct sockaddr*)&clientAddr, &addrlen );

		inet_ntop(AF_INET,&(clientAddr.sin_addr),clientIp,INET_ADDRSTRLEN);
		clientPort = ntohs(clientAddr.sin_port);

		debug(4,"Received: [%s:%i] %s\n",clientIp,clientPort,buffer);

		//Manage Message
			//NO NEED OF MANAGE
		//Answer
		memset(buffer,0,BUFFERSIZE);
		snprintf(buffer, BUFFERSIZE, "Hi %s\r\nPuerto: %i\r\nArchivos: %i\r\n\r\n", serverName,tport,0);	

		sendto(serverSocket, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, addrlen);

		debug(4,"Sent: [%s:%i] %s\n",clientIp,clientPort,buffer);

   	    fflush(stdout);
	}

	return NULL;
}

char *createResponse(char* command)
{
	char *buffer;
	char *temp;
	char *response;

	char **args;
	char *cmd;

	int files;
	char *list;

	buffer = (char*) calloc (1,BUFFERSIZE);
	temp = (char*) calloc (1,BUFFERSIZE);
	response = (char*) calloc (1,BUFFERSIZE);
	args = (char**) calloc (255,BUFFERSIZE);

	memset(buffer,0,BUFFERSIZE);
	memset(temp,0,BUFFERSIZE);
	memset(response,0,BUFFERSIZE);
	memset(args,0,255*BUFFERSIZE);

	strcpy(temp,command);

	cmd = getCommand(command,args);
	debug(5,"CMD: %s",cmd);

	if(strcmp(cmd,"PING")==0)
	{
		debug(3,"CMD: PING");
		snprintf(response, BUFFERSIZE, "PONG \r\n\r\n");
		snprintf(buffer, BUFFERSIZE, "OK %s%s", temp,response);
	}
	else if (strcmp(cmd,"FILELIST")==0)
	{
		debug(3,"CMD: FILELIST");
		files = fileList("archive", list);
		snprintf(response, BUFFERSIZE, "%sCantidad: %d\r\n\r\n",list,files);
		snprintf(buffer, BUFFERSIZE, "OK %s%s", temp,response);
	}
	else if (strcmp(cmd,"BYE")==0)
	{
		debug(3,"CMD: BYE");
		snprintf(response, BUFFERSIZE, "BYE \r\n\r\n");
		snprintf(buffer, BUFFERSIZE, "OK %s%s", temp,response);
	}
	else
	{
		debug(3,"Unknown command");
		snprintf(buffer, BUFFERSIZE, "FAIL %s", temp);
	}

	

	return buffer;
}

