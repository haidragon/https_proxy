#ifndef HTTPS_H
#define HTTPS_H

#include <openssl/ssl.h>

#include "socks.h"
#include "const.h"
#include "forward.h"
#include "ssl.h"

void WINAPI	FakeSslServer( LPVOID arg )
{
	char	* real_server_ip = (char *)arg;

	// �����˿ڣ���Ϊα���HTTP�������ṩ����
	SOCKET	fake_server = ListenPort( SSL_PORT );
	if( INVALID_SOCKET == fake_server )
	{
		return;
	}

	SSL_DATA_FORWARD	ssl_data_forward;

	SOCKADDR_IN		real_client_in;
	int	addr_len = sizeof(SOCKADDR_IN);
	memset( (void *)&real_client_in, 0, addr_len );

	HANDLE	thread_handle;
	DWORD	thread_id;

	while( TRUE )
	{
		// ������ʵ�û�������
		SOCKET	real_client = accept( fake_server, (struct sockaddr *)&real_client_in, &addr_len );
		if( SOCKET_ERROR == real_client )
		{
			printf( "Accept error: %d\n", GetLastError() );
			continue;
		}

		//printf( "Connected from %d\n", real_client );

		// ��Ϊ�ٷ�������ssl
		SSL	*fake_server_ssl = Server_SSL_Init( );
		if( NULL == fake_server_ssl )
		{
			continue;
		}
		SSL_set_fd( fake_server_ssl, (int)real_client );

		//printf( "Waiting ssl handshark\n" );
		// ��ͻ��˽���SSL����
		int ret = SSL_accept( fake_server_ssl );
		if( 1 != ret )
		{
			SSL_Error( "SSL_accept error" );
			continue;
		}
		//printf( "SSL handshark ok.\n" );

		// αװ���û����ӵ���ʵ�ķ�����
		SOCKET fake_client = ConnectTarget( real_server_ip, /*SSL_PORT*/443 );
		if( fake_client == INVALID_SOCKET )
		{
			closesocket( real_client );
			continue;
		}

		// ��Ϊ�ٿͻ��˵�ssl
		SSL	*fake_client_ssl = Client_SSL_Init( );
		if( NULL == fake_client_ssl )
		{
			return;
		}
		SSL_set_fd( fake_client_ssl, (int)fake_client );

		ret = SSL_connect( fake_client_ssl );
		if( 1 != ret )
		{
			SSL_Error( "SSL_connect error" );
			return;
		}
		printf( "SSL_connect!\n" );

		ssl_data_forward.ssl_session1.sock = real_client;
		ssl_data_forward.ssl_session1.ssl = fake_server_ssl;

		ssl_data_forward.ssl_session2.sock = fake_client;
		ssl_data_forward.ssl_session2.ssl = fake_client_ssl;

		// �������߳�����ʵ�����������ǿͻ���֮��ת������
		thread_handle = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ForwardHttps, (LPVOID)(&ssl_data_forward), 0, &thread_id );
		if( NULL == thread_handle )
		{
			printf( "CreateThread for forward http data error: %d\n", GetLastError() );
			continue;
		}

		CloseHandle( thread_handle );
	}
}

#endif
