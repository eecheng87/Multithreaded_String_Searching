#include "server.h"

int main(int argc, char *argv[])
{
    char *delim = "\"";
    char *pch;
    queue *tmp_q;
    // init argv

    if(strlen(argv[2])>80)
    {
        printf("\nROOT is larger than 80, app exit\n");
        printf("Please restart server\n");
        return 0;
    }
    strcpy(ROOT,argv[2]);
    PORT = atoi(argv[4]);
    THREAD_NUMBER = atoi(argv[6]);
    int             listenfd, connfd, sockfd, maxfd, maxi, i;
    int             nready, client[FD_SIZE];
    //int             lens;
    ssize_t     n;
    fd_set        rset, allset;
    char         buf[BUF_LEN];
    socklen_t    clilen;
    struct sockaddr_in servaddr, chiaddr;

    // initial muxtex lock
    pthread_mutex_init(&mutex,0);

    // initial thread pool
    // initial thread state
    int tn;
    thread_state = (int*)malloc(THREAD_NUMBER*sizeof(int));
    for(tn = 0; tn < THREAD_NUMBER; ++tn)
        *(thread_state+tn) = 0;
    thread_pool tp[THREAD_NUMBER];
    for(tn = 0; tn < THREAD_NUMBER; ++tn)
    {
        tp[tn].number = tn;
        pthread_attr_init(&tp[tn].attr);
        pthread_create(&(tp[tn].thread),&(tp[tn].attr),seeking,(void*)&tp[tn]);
    }

    if( ( listenfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
    {
        printf( "Create socket Error : %d\n", errno );
        exit( EXIT_FAILURE );
    }
    // fill server information
    bzero( &servaddr, sizeof( servaddr ) );
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr  =htonl( INADDR_ANY );
    servaddr.sin_port = htons( PORT );

    if( bind( listenfd, ( struct sockaddr * )&servaddr, sizeof( servaddr ) ) == -1 )
        exit(EXIT_FAILURE  );
    if( listen( listenfd, MAX_BACK ) == -1 )
        exit( EXIT_FAILURE );



    maxfd = listenfd;    // update current maxfd
    maxi = -1;

    // initial array client
    // -1 means there isn't client.c occupy
    for( i = 0; i < FD_SIZE; i++ )
    {
        client[i] = -1;
    }

    FD_ZERO( &allset );        // initial set to empty
    FD_SET( listenfd, &allset ); // add server socket into set


    while( 1 )
    {
        rset = allset;// initial rset to original set(allset)

        // careful: rset will be modify by `select`
        if( (nready = select( maxfd + 1, &rset, NULL, NULL, NULL )) == -1)
        {
            printf("select fail\n");
            exit( EXIT_FAILURE );
        }

        if( nready <= 0 )            // return value of select is negative
        {
            // no client
            continue;
        }



        if( FD_ISSET( listenfd, &rset ) )
        {
            clilen = sizeof( chiaddr );

            if( ( connfd  = accept( listenfd, (struct sockaddr *)&chiaddr, &clilen ) ) == -1 )
            {
                // wait for some client try to make connect
                printf( "accept error \n");
                continue;
            }


            for( i = 0; i < FD_SIZE; i++ )
            {
                // find empty client to let new client occupy
                if( client[i] < 0 )
                {
                    client[i] = connfd;
                    break;
                }
            }

            // don't care last one
            if( i == FD_SIZE )
            {
                close( connfd );
                continue;
            }

            // add new client into set
            FD_SET( connfd, &allset );
            // update max
            if( connfd > maxfd )
            {
                maxfd = connfd;
            }

            if( i > maxi )
            {
                maxi = i;
            }
        }

        for( i = 0; i <= maxi; i++ )
        {
            if( ( sockfd = client[i] ) > 0 )
            {
                // try to read
                if( FD_ISSET( sockfd, &rset ))
                {
                    memset( buf, 0, sizeof( buf ) );

                    n = read( sockfd, buf, BUF_LEN);
                    // print client query string
                    printf("%s\n",buf);
                    if( n < 0 )
                    {
                        printf("error!\n");
                        close( sockfd );
                        FD_CLR( sockfd, &allset );
                        client[i] = -1;
                        continue;
                    }
                    if( n == 0 )
                    {
                        // if client is close by ctrl-c
                        printf("some client leave\n");
                        close( sockfd );
                        FD_CLR( sockfd, &allset );
                        client[i] = -1;
                        continue;
                    }

                    // if input is "exit", server close it
                    if( strcmp( buf, "exit" ) == 0 )
                    {
                        close( sockfd );
                        FD_CLR( sockfd, &allset );
                        client[i] = -1;
                        continue;
                    }
                    // processing input string fron clients
                    pch = strtok(buf,delim);
                    while(pch!=NULL)
                    {
                        // insert into queue
                        pch = strtok(NULL,delim);
                        if(!pch)break;
                        if(head==NULL)
                        {
                            tail = head = (queue*)malloc(sizeof(queue));
                            strcpy(head->element,pch);
                            head->fd = sockfd;
                            head->next = NULL;
                        }
                        else
                        {
                            tmp_q = (queue*)malloc(sizeof(queue));
                            tmp_q->fd = sockfd;
                            // maybe this line is super important
                            tmp_q->next = NULL;
                            // maybe this line is super important
                            strcpy(tmp_q->element,pch);
                            tail->next = tmp_q;
                            tail = tmp_q;
                        }
                        pch = strtok(NULL,delim);
                    }

                    //write( sockfd, buf, n );
                }
            }
        }

    }

    return 0;
}

void *seeking(void* para)
{
    char query_str[QUERY_SIZE];
    thread_pool *tmp_pool = (thread_pool*)para;
    int offset = tmp_pool->number;
    int send_fd = 0;
    while(1)
    {

        if(*(thread_state+offset)==1)
        {
            //busy
            //while(1);
            answer *ans = (answer*)malloc(sizeof(answer));
            strcpy(ans->q_str,query_str);
            ans->result_head = NULL;
            ans->result_tail = NULL;
            dir_recursive(ROOT,query_str,send_fd,ans);
            print_ans(ans,send_fd);
            *(thread_state+offset) = 0;
            free(ans);
        }
        else
        {
            //idle
            pthread_mutex_lock(&mutex);
            if(head!=NULL)
            {
                // queue is not empty
                queue *tmpq = head;
                strcpy(query_str,tmpq->element);
                send_fd = tmpq->fd;
                head = head->next;
                free(tmpq);
                //printf("%s",query_str);
                *(thread_state+offset) = 1;
            }
            pthread_mutex_unlock(&mutex);
        }

    }
}
void print_ans(answer* ans,int fd)
{
    char buff[140]; // query string size limit
    memset(buff,0,sizeof(buff));
    strcat(buff,"String: \"");
    strcat(buff,ans->q_str);
    strcat(buff,"\"");
    //answer *tmp = ans;
    result *tmp_r = ans->result_head;
    int er = write(fd,buff,sizeof(buff));
    if(er<0)
        printf("write faill\n");
    memset(buff,0,sizeof(buff));
    // check not found condition

    if(tmp_r==NULL)
    {
        strcpy(buff,"Not found");
        er = write(fd,buff,sizeof(buff));
        if(er<0)
            printf("write faill\n");
    }
    else
    {
        while(tmp_r)
        {
            result* trash = tmp_r;
            char num[20];
            memset(buff,0,sizeof(buff));
            strcpy(buff,"File: ");
            strcat(buff,tmp_r->path);
            strcat(buff,", Count: ");
            sprintf(num,"%d",tmp_r->cnt);
            strcat(buff,num);
            er = write(fd,buff,sizeof(buff));
            if(er<0)
                printf("write faill\n");
            tmp_r = tmp_r->next;
            free(trash);
        }
    }
}
int getsize(char* p)
{
    char *t;
    for(t=p; *t!='\0'; ++t);
    return t-p;
}
void find_string(char *file_path,char *q,int fd,answer* ans)
{
    char *line=NULL;
    size_t len=0;
    ssize_t read;
    //int q_index = 0;
    //int global_index = 0;
    int cnt = 0;
    int cur_line_len;
    int q_len = getsize(q);
    int i,j;
    /*if(strcmp(file_path,"testdir/testdir2/test2.txt")==0){
        //printf("%d %d\n",sizeof(file_path),sizeof(tmp_r->path));
        printf("hihi");
    }*/
    FILE *ff = fopen(file_path,"r");


    while((read = getline(&line,&len,ff)) != -1)
    {
        cur_line_len = getsize(line);
        for(i=0; i<=cur_line_len-q_len; ++i)
        {
            for(j=0; j<q_len; ++j)
            {
                if((j==q_len-1)&&(line[i+j]==q[j]))
                {
                    cnt++;
                }
                if(line[i+j]!=q[j])
                {
                    j=100;
                }
            }
        }
    }
    fclose(ff);

    if(cnt==0)
    {
        //ans->result_tail->next = NULL;
        return;
    }

    result *tmp_r = (result*)malloc(sizeof(result));
    //memset(tmp_r->path,0,MAX_PATH);


    strcpy(tmp_r->path,file_path);
    tmp_r->cnt = cnt;
    tmp_r->next = NULL; // this is important, may cuz core dump!

    if(ans->result_head==NULL)
    {
        ans->result_head = ans->result_tail = tmp_r;
        // ans->result_head->next = ans->result_tail->next = NULL;
    }
    else
    {
        ans->result_tail->next = tmp_r;
        ans->result_tail = ans->result_tail->next;
        // ans->result_tail->next = NULL; // this is important, may cuz core dump!
    }

    //printf("%d\n",cnt);
    // write( sockfd, buf, n );
}
void dir_recursive(char *path,char *qs,int fd,answer *ans)
{

    char glue='/'; // linux delimeter

    // try to open directory
    DIR * dp = opendir(path);

    if (!dp)
    {
        //printf("%s\n",path);
        find_string(path,qs,fd,ans);
        //printf("%s\n",path);
        return ;
    }

    struct dirent *filename;
    while((filename=readdir(dp)))
    {
        // skip current path or parent path
        if(!strcmp(filename->d_name,"..") || !strcmp(filename->d_name,"."))
        {
            continue;
        }

        // calculate the size of the path
        int pathLength=strlen(path)+strlen(filename->d_name)+2;
        char *pathStr = (char*)malloc(sizeof(char) * pathLength);

        strcpy(pathStr, path);

        int j = strlen(pathStr);
        if(pathStr[j-1]!=glue)
        {
            pathStr[j]=glue;
            pathStr[j+1]='\0';
        }

        // concat sub-directory of file name
        strcat(pathStr, filename->d_name);
        dir_recursive(pathStr,qs,fd,ans);
    }
    closedir(dp);

    return ;
}