#include "netlib/tcp.h"
#include "utils/debug.h"
#include "defaults.h"
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

unsigned long currentTimeMillis() ;

int start_client(const char *ip_dst, const unsigned int port, const char *filename) 
{

	int clientSocket;
	unsigned long start, end;
	int file;
	char *readBuffer;
	char *buffer;
	
	int localerror;
	
	int readBytes,totalReadBytes;
	int writeBytes,totalWriteBytes;

	char *command = NULL;
	char *data = NULL;
	char tmp[1];
	char r;
	char n;
	int mode;
	int filesize;

	clientSocket = newTCPClientSocket4(ip_dst,port);

	buffer = (char *) calloc(1,BUFFERSIZE);
	bzero(buffer,sizeof(buffer));
	strcpy(buffer,"GET:");
	strcat(buffer,filename);
	strcat(buffer,"\r\n");

	totalWriteBytes = 0;
	while(totalWriteBytes < strlen(buffer)) 
	{
		writeBytes = write(clientSocket,buffer+totalWriteBytes,strlen(buffer)-totalWriteBytes);
		totalWriteBytes += writeBytes;
	}
	
	//Read OK or NOTFOUND
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

	if(strcmp(command,"OK")!=0)
	{
		debug(5,"\tComando incorrecto");
		free(buffer);	
		closeTCPSocket(clientSocket);
		return NULL;
	}

	//Read Size
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

	if(strcmp(command,"SIZE")!=0)
	{
		debug(5,"\tComando incorrecto");
		free(buffer);	
		closeTCPSocket(clientSocket);
		return NULL;
	}

	filesize = atoi(data);

	//SEND OK
	bzero(buffer,sizeof(buffer));
	strcpy(buffer,"OK\r\n");

	totalWriteBytes = 0;
	while(totalWriteBytes < strlen(buffer)) 
	{
		writeBytes = write(clientSocket,buffer+totalWriteBytes,strlen(buffer)-totalWriteBytes);
		totalWriteBytes += writeBytes;
	}
	
	//OPEN FILE
	if((file = open(filename,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1) 
	{
		localerror = errno;
		fprintf(stderr,"Can't open file for write (%s)\n",strerror(localerror));
		return false;
	}	
	

	//READ FILE AND SAVE
	readBuffer = (char *) calloc(1,BUFFERSIZE);
	start = currentTimeMillis();
	totalReadBytes = 0;
	
	while((readBytes = read(clientSocket,readBuffer,BUFFERSIZE)) > 0) 
	{
		debug(5,"\tSe leyeron de %s:%u %u bytes",ip_dst,port,writeBytes);
		totalWriteBytes = 0;
		while(totalWriteBytes < readBytes) 
		{
			writeBytes = write(file,readBuffer+totalWriteBytes,readBytes-totalWriteBytes);
			totalWriteBytes += writeBytes;
		}
		end = currentTimeMillis();

		printf("\rRecibido a %s:%u\t%lu\tKBps",ip_dst,port,(readBytes/(end-start)*1000/1024));

		totalReadBytes += readBytes;
		start = currentTimeMillis();
	}

	close(file);
	free(readBuffer);

	//SEND BYE
	bzero(buffer,sizeof(buffer));
	strcpy(buffer,"BYE\r\n");

	totalWriteBytes = 0;
	while(totalWriteBytes < strlen(buffer)) 
	{
		writeBytes = write(clientSocket,buffer+totalWriteBytes,strlen(buffer)-totalWriteBytes);
		totalWriteBytes += writeBytes;
	}

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

	free(buffer);
	closeTCPSocket(clientSocket);
	
	return true;
}

unsigned long currentTimeMillis() 
{
        unsigned long t;
        struct timeval tv;
        gettimeofday(&tv, 0);
        t = (tv.tv_sec*1000)+tv.tv_usec;
        return t;
}