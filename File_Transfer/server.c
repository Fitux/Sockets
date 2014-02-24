#include "netlib/tcp.h"
#include "utils/debug.h"
#include "defaults.h"
#include "server.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>

void *attendClient(void *ptr);

int start_server(const char iface[], const unsigned short port, const unsigned short maxClients) 
{

	int serverSocket;
	int clientSocket;
	pthread_t thread;
	
	char clientIP[18];
	unsigned int clientPort;
	
	/*
	int file;
	
	int localerror;
	
	int readBytes = 0;
	int totalReadBytes = 0;	
	int totalWriteBytes = 0;
	int writeBytes = 0;
	char *buffer = NULL;
	*/
	
	serverSocket = newTCPServerSocket4(iface,port,maxClients);
	
	if(serverSocket == -1) 
	{
		return false;
	}
	
	while(1)
	{
		bzero(clientIP,sizeof(clientIP));
		clientPort = 0;
			
		debug(5,"%s","Waiting for a Client...");
		clientSocket = waitConnection4(serverSocket,clientIP,&clientPort);
		debug(4,"Connected Client %s:%u",clientIP,clientPort);

		pthread_create(&thread, NULL, attendClient, (void *)&clientSocket);

		/*
		if((file = open(filename,O_RDONLY)) == -1) 
		{
			localerror = errno;
			fprintf(stderr,"Can't open filename (%s)",strerror(localerror));
			return false;
		}

		buffer = (char *) calloc(1,BUFFERSIZE);

		while((readBytes = read(file,buffer,BUFFERSIZE))>0) 
		{

			debug(5,"\tSe leyeron de archivo %u bytes",readBytes);		
			totalWriteBytes = 0;
			while(totalWriteBytes < readBytes) 
			{
				writeBytes = write(clientSocket,buffer+totalWriteBytes,readBytes-totalWriteBytes);
				debug(5,"\tSe escribieron a %s:%u %u bytes",clientIP,clientPort,readBytes);
				totalWriteBytes += writeBytes;
			}
			totalReadBytes += readBytes;
		}	

		debug(3,"\t Se leyeron un total de %i de bytes\n",totalReadBytes);
		
		free(buffer);	
		close(file);
		closeTCPSocket(clientSocket);
		debug(4,"Close connection (%s:%u)",clientIP,clientPort);
		*/
	}


	closeTCPSocket(serverSocket);	
	return true;
	
}

