#ifndef SOCKS_H

#define SOCKS_H

#include <winsock2.h>
#include <stdio.h>

#include "const.h"

// 加载WinSock库
BOOL	LoadSockLib( )
{
	WSADATA		wsa;
	
	int ret = WSAStartup( 0x0202, &wsa );
	if( 0 == ret )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void	CleanSockLib( )
{
	WSACleanup( );
}

// 监听指定端口
SOCKET	ListenPort( int listen_port )
{
	SOCKET	sock = socket( AF_INET, SOCK_STREAM, 0 );
	if( INVALID_SOCKET == sock )
	{
		printf( "Create socket as a server error: %d", GetLastError() );
		return INVALID_SOCKET;
	}
	
	SOCKADDR_IN	sin;
	memset( (void *)&sin, 0, sizeof(SOCKADDR_IN) );

	sin.sin_family = AF_INET;
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	sin.sin_port = htons( listen_port );

	int	ret = bind( sock, (struct sockaddr *)&sin, sizeof(SOCKADDR_IN) );
	if( SOCKET_ERROR == ret )
	{
		printf( "Bind %d error: %d\n", listen_port, GetLastError() );
		return INVALID_SOCKET;
	}

	ret = listen( sock, MAX_USER );
	if( SOCKET_ERROR == ret )
	{
		printf( "Listen %d error: %d\n", listen_port, GetLastError() );
		return INVALID_SOCKET;
	}
	
	return sock;
}

// 连接到指定IP的指定端口
SOCKET	ConnectTarget( char * ip, int port )
{
	SOCKET	sock = socket( AF_INET, SOCK_STREAM, 0 );

	if( INVALID_SOCKET == sock )
	{
		printf( "create socket as a client error.\n" );
		return INVALID_SOCKET;
	}
	
	SOCKADDR_IN		sin;
	memset( (void *)&sin, 0, sizeof(SOCKADDR_IN) );

	sin.sin_family = AF_INET;
	sin.sin_addr.S_un.S_addr = inet_addr( ip );
	sin.sin_port = htons( port );

	int ret = connect( sock, (struct sockaddr *)&sin, sizeof(SOCKADDR_IN) );
	if( SOCKET_ERROR == ret )
	{
		printf( "connect to real server %s:%d error\n", ip, port );
		return INVALID_SOCKET;
	}

	return sock;
}

#endif