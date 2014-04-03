

#ifndef SERVER_H
#define SERVER_H

int start_server(const char *iface, const unsigned short tcpport, const unsigned short udpport, const unsigned short maxClients, const char *name);

#endif