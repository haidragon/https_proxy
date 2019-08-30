#ifndef CONST_H

#define CONST_H

#include <winsock2.h>
#include "openssl/ssl.h"

/////////////////////////////////////////////////////////////////////////////////////////
// 定义全局变量
/////////////////////////////////////////////////////////////////////////////////////////
#define	PRIVATE_KEY_PWD		"123456"						// 私钥的密码
#define	CERT_FILE			"ca.crt"				// 伪造的证书，默认
#define	KEY_FILE			"ca.key"				// 伪造证书的私钥
#define	TIME_OUT			15							// Select超时
#define	MAX_USER			100							// 允许连接的最大用户数
#define	SSL_PORT			995							// HTTPS的端口
#define	HTTP_PORT			800							// HTTP的端口

char	fake_cert[512] = { 0 };							// 根据目标修改而成的证书					

typedef struct _http_data_forward
{
	SOCKET	sock1;
	SOCKET	sock2;
}HTTP_DATA_FORWARD;

typedef struct _ssl_data_forward
{
	struct _ssl_session1
	{
		SOCKET	sock;
		SSL		*ssl;
	}ssl_session1;

	struct _ssl_session2
	{
		SOCKET	sock;
		SSL		*ssl;
	}ssl_session2;

}SSL_DATA_FORWARD;

#endif
