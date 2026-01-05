/******************************************************************************\
* simplec.c - Simple TCP/UDP client using Winsock 1.1
* 
*       This is a part of the Microsoft Source Code Samples.
*       Copyright 1996-1997 Microsoft Corporation.
*       All rights reserved.
*       This source code is only intended as a supplement to
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the
*       Microsoft samples programs.
\******************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include <afxsock.h>        // MFC socket extensions
//#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_PORT 5001
#define DEFAULT_PROTO SOCK_STREAM // TCP
long tcpWeight;
char TcpBuffer[128];

int ClientRequest(const char *server_name,char *command,CString &errmsg)
{
    char Buffer[12800];
	unsigned short port = DEFAULT_PORT;
	int retval, loopflag=0;
	unsigned int addr;
	int socket_type = DEFAULT_PROTO;
	struct sockaddr_in server;
	struct hostent *hp;
	WSADATA wsaData;
	SOCKET  conn_socket;

    socket_type = SOCK_STREAM;
	
	if (WSAStartup(0x202,&wsaData) == SOCKET_ERROR) {
        errmsg.Format("WSAStartup failed with error %d\n",WSAGetLastError());
        TRACE(errmsg);
		WSACleanup();
		return -1;
	}
	
	//
	// Attempt to detect if we should call gethostbyname() or
	// gethostbyaddr()

	if (isalpha(server_name[0])) {   /* server address is a name */
		hp = gethostbyname(server_name);
	}
	else  { /* Convert nnn.nnn address to a usable one */
		addr = inet_addr(server_name);
		hp = gethostbyaddr((char *)&addr,4,AF_INET);
	}
	if (hp == NULL ) {
        errmsg.Format("Client: Cannot resolve address [%s]: Error %d\n",
			server_name,WSAGetLastError());
        TRACE(errmsg);
		WSACleanup();
        return -1;
	}

	//
	// Copy the resolved information into the sockaddr_in structure
	//
	memset(&server,0,sizeof(server));
	memcpy(&(server.sin_addr),hp->h_addr,hp->h_length);
	server.sin_family = hp->h_addrtype;
	server.sin_port = htons(port);

	conn_socket = socket(AF_INET,socket_type,0); /* Open a socket */
	if (conn_socket <0 ) {
        errmsg.Format("Client: Error Opening socket: Error %d\n",
			WSAGetLastError());
        TRACE(errmsg);
		WSACleanup();
		return -1;
	}

	//
	// Notice that nothing in this code is specific to whether we 
	// are using UDP or TCP.
	// We achieve this by using a simple trick.
	//    When connect() is called on a datagram socket, it does not 
	//    actually establish the connection as a stream (TCP) socket
	//    would. Instead, TCP/IP establishes the remote half of the
	//    ( LocalIPAddress, LocalPort, RemoteIP, RemotePort) mapping.
	//    This enables us to use send() and recv() on datagram sockets,
	//    instead of recvfrom() and sendto()


    TRACE("Client connecting to: %s\n",hp->h_name);
	if (connect(conn_socket,(struct sockaddr*)&server,sizeof(server))
		== SOCKET_ERROR) {
        errmsg.Format("connect() failed: %d\n",WSAGetLastError());
        TRACE(errmsg);
		WSACleanup();
		return -1;
	}

	// cook up a string to send
	//
    strcpy(Buffer,command);
    retval = send(conn_socket,Buffer,sizeof(Buffer),0);
    if (retval == SOCKET_ERROR) {
        errmsg.Format("send() failed: error %d\n",WSAGetLastError());
        TRACE(errmsg);
        WSACleanup();
        return -1;
    }
    TRACE("Sent Data [%s]\n",Buffer);
    retval = recv(conn_socket,Buffer,sizeof (Buffer),0 );
    if (retval == SOCKET_ERROR) {
        errmsg.Format("recv() failed: error %d\n",WSAGetLastError());
        TRACE(errmsg);
        closesocket(conn_socket);
        WSACleanup();
        return -1;
    }
    //
    // We are not likely to see this with UDP, since there is no
    // 'connection' established.
    //
    if (retval == 0) {
        errmsg.Format("Server closed connection\n");
        TRACE(errmsg);
        closesocket(conn_socket);
        WSACleanup();
        return -1;
    }
    tcpWeight = atol(Buffer);
    TRACE("Received %d bytes, data [%s] from server\n",retval,Buffer);
    TRACE("Terminating connection\n");
	errmsg = Buffer;

	closesocket(conn_socket);
	WSACleanup();
	return 0;
}

