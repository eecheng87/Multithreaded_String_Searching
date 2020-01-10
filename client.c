#include "client.h"

int main(int argc, char *argv[])
{
    pthread_t tid;
    pthread_attr_t attr;
    // init argv
    strcpy(LOCALHOST,argv[2]);
    PORT = atoi(argv[4]);

    int       connfd;
    struct sockaddr_in servaddr;
    if( ( connfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
        exit( EXIT_FAILURE );



    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, LOCALHOST, &servaddr.sin_addr);

    if( connect( connfd, ( struct sockaddr *  )&servaddr, sizeof( servaddr ) ) < 0 )
        exit(EXIT_FAILURE);



    pthread_attr_init(&attr);
    pthread_create(&tid,&attr,request,(void*)&connfd);
    send_and_recv( connfd );

    close( connfd );
    printf("Exit\n");

    return 0;
}
void* request(void* cnfd)
{
    int connfd = *((int*)cnfd);
    FILE * fp = stdin;
    int   lens;
    char send[MAXLINE];
    fd_set rset;
    FD_ZERO( &rset );
    int maxfd = ( fileno( fp ) > connfd ? fileno( fp ) : connfd  + 1 );

    while( 1 )
    {
        FD_SET( fileno( fp ), &rset );
        FD_SET( connfd, &rset );

        if( select( maxfd, &rset, NULL, NULL, NULL ) == -1 )
        {
            printf("client select error..\n");
            exit(EXIT_FAILURE  );
        }
        // if client try to type
        if( FD_ISSET( fileno( fp ), &rset ) )
        {
            memset( send, 0, sizeof( send ) );
            if( fgets( send, MAXLINE, fp ) == NULL )
            {
                printf("End...\n");
                exit( EXIT_FAILURE );
            }
            else
            {
                lens = strlen( send );
                send[lens-1] = '\0';  // discard '\n'
                if(lens > 137)
                {
                    /*
                        processing input more than 136
                    */
                    printf("Request length is larger than 136\n");
                    printf("Reqeust is reject\n");
                    memset( send, 0, sizeof( send ) );
                    char *cp;
                    if((cp=strchr(send,'\n')))
                    {
                        *cp = 0;
                    }
                    else
                    {
                        int er1 = scanf("%*[^\n]");
                        if(er1<0)
                            printf("scanf fail\n");
                        int er2 = scanf("%*c");
                        if(er2<0)
                            printf("scanf fail\n");
                    }
                }
                else if( strcmp( send, "exit" ) == 0 )
                {
                    printf( "exit connection..\n" );
                    return NULL;
                }
                else
                {
                    // check quote number weather valid
                    int index;
                    int qnum = 0;
                    for(index = 0; (send[index]!='\0')&&(index<137); ++index)
                    {
                        if(send[index]=='\"')
                            qnum++;
                    }
                    if(qnum==0||qnum%2==1)
                    {
                        printf("The strings format is not correct\n");
                    }
                    else
                    {
                        int erw = write( connfd, send, strlen( send ) );
                        if(erw<0)
                            printf("write fail\n");
                    }

                }

            }
        }
    }

}
void send_and_recv( int connfd )
{
    FILE * fp = stdin;
    int   lens;

    char recv[MAXLINE];
    fd_set rset;
    FD_ZERO( &rset );
    int maxfd = ( fileno( fp ) > connfd ? fileno( fp ) : connfd  + 1 );

    int n;

    while( 1 )
    {
        FD_SET( fileno( fp ), &rset );
        FD_SET( connfd, &rset );

        if( select( maxfd, &rset, NULL, NULL, NULL ) == -1 )
        {
            printf("client select error..\n");
            exit(EXIT_FAILURE  );
        }
        if( FD_ISSET( connfd, &rset ) )
        {
            memset( recv, 0, sizeof( recv ) );
            n = read( connfd, recv, MAXLINE );
            if( n == 0 )
            {
                printf("Recv ok...\n");
                break;
            }
            else if( n == -1 )
            {
                printf("Recv error...\n");
                break;
            }
            else
            {
                lens = strlen( recv );
                recv[lens] = '\0';
                int erw = write( STDOUT_FILENO, recv, MAXLINE );
                if(erw<0)
                    printf("write fail\n");
                printf("\n");
            }

        }
    }

}