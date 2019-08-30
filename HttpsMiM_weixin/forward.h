#ifndef FORWARD_H
#define FORWARD_H

#include <winsock2.h>
#include "const.h"
#include "ssl.h"

void ForwardHttp( LPVOID arg )
{
	HTTP_DATA_FORWARD	*http_data_forward = (HTTP_DATA_FORWARD *)arg;

	SOCKET	sock1 = http_data_forward->sock1;
	SOCKET	sock2 = http_data_forward->sock2;

	fd_set	fd_read;
	timeval	time_out;
	int		ret;
	
	time_out.tv_sec = TIME_OUT;
	time_out.tv_usec = 0;
	
	unsigned char	buffer[71024] = { 0 };

	while( TRUE )
	{
		FD_ZERO( &fd_read );
		FD_SET( sock1, &fd_read );
		FD_SET( sock2, &fd_read );

		ret = select( 0, &fd_read, NULL, NULL, &time_out );

		// socket出现错误
		if( SOCKET_ERROR == ret )
		{
			printf( "select error: %d\n", GetLastError() );
			break;
		}

		// 超时
		else if( 0 == ret )
		{
			//printf( "time out.\n" );

			closesocket( sock1 );
			closesocket( sock2 );
			break;
		}
		
		// sock1可读，则读取并写入到sock2
		if( FD_ISSET( sock1, &fd_read ) )
		{
			memset( (void *)buffer, 0, sizeof(buffer) );
			
			ret = recv( sock1, (char *)buffer, sizeof(buffer), 0 );
			if( ret <= 0 )
			{
				printf( "sock1 recv error: %d\n", GetLastError() );
				closesocket( sock1 );
				closesocket( sock2 );

				break;
			}

			printf( "%s", buffer );
			ret = send( sock2, (char *)buffer, ret, 0 );
			if( ret == SOCKET_ERROR )
			{	
				printf( "sock2 send error: %d\n", GetLastError() );
				closesocket( sock1 );
				closesocket( sock2 );

				break;
			}
		}

		// sock2可读，则读取并写入sock1
		if( FD_ISSET( sock2, &fd_read ) )
		{
			memset( (void *)buffer, 0, sizeof(buffer) );
			
			ret = recv( sock2, (char *)buffer, sizeof(buffer), 0 );
			if( ret <= 0 )
			{
				printf( "sock2 recv error: %d\n", GetLastError() );
				closesocket( sock1 );
				closesocket( sock2 );

				break;
			}

			//printf( "%s", buffer );
			ret = send( sock1, (char *)buffer, ret, 0 );
			if( ret == SOCKET_ERROR )
			{	
				printf( "sock1 send error: %d\n", GetLastError() );
				closesocket( sock1 );
				closesocket( sock2 );

				break;
			}	
		}
	}
}

void ForwardHttps( LPVOID arg )
{
	SSL_DATA_FORWARD	*ssl_data_forward = (SSL_DATA_FORWARD *)arg;

	SSL	*ssl1 = ssl_data_forward->ssl_session1.ssl;
	SOCKET	sock1 = ssl_data_forward->ssl_session1.sock;
	
	SSL	*ssl2 = ssl_data_forward->ssl_session2.ssl;
	SOCKET	sock2 = ssl_data_forward->ssl_session2.sock;

	fd_set	fd_read;
	timeval	time_out;
	int		ret;
	
	time_out.tv_sec = TIME_OUT;
	time_out.tv_usec = 0;
	
	unsigned char	buffer[710024] = { 0 };

	while( TRUE )
	{
		FD_ZERO( &fd_read );
		FD_SET( sock1, &fd_read );
		FD_SET( sock2, &fd_read );

		ret = select( 0, &fd_read, NULL, NULL, &time_out );

		// socket出现错误
		if( SOCKET_ERROR == ret )
		{
			printf( "%d select error: %d\n", WSAECONNRESET, GetLastError() );

			SSL_shutdown( ssl1 );
			SSL_free( ssl1 );

			SSL_shutdown( ssl2 );
			SSL_free( ssl2 );

			closesocket( sock1 );
			closesocket( sock2 );
			break;
		}

		// 超时
		else if( 0 == ret )
		{
			printf( " time out,shutdown sockets\n" );
			//continue;

			SSL_shutdown( ssl1 );
			SSL_free( ssl1 );

			SSL_shutdown( ssl2 );
			SSL_free( ssl2 );

			closesocket( sock1 );
			closesocket( sock2 );

			break;
		}
		
		// ssl1可读，则读取并写入到ssl2
		if( FD_ISSET( sock1, &fd_read ) )
		{
			memset( (void *)buffer, 0, sizeof(buffer) );
			
			ret = SSL_read( ssl1, (char *)buffer, sizeof(buffer) );
			if( ret <= 0 )
			{
				SSL_Error( "ssl1 recv erro" );

				SSL_shutdown( ssl1 );
				SSL_free( ssl1 );

				SSL_shutdown( ssl2 );
				SSL_free( ssl2 );

				closesocket( sock1 );
				closesocket( sock2 );

				break;
			}

			printf( "%s", buffer );
			ret = SSL_write( ssl2, (char *)buffer, ret );
			if( ret <= 0 )
			{	
				SSL_Error( "ssl2 send error" );
				
				SSL_shutdown( ssl1 );
				SSL_free( ssl1 );

				SSL_shutdown( ssl2 );
				SSL_free( ssl2 );

				closesocket( sock1 );
				closesocket( sock2 );

				break;
			}
		}

		// ssl2可读，则读取并写入ssl1
		if( FD_ISSET( sock2, &fd_read ) )
		{
			memset( (void *)buffer, 0, sizeof(buffer) );
			
			ret = SSL_read( ssl2, (char *)buffer, sizeof(buffer) );
			if( ret <= 0 )
			{
				//SSL_Error( "ssl2 recv erro" );
				printf( "解密接收数据完毕\n\n" );

				SSL_shutdown( ssl1 );
				SSL_free( ssl1 );

				SSL_shutdown( ssl2 );
				SSL_free( ssl2 );

				closesocket( sock1 );
				closesocket( sock2 );

				break;
			}

			//printf( "%s", buffer );
			ret = SSL_write( ssl1, (char *)buffer, ret );
			if( ret <= 0 )
			{	
				//SSL_Error( "ssl1 send erro" );
				printf( "解密发送数据完毕\n\n" );

				SSL_shutdown( ssl1 );
				SSL_free( ssl1 );

				SSL_shutdown( ssl2 );
				SSL_free( ssl2 );

				closesocket( sock1 );
				closesocket( sock2 );

				break;
			}	
		}
	}
}

#endif
