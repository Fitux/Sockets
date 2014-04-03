
#ifndef UTILS_H
#define UTILS_H

char * getCommand(char *buffer, char **args);
off_t fsize(const char *filename);
int getFile(char *filepath);
int fileList(char *path, char *list);

#endif