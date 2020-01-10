#ifndef SERVER_H
#define SERVER_H
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <pthread.h>
#include <dirent.h>
#define BUF_LEN 1024
#define FD_SIZE 100
#define MAX_BACK 100
#define QUERY_SIZE 129
#define MAX_PATH 150
char ROOT[100];
int PORT;
int THREAD_NUMBER;
typedef struct q
{
    char element[QUERY_SIZE];
    int fd;
    struct q *next;
} queue;
typedef struct para
{
    pthread_t thread;
    pthread_attr_t attr;
    int number;
    char* root;
    char* q_str;
} thread_pool;
typedef struct re
{
    char path[MAX_PATH];
    int cnt;
    struct re *next;
} result;
typedef struct ans
{
    char q_str[QUERY_SIZE];
    result *result_head;
    result *result_tail;
} answer;
queue *head=NULL;
queue *tail=NULL;
int *thread_state=NULL; // 0 means idle, 1 means busy
pthread_mutex_t mutex;
int read_from_client (int);
void *seeking(void*);
void dir_recursive(char*,char*,int,answer*);
void find_string(char*,char*,int,answer*);
int getsize(char*);
void print_ans(answer*,int);
#endif