int ADAMRequest(const char *server_name,char *command, BYTE *byData)
{
    char Buffer[128];
	unsigned short port = DEFAULT_PORT;
	int retval, loopflag=0;
	unsigned int addr;
	int socket_type = DEFAULT_PROTO;
	struct sockaddr_in server;
	struct hostent *hp;
	WSADATA wsaData;
	SOCKET  conn_socket;

    socket_type = SOCK_STREAM;
	
	if (WSAStartup(0x202,&wsaData) == SOCKET_ERROR) {
		WSACleanup();
		return -1;
	}
	
	//
	// Attempt to detect if we should call gethostbyname() or
	// gethostbyaddr()

	if (isalpha(server_name[0])) {   /* server address is a name */
		hp = gethostbyname(server_name);
	}
	else  { /* Convert nnn.nnn address to a usable one */
		addr = inet_addr(server_name);
		hp = gethostbyaddr((char *)&addr,4,AF_INET);
	}
	if (hp == NULL ) {
		WSACleanup();
        return -1;
	}

	//
	// Copy the resolved information into the sockaddr_in structure
	//
	memset(&server,0,sizeof(server));
	memcpy(&(server.sin_addr),hp->h_addr,hp->h_length);
	server.sin_family = hp->h_addrtype;
	server.sin_port = htons(port);

	conn_socket = socket(AF_INET,socket_type,0); /* Open a socket */
	if (conn_socket <0 ) {
		WSACleanup();
		return -1;
	}

	//
	// Notice that nothing in this code is specific to whether we 
	// are using UDP or TCP.
	// We achieve this by using a simple trick.
	//    When connect() is called on a datagram socket, it does not 
	//    actually establish the connection as a stream (TCP) socket
	//    would. Instead, TCP/IP establishes the remote half of the
	//    ( LocalIPAddress, LocalPort, RemoteIP, RemotePort) mapping.
	//    This enables us to use send() and recv() on datagram sockets,
	//    instead of recvfrom() and sendto()


    TRACE("Client connecting to: %s\n",hp->h_name);
	if (connect(conn_socket,(struct sockaddr*)&server,sizeof(server))
		== SOCKET_ERROR) {
		WSACleanup();
		return -1;
	}

	// cook up a string to send
	//
    strcpy(Buffer,command);
    retval = send(conn_socket,Buffer,sizeof(Buffer),0);
    if (retval == SOCKET_ERROR) {
        WSACleanup();
        return -1;
    }
    TRACE("Sent Data [%s]\n",Buffer);
    retval = recv(conn_socket,Buffer,sizeof (Buffer),0 );
    if (retval == SOCKET_ERROR) {
        closesocket(conn_socket);
        WSACleanup();
        return -1;
    }
    //
    // We are not likely to see this with UDP, since there is no
    // 'connection' established.
    //
    if (retval == 0) {
        closesocket(conn_socket);
        WSACleanup();
        return -1;
    }
    TRACE("Received %d bytes, data [%s] from server\n",retval,Buffer);
    TRACE("Terminating connection\n");
    for(int i=0; i<128; i++) {
        char ch = Buffer[i];
        if(ch=='0') {
            byData[i] = 0;
        }
        else {
            byData[i] = 1;
        }
    }

	closesocket(conn_socket);
	WSACleanup();
	return 0;
}
