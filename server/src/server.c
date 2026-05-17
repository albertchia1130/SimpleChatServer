

#include <netinet/in.h> //structure for storing address information 
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/socket.h> //for socket APIs 
#include <sys/types.h> 
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <syslog.h>

#define messageSize 255

typedef struct ClientAttr{
    char  Name[10];
    struct ClientAttr* nextClient;
    int ClientID;

}Client_number;

int CreateNAttachClient(int clientSocket); //Create a thread for each client attached
int FreeNDetachClient(Client_number* Client); //Exit thread and free up resource when client exit
void BroadcastMessage(int user, char* Message); //broadcast message received from a client to all subscriber
static void* ClientMessageFunc(void *arg);

Client_number* HeadListofClient; //Starting point of client list;
  
int main(int argc, char const* argv[]) 
{ 
    int clientSocket =0;
    // create server socket similar to what was done in 
    // client program 
    int servSockD = socket(AF_INET, SOCK_STREAM, 0); 
    // define server address 
    struct sockaddr_in servAddr; 
  
    servAddr.sin_family = AF_INET; 
    servAddr.sin_port = htons(8080); 
  
    // bind socket to the specified IP and port 
    bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)); 
  
    // listen for connections 
    printf("phase 1\n");
    listen(servSockD, SOMAXCONN); 
    printf("Entering Listen\n");
    
    // send's messages to client socket 
    while(1)
    {
        clientSocket = accept(servSockD, NULL, NULL);
        syslog(LOG_INFO, "New User");
        printf("A new connection\n");
        CreateNAttachClient(clientSocket); //Attach client Socket to the list
    }
    return 0; 
}

static void* ClientMessageFunc(void *arg)
{
    Client_number* Client = (Client_number* ) arg;
    int SocketId = Client->ClientID;
    char serMsg[messageSize];
    int readStatus = 1;
    while(readStatus)
    {
        memset(serMsg, 0, (255*sizeof(char)));
        readStatus = read(SocketId, serMsg, sizeof(serMsg), MSG_OOB);
        if(readStatus <= 0)
        {
            printf("Client Has Exited\n");
            FreeNDetachClient(Client);
            pthread_exit(0);
        }
        BroadcastMessage( SocketId, serMsg);
        printf("user %d sent:%s\n", SocketId, serMsg);
    }
}

int CreateNAttachClient(int clientSocket)
{
    pthread_t thread_id;
    Client_number* ClientNumber;
    Client_number* Index = HeadListofClient;
    ClientNumber = (Client_number *)calloc(1,sizeof(Client_number));
    ClientNumber->ClientID = clientSocket;
    if(Index == NULL)
    {
        HeadListofClient = ClientNumber; //Attached to the list
    }
    else
    {
        while(Index != NULL)
        {
            if (Index->nextClient ==NULL)
            {
                Index->nextClient = ClientNumber; //Attach Client to the list.
                break;
            }
            else
            {
                Index  = Index->nextClient;
            }
        }
    }

    pthread_create(&thread_id, NULL, &ClientMessageFunc, (void *)ClientNumber);
}

int FreeNDetachClient(Client_number* Client)
{
    Client_number* Index = HeadListofClient;
    if(Index == Client)
    {
        free(Client);
        return 0;
    }
    while(Index != NULL)
    {
        if (Index->nextClient == Client)
        {
            Index->nextClient = Client->nextClient; //Detach Client from the list.
            free(Client);
            printf("Chat has deleted users:\n");
            break;
        }
        else if(Index->nextClient != NULL)
        {
            Index  = Index->nextClient;
        }
        else
        {
            printf("Client not found\n");
            break;
        }
    }
    return 0;
}


void BroadcastMessage(int user, char* Message)
{
    Client_number* Index = HeadListofClient;
    char SendMessage[50];
    sprintf(SendMessage, "user %d sent: ",user);
    strcat(SendMessage,Message);
    while(Index != NULL)
    {
        send( Index->ClientID, SendMessage, strlen(SendMessage), 0 );
        if (Index->nextClient ==NULL)
        {
            break;
        }
        else
        {
           Index= Index->nextClient;
        }
    }
}