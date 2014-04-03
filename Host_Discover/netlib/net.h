/**
  @file tcp.h
  @brief Funciones generales para trabajar con un Socket de tipo TCP

  @author Alvaro Parres
  @date Feb/2013

*/

#ifndef NET_H
#define NET_H

int newSocket(const char *ip, const unsigned short port, const int q_size, const int type);
int buildAddr(struct sockaddr_in *addr, const char *ip, const unsigned short port);
void closeSocket(const int socketFD);
int getNewSocket(const int domain, int type);
int waitConnection(int socket, char *clientIP, unsigned int *clientPort);
int readfromSocket(const int socket, char *buffer, const unsigned int maxRead);
int sendtoSocket(const int socket, char *buffer, const unsigned int size);

#endif
