#ifndef CLIENT_H
#define CLIENT_H
#define _POSIX_C_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
#include <pthread.h>
#define MAXLINE 140
char LOCALHOST[100];
int PORT;
void send_and_recv(int);
void* request(void*);
#endif