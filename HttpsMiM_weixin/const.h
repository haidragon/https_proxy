#ifndef CONST_H

#define CONST_H

#include <winsock2.h>
#include "openssl/ssl.h"

/////////////////////////////////////////////////////////////////////////////////////////
// ����ȫ�ֱ���
/////////////////////////////////////////////////////////////////////////////////////////
#define	PRIVATE_KEY_PWD		"123456"						// ˽Կ������
#define	CERT_FILE			"ca.crt"				// α���֤�飬Ĭ��
#define	KEY_FILE			"ca.key"				// α��֤���˽Կ
#define	TIME_OUT			15							// Select��ʱ
#define	MAX_USER			100							// �������ӵ�����û���
#define	SSL_PORT			995							// HTTPS�Ķ˿�
#define	HTTP_PORT			800							// HTTP�Ķ˿�

char	fake_cert[512] = { 0 };							// ����Ŀ���޸Ķ��ɵ�֤��					

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
