
#include "debug.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

char *getCommand(char *buffer, char **args) 
{
    char *pch;
    char *command;
    int cont=0;

    command = (char *)calloc (255,1);
    strcat(buffer,"\0");

    pch = strtok (buffer," :\r\n"); 

    while (pch != NULL)
	{
		if(strlen(command) == 0)
		{
			strcpy(command, pch);
		}
		else if(args[cont] == NULL || strlen(args[cont]) == 0)
		{	
			if(args[cont] == NULL)
				args[cont] = (char *)calloc (255,1);
			strcpy(args[cont], pch);
			cont++;
		}	
	
		pch = strtok(NULL, " :\r\n");
	}

    return command; 
}

off_t fsize(const char *filename) 
{
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}

int getFile(char *filepath)
{
    int file;
    //resp = (char *) calloc(1,255);
    //printf("File Size: %d", getFileSize(filename));       //Print Filesize
                                //MD5 (pending)
    //OPEN FILE
    if((file = open(filepath, O_RDONLY)) == -1) 
    {
        close(file);
        return -1;
    }
    else
    {
        close(file);
        return 0;
    }
 
    //Process to transfer file  (pending)
}

int fileList(char *path, char *list)
{
    DIR *dir;
    int f_count = 0;
    struct dirent *ent;

    list = (char*) calloc (1,10000);
    memset(list,0,10000);
  
    if ((dir = opendir (path)) != NULL) {     
        while ((ent = readdir (dir)) != NULL) 
        {   
            strcat(list,ent->d_name);
            strcat(list," \r\n");
            f_count++;
        }
        closedir (dir);
    } else {
        return 0;
    }
 
    return f_count;
}