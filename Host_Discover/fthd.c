#include "netlib/defaults.h"
#include "utils/debug.h"
#include "fthd.h"

#include "server.h"
#include "client.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <inttypes.h>
#include <limits.h>

//Variables Globales
unsigned short udpport = CONFIG_DEFAULT_UDPPORT;
unsigned short tcpport = CONFIG_DEFAULT_TCPPORT;
int debugLevel = CONFIG_DEFAULT_VERBOSE;
char ip_dst[18] = CONFIG_BDCAST_IFACE;
char ip_listen[18] = CONFIG_LISENT_IFACE;
char name[255];

enum mode_enum {
	NOT_DEFINE = 0,
	SERVER = 1,
	CLIENT = 2
};

mode_enum mode = NOT_DEFINE;

//Prototipos Locales
int processArguments(int argc, char* argv[]);
void printHelp(const char *cmd, const short error, const char *msg=NULL);

//Fin de Prototipos Locales

int startfthd(int argc, char *argv[]) {
	
	printf(".:: TCP PING ::.\n");
	
	if(!processArguments(argc, argv)) return -1;
	
	if(mode == SERVER) {
		start_server(ip_listen, tcpport, udpport, CONFIG_MAX_CLIENT,name);
	} else if(mode == CLIENT) {
		start_client(ip_dst, tcpport, udpport, name);
	}
	
	return 0;
}

int processArguments(int argc, char* argv[]) {

	int i;
	unsigned int temp;
	
	if(argc < 3) {
		printHelp(argv[0],true,"Faltan Argumentos\n");
		return false;
	}	

    stpncpy(name, argv[1], 255);

    if(strcmp(argv[2],"-s")==0){
		mode = SERVER;
	} else if(strcmp(argv[2],"-c")==0) {
		mode = CLIENT;
	} else {
		printHelp(argv[0],true,"Unkown Mode\n");
		return false;
	}
	
	i=3;

	for(; i<(argc); i++) {
		if(strcmp(argv[i],"-h")==0) {
			printHelp(argv[0],false);
			return true;
		}	
		
		if(strcmp(argv[i],"-pt")==0) {
			temp = strtoimax(argv[++i], NULL, 10);
			if(temp == 0 || temp < MINPORT || temp > MAXPORT) {
				printHelp(argv[0],true,"Port out of range\n");
				return false;
			}
			tcpport = temp;
		} else if(strcmp(argv[i],"-pu")==0) {
			temp = strtoimax(argv[++i], NULL, 10);
			if(temp == 0 || temp < MINPORT || temp > MAXPORT) {
				printHelp(argv[0],true,"Port out of range\n");
				return false;
			}
			udpport = temp;
		} else if(strcmp(argv[i],"-d")==0) {
			stpncpy(ip_dst,argv[++i],18);
		} else if(strstr(argv[i],"-v")!=NULL) {
			debugLevel = strlen(argv[i])-1;
		} else if(strcmp(argv[i],"-i")==0) {
			stpncpy(ip_listen,argv[++i],18);	
		} else {
			printHelp(argv[0],true,"Unkown option\n");
			return false;
		}	
	}

	debug(2,"%s","\tConfigurations:");
	if(mode == SERVER) {
		debug(2,"\t\tMode:\t%s","Server");
		debug(2,"\t\tListen on:\t%s:%u",ip_listen,tcpport);
	} else if (mode == CLIENT) {	
		debug(2,"\t\tMode:\t%s","Client");
		debug(2,"\t\tDestiny:\t%s:%u",ip_dst,tcpport);
	}

	debug(2,"\t\tName:\t%s",name);
	
	return true;
}

void printHelp(const char *cmd, const short error, const char *msg) {
	
	if(error) {
		fprintf(stderr,"Error: %s\n\n",msg);
	}
	
	printf("Use:\t%s NAME -s  [options] ( SERVER )\n", cmd);
	printf("Use:\t%s NAME -c  [options] ( CLIENT ) \n", cmd);
	printf("\n");
	printf("Global Options:\n");
	printf("\t-pt\t Set port number to work on TCP(Default %u)\n",CONFIG_DEFAULT_TCPPORT);
	printf("\t-pu\t Set port number to work on UDP(Default %u)\n",CONFIG_DEFAULT_UDPPORT);
	printf("\t-v[vvvv]\t Increments the verbosity of the server (Default %u)\n",CONFIG_DEFAULT_VERBOSE);
	printf("\t-h\t Prints this help message\n");
	printf("\t-i IP_ADDRe\t Interface to listen on. (Default %s)\n",CONFIG_LISENT_IFACE);
	printf("\t-d IP_ADDR\t Set the IP Dst. (Default %s)\n", CONFIG_DEST_IFACE);	
	printf("\n");
} 