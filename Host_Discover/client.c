#include "netlib/net.h"
#include "netlib/defaults.h"
#include "utils/debug.h"
#include "utils/utils.h"
#include "client.h"

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <inttypes.h>

typedef struct Server{
   char *name;
   char *ip;
   int port;
}Server;

void *getAndShowServers(void *ptr);
char *createCommand(char *command);

Server servers[255];
int lastServer;
int endc;

int start_client(const char *ip_dst, const unsigned int tcpport, const unsigned int udpport, const char *name)
{
	int udpSocket, tcpSocket;
	int broadcastPermission;
	struct sockaddr_in broadcastAddr;
	char *buffer;
	int access;
	pthread_t thread;
	int status;
	char *cmd;

	endc = 1;

	udpSocket = newSocket(ip_dst,udpport,0,UDPSENDER);
	buildAddr(&broadcastAddr,ip_dst,udpport);
	broadcastPermission = 1;

	buffer = (char*) calloc (1,BUFFERSIZE);

	status = setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, sizeof(broadcastPermission));
    if(status == -1) 
    {
		debug(5,"Can't set Brodcast Option");
		return 1;
    }

    snprintf(buffer, BUFFERSIZE, "Hello from: %s\r\n\r\n", name);

    sendto(udpSocket,buffer,strlen(buffer),0,(struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr)); 

    debug(4,"Sent %s, port %i, ip %s",buffer, udpport, ip_dst);

    pthread_create(&thread, NULL, getAndShowServers, (void *)&udpSocket);
	
    while(true)
    {
    	scanf("%i",&access);

    	if(access < lastServer)
    	{
    		pthread_cancel(thread);
    		break;
    	}
    }

    closeSocket(udpSocket);

    tcpSocket = newSocket(servers[access].ip,servers[access].port,0,TCPSENDER);

    debug(1,"Conexion realizada con el servidor : %s", servers[access].name);

    cmd = (char*) calloc (1,BUFFERSIZE);

    while(endc)
    {
    	scanf("%s",cmd);

    	debug(4,"CMD: %s",cmd);
    	buffer = createCommand(cmd);

	    sendto(tcpSocket,buffer,strlen(buffer),0,NULL,0);
	    debug(4,"Sent: %s",buffer);
	    recvfrom(tcpSocket,buffer,BUFFERSIZE,0,NULL,0);
	    debug(4,"Received: %s",buffer);
    }
    
    closeSocket(tcpSocket);
	return true;
}

void *getAndShowServers(void *ptr)
{
	int clientSocket;
	struct sockaddr_in clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	int serverPort;
	char *buffer;
	char serverIp[18];
	char *listServers[255];
	int numFiles;

	char ** args;
	char * serverName;

	lastServer = 0;

	clientSocket = *((int *)ptr);
	buffer = (char*) calloc (1,BUFFERSIZE);

	debug(4,"Getting servers");

	while(recvfrom(clientSocket, buffer, BUFFERSIZE, 0, (struct sockaddr*)&clientAddr, &addrlen))
	{
		inet_ntop(AF_INET,&(clientAddr.sin_addr),serverIp,INET_ADDRSTRLEN);
		serverPort = ntohs(clientAddr.sin_port);

		args = (char**) calloc (255,BUFFERSIZE);

		debug(1,"Received: %s", buffer);
		getCommand(buffer,args);

		serverName = args[0];
		serverPort = strtoimax(args[2],NULL,10);
		numFiles = strtoimax(args[4],NULL,10);

		listServers[lastServer] = (char*) calloc (1,BUFFERSIZE);
		snprintf(listServers[lastServer], BUFFERSIZE, "%d Server %.*s\tIP: %s\tPort: %i\tFiles: %i\t", lastServer, 10, serverName, serverIp, serverPort, numFiles);
		servers[lastServer].name = serverName;
		servers[lastServer].ip = serverIp;
		servers[lastServer].port = serverPort;

		lastServer++;

		debug(1,"Write the number of server");

		for(int i=0; i<lastServer;i++)
			debug(1,"%s",listServers[i]);
	}

	return NULL;
}

char *createCommand(char *command)
{
	char *buffer;
	char *temp;
	char *cmd;

	buffer = (char*) calloc (1,BUFFERSIZE);
	temp = (char*) calloc (1,BUFFERSIZE);
	cmd = (char*) calloc (1,BUFFERSIZE);

	memset(buffer,0,BUFFERSIZE);
	memset(temp,0,BUFFERSIZE);
	memset(cmd,0,BUFFERSIZE);

	if(strcmp(command,"PING")==0)
	{
		debug(3,"CMD: PING");
		snprintf(cmd, BUFFERSIZE, "PING \r\n\r\n");
		snprintf(buffer, BUFFERSIZE, "%s",cmd);
	}
	else if (strcmp(command,"FILELIST")==0)
	{
		debug(3,"CMD: FILELIST");
		snprintf(cmd, BUFFERSIZE, "FILELIST \r\n\r\n");
		snprintf(buffer, BUFFERSIZE, "%s",cmd);
	}
	else if (strcmp(command,"BYE")==0)
	{
		debug(3,"CMD: BYE");
		snprintf(cmd, BUFFERSIZE, "BYE \r\n\r\n");
		snprintf(buffer, BUFFERSIZE, "%s",cmd);
		endc = 0;
	}
	else
	{
		debug(3,"Unknown command");
		snprintf(buffer, BUFFERSIZE, "FAIL \r\n\r\n");
	}

	return buffer;
}