void *attendClient(void *ptr)
{
    int clientSocket;
    char *filename;
    int file;
    int localerror;

    int readBytes = 0;
	int totalReadBytes = 0;	
	int totalWriteBytes = 0;
	int writeBytes = 0;
	char *command = NULL;
	char *data = NULL;
	char *buffer = NULL;
	char tmp[1];
	char r;
	char n;
	int mode;
	int fileSize;

	struct stat buf;

	clientSocket = *((int *)ptr);
	filename = (char *) calloc(1,BUFFERSIZE);
	buffer = (char *) calloc(1,BUFFERSIZE);

	//READ GET
	data = (char *) calloc(1,BUFFERSIZE);
	command = (char *) calloc(1,BUFFERSIZE);
	strcpy(data,"");
	strcpy(command,"");
	r = 0;
	n = 0;
	mode = 0;
	while(r != '\r' && n != '\n' && (readBytes = read(clientSocket, tmp, 1)) > 0)
	{
		r = n;
		n = tmp[0];

		if(n == ':')
			mode=1;

		if(n != '\r' && n != '\n' && n != ':')
		{
			if(mode==0)
			{
				strcat(command,tmp);
			}
			else
			{
				strcat(data,tmp);
			}
		}

	}

	debug(5,"\tComando: %s",command);

	if(strcmp(command,"GET")!=0)
	{
		debug(5,"\tComando incorrecto");
		free(buffer);	
		closeTCPSocket(clientSocket);
		return NULL;
	}

	strcpy(filename,data);
		

	//OPEN FILE
	if((file = open(filename,O_RDONLY)) == -1) 
	{
		//SEND NOT FOUND
		debug(5,"\tNo se abrio el archivo %s",filename);
		localerror = errno;
		fprintf(stderr,"Can't open filename (%s)",strerror(localerror));

		bzero(buffer,sizeof(buffer));
		strcpy(buffer,"NOT FOUND\r\n");

		totalWriteBytes = 0;
		while(totalWriteBytes < strlen(buffer)) 
		{
			writeBytes = write(clientSocket,buffer+totalWriteBytes,strlen(buffer)-totalWriteBytes);
			totalWriteBytes += writeBytes;
		}

		free(buffer);	
		closeTCPSocket(clientSocket);
		return NULL;
	}
	else
	{
		//SEND OK
		debug(5,"\tSe abrio el archivo %s",filename);

		bzero(buffer,sizeof(buffer));
		strcpy(buffer,"OK\r\n");

		totalWriteBytes = 0;
		while(totalWriteBytes < strlen(buffer)) 
		{
			writeBytes = write(clientSocket,buffer+totalWriteBytes,strlen(buffer)-totalWriteBytes);
			totalWriteBytes += writeBytes;
		}
	}

	//SEND SIZE
	fstat(file, &buf);
	fileSize = buf.st_size;
	debug(5,"\tEl archivo pesa %u ",fileSize);

	bzero(buffer,sizeof(buffer));
	snprintf(buffer, BUFFERSIZE, "SIZE:%i\r\n", fileSize);

	totalWriteBytes = 0;
	while(totalWriteBytes < strlen(buffer)) 
	{
		writeBytes = write(clientSocket,buffer+totalWriteBytes,strlen(buffer)-totalWriteBytes);
		totalWriteBytes += writeBytes;
	}

	//READ OK
	data = (char *) calloc(1,BUFFERSIZE);
	command = (char *) calloc(1,BUFFERSIZE);
	strcpy(data,"");
	strcpy(command,"");
	r = 0;
	n = 0;
	mode = 0;
	while(r != '\r' && n != '\n' && (readBytes = read(clientSocket, tmp, 1)) > 0)
	{
		debug(5,"\tChar %c",tmp[0]);
		r = n;
		n = tmp[0];

		if(n == ':')
			mode=1;

		if(n != '\r' && n != '\n' && n != ':')
		{
			if(mode==0)
			{
				strcat(command,tmp);
				debug(5,"\tComando %s",command);
			}
			else
			{
				strcat(data,tmp);
				debug(5,"\tData %s",data);
			}
		}

	}

	debug(5,"\tComando: %s",command);

	if(strcmp(command,"OK")!=0)
	{
		debug(5,"\tComando incorrecto");
		free(buffer);	
		closeTCPSocket(clientSocket);
		return NULL;
	}

	//SEND FILE
	bzero(buffer,sizeof(buffer));
	while((readBytes = read(file,buffer,BUFFERSIZE))>0) 
	{

		debug(5,"\tSe leyeron de archivo %u bytes",readBytes);		
		totalWriteBytes = 0;
		while(totalWriteBytes < readBytes) 
		{
			writeBytes = write(clientSocket,buffer+totalWriteBytes,readBytes-totalWriteBytes);
			debug(5,"\tSe escribieron %u bytes",readBytes);
			totalWriteBytes += writeBytes;
		}
		totalReadBytes += readBytes;
	}	
	
	debug(3,"\t Se leyeron un total de %i de bytes\n",totalReadBytes);
	
	//READ BYE
	bzero(data,sizeof(data));
	bzero(command,sizeof(command));
	strcpy(data,"");
	strcpy(command,"");
	r = 0;
	n = 0;
	mode = 0;
	while(r != '\r' && n != '\n' && (readBytes = read(clientSocket, tmp, 1)) > 0)
	{
		r = n;
		n = tmp[0];

		if(n == ':')
			mode=1;

		if(n != '\r' && n != '\n' && n != ':')
		{
			if(mode==0)
				strcat(command,tmp);
			else
				strcat(data,tmp);
		}

	}

	debug(5,"\tComando: %s",command);

	if(strcmp(command,"BYE")!=0)
	{
		debug(5,"\tComando incorrecto");
		free(buffer);	
		closeTCPSocket(clientSocket);
		return NULL;
	}

	//SEND BYE
	bzero(buffer,sizeof(buffer));
	strcpy(buffer,"BYE\r\n");

	totalWriteBytes = 0;
	while(totalWriteBytes < strlen(buffer)) 
	{
		writeBytes = write(clientSocket,buffer+totalWriteBytes,strlen(buffer)-totalWriteBytes);
		totalWriteBytes += writeBytes;
	}

	free(buffer);	
	close(file);
	closeTCPSocket(clientSocket);
	return NULL;
}


