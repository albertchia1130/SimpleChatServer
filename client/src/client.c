
#include <arpa/inet.h>
#include <netinet/in.h> //structure for storing address information 
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/socket.h> //for socket APIs 
#include <sys/types.h> 
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <syslog.h>


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 8080

int ConnectSocket;

static void*  ReadingThread(void *arg);
static void*  WritingThread(void *arg);
void PromtForServer(char * ipaddr);
int CreateAccount();

int main() 
{
    
    pthread_t writing_thread_id,reading_thread_id;
    int iResult;
    struct sockaddr_in servAddr;
    char ipaddress[DEFAULT_BUFLEN];

    ConnectSocket = socket(AF_INET, SOCK_STREAM, 0);

    do{

        PromtForServer(ipaddress);
        servAddr.sin_family = AF_INET;
        servAddr.sin_port= htons(DEFAULT_PORT); // use some unused port number
        servAddr.sin_addr.s_addr = inet_addr(ipaddress);

        // Connect to server.
        iResult = connect( ConnectSocket,(struct sockaddr*)&servAddr,
                    sizeof(servAddr));
        if (iResult == -1) {
            printf("Connection failed, Please try again\n");
        }

    }while(iResult == -1);


    

    printf("Connection Succeded\n");

    if(CreateAccount() == 0){

        pthread_create(&reading_thread_id, NULL, &ReadingThread,NULL);
        pthread_create(&writing_thread_id, NULL, &WritingThread,NULL);
    }



    pthread_join(reading_thread_id, NULL);

    return 0;
}

static void*  ReadingThread(void *arg) 
{ 
    int readStatus = 1;
    char serMsg[DEFAULT_BUFLEN];
    printf("Thread Created for read\n");
    while(1)
    {
       memset(serMsg, 0, (255*sizeof(char)));
       readStatus = recv(ConnectSocket, serMsg, sizeof(serMsg), 0);
       if (readStatus <= 0) {
            printf("read failed with error"); 
            return 0;
        }
       printf("%s\n",serMsg);
    }
    return 0; 
}

static void* WritingThread(void *arg) 
{ 
    char sendbuf[DEFAULT_BUFLEN];
    printf("Thread Created for write\n");
    while(1)
    {
        fgets(sendbuf, DEFAULT_BUFLEN, stdin);

        send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        memset(sendbuf,0,sizeof(sendbuf));
    }
    return 0;
}

void PromtForServer(char * ipaddr)
{
    FILE *fptr;
    char myString[10][DEFAULT_BUFLEN];
    char userinput[DEFAULT_BUFLEN];
    int loop = 0;

    printf("Select Server to connect:\n");
    fptr = fopen("../Server_config/server_lists.ini", "r"); 

    while(fgets(myString[loop], DEFAULT_BUFLEN, fptr)) {
        printf("%d: %s\n", loop,myString[loop]);
        loop++;
    }
    fclose(fptr);
    fgets(userinput, sizeof(userinput), stdin);
    
    strcpy(ipaddr,myString[atoi(userinput)]);
}

int CreateAccount()
{
    int SyncStat = 0;
    char serMsg[DEFAULT_BUFLEN];

    SyncStat = recv(ConnectSocket, serMsg, sizeof(serMsg), 0);
    if (SyncStat <= 0) {
        printf("server error"); 
    }
    printf("%s\n",serMsg);


    return SyncStat;
}