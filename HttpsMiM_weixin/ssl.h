#ifndef SSL_H
#define SSL_H

#include "openssl/ssl.h"
#include "openssl/err.h"
#include <stdio.h>

#include "const.h"

void SSL_Error( char *custom_string )
{
	char	error_buffer[1024] = { 0 };
     
	ERR_error_string( ERR_get_error(), error_buffer );

	fprintf( stderr, "%s: %s\n", custom_string, error_buffer );
}

SSL	*Server_SSL_Init( )
{
	// 加载SSL环境
	SSL_CTX	*server_ctx = SSL_CTX_new( SSLv23_server_method() );
	if( NULL == server_ctx )
	{
		SSL_Error( "Init ssl ctx error" );
		return NULL;
	}

	// 设置证书文件的口令
	//SSL_CTX_set_default_passwd_cb_userdata( server_ctx, NULL );//PRIVATE_KEY_PWD

	// 加载证书
	//if( SSL_CTX_use_certificate_file( server_ctx, fake_cert, SSL_FILETYPE_PEM ) <= 0 )
	if (SSL_CTX_use_certificate_file(server_ctx, "C:\\Users\\Administrator\\Desktop\\HttpsMiM\\HttpsMiM_weixin\\HttpsMiM_weixin\\Release\\cet\\ca.crt", SSL_FILETYPE_PEM) <= 0)
	{
		SSL_Error( "Load cert file error" );
		return NULL;
	}

	// 加载私钥 C:\\Users\\Administrator\\Desktop\\HttpsMiM\\HttpsMiM_weixin\\HttpsMiM_weixin\\Release\\corp.crt"
	//if( SSL_CTX_use_PrivateKey_file( server_ctx, KEY_FILE, SSL_FILETYPE_PEM ) <= 0 )
	if (SSL_CTX_use_PrivateKey_file(server_ctx, "C:\\Users\\Administrator\\Desktop\\HttpsMiM\\HttpsMiM_weixin\\HttpsMiM_weixin\\Release\\cet\\ca.key", SSL_FILETYPE_PEM) <= 0)
	{
		SSL_Error( "Load cert file error" );
		return NULL;
	}

	// 检查私钥和证书是否匹配
	if( !SSL_CTX_check_private_key( server_ctx ) )
	{
		printf( "Private key does not match the certificate public key\n" );
		return NULL;
	}

	SSL	*ssl = SSL_new(server_ctx);
	if( NULL == ssl )
	{
		SSL_Error( "Create ssl error" );
		return NULL;
	}

	return ssl;
}

SSL	*Client_SSL_Init( )
{
	SSL_CTX	*client_ctx;

	client_ctx = SSL_CTX_new( SSLv23_client_method() );

	if( NULL == client_ctx )
	{
		SSL_Error( "Init ssl ctx error" );
		return NULL;
	}

	SSL	*ssl = SSL_new (client_ctx);
	if( NULL == ssl )
	{
		SSL_Error( "Create ssl error" );
		return NULL;
	}

	return ssl;
}

#endif
