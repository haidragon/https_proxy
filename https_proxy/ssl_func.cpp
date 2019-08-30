#include "ssl_func.h"
#include<cstring>
int create_server_socket(int port){
    int s;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return -1;
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) return -1;
    if (listen(s, 1) < 0) return -1;

    return s;
}

int create_client_socket(int port, char *server_ip)
{
    int serv_sock;
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(server_ip);
	serv_addr.sin_port = htons(port);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    if (serv_sock == -1) return -1;
    if (connect(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) return -1;

    return serv_sock;
}

void init_openssl(){
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl(){
    EVP_cleanup();
}

SSL_CTX *create_context(){
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
	    perror("Unable to create SSL context");
	    ERR_print_errors_fp(stderr);
	    exit(EXIT_FAILURE);
    }
    return ctx;
}

int configure_context(SSL_CTX *ctx, char *cert_file, char *pri_key_file){
    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0) return -1;

    if (SSL_CTX_use_PrivateKey_file(ctx, pri_key_file, SSL_FILETYPE_PEM) <= 0 ) return -1;
    
    if(!SSL_CTX_check_private_key(ctx)) return -1;

    return 0;
}
