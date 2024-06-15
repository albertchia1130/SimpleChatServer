#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "36"
#define IP_ADDR "123.253.35.37"

WSADATA wsaData;
SOCKET ConnectSocket = INVALID_SOCKET;
DWORD   dwThreadIdArray1, dwThreadIdArray2;

DWORD WINAPI ReadingThread( LPVOID lpParam );
DWORD WINAPI WritingThread( LPVOID lpParam );

int __cdecl main(int argc, char **argv) 
{
    int iResult;
    HANDLE hThread1,hThread2;
    struct addrinfo *result = NULL,*ptr = NULL,hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    // Resolve the server address and port
    iResult = getaddrinfo(IP_ADDR, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
    printf("Connection Succeded\n");

    hThread1 =CreateThread(NULL,0,WritingThread,NULL,0,&dwThreadIdArray1);
    hThread2 =CreateThread(NULL,0,ReadingThread,NULL,0,&dwThreadIdArray2);


    WaitForSingleObject(hThread2,INFINITE);
    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}

DWORD WINAPI ReadingThread( LPVOID lpParam ) 
{ 
    int readStatus = 1;
    char serMsg[DEFAULT_BUFLEN];
    printf("Thread Created for read\n");
    while(1)
    {
       memset(serMsg, 0, (255*sizeof(char)));
       readStatus = recv(ConnectSocket, serMsg, sizeof(serMsg), 0);
       if (readStatus <= 0) {
            printf("read failed with error: %d\n", WSAGetLastError()); 
            printf("Server Closed\n");
            closesocket(ConnectSocket);
            WSACleanup();
            return 0;
        }
       printf("%s\n",serMsg);
    }
    return 0; 
}

DWORD WINAPI WritingThread( LPVOID lpParam ) 
{ 
    int writeStatus = 1;
    char sendbuf[DEFAULT_BUFLEN];
    printf("Thread Created for write\n");
    while(1)
    {
        gets(sendbuf);

        writeStatus = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
        memset(sendbuf,0,sizeof(sendbuf));

        if (writeStatus == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
    }
    return 0; 
} 