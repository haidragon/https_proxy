#ifndef HTTP_H
#define HTTP_H

#include "socks.h"
#include "const.h"
#include "forward.h"

void WINAPI	FakeHttpServer( LPVOID real_target_ip )
{
	char	* real_server_ip = (char *)real_target_ip;

	// �����˿ڣ���Ϊα���HTTP�������ṩ����
	SOCKET	fake_server = ListenPort( HTTP_PORT );
	if( INVALID_SOCKET == fake_server )
	{
		return;
	}

	// ��ʵ���û����ӹ���
	SOCKET			real_client;
	SOCKADDR_IN		real_client_in;
	
	int	addr_len = sizeof(SOCKADDR_IN);
	memset( (void *)&real_client_in, 0, addr_len );

	HANDLE	thread_handle;
	DWORD	thread_id;
	
	while( TRUE )
	{
		HTTP_DATA_FORWARD	http_data_forward;

		// ������ʵ�û�������
		real_client = accept( fake_server, (struct sockaddr *)&real_client_in, &addr_len );

		if( SOCKET_ERROR == real_client )
		{
			printf( "Accept error: %d\n", GetLastError() );
			continue;
		}
		
		http_data_forward.sock1 = real_client;

		// αװ���û����ӵ���ʵ�ķ�����
		SOCKET fake_client = ConnectTarget( real_server_ip, 80 );
		if( fake_client == INVALID_SOCKET )
		{
			closesocket( real_client );
			continue;
		}

		http_data_forward.sock2 = fake_client;

		// �������߳�����ʵ�����������ǿͻ���֮��ת������
		thread_handle = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ForwardHttp, (LPVOID)(&http_data_forward), 0, &thread_id );
		if( NULL == thread_handle )
		{
			printf( "CreateThread for forward http data error: %d\n", GetLastError() );
			continue;
		}

		CloseHandle( thread_handle );
	}
}

#endif
