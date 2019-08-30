#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

int create_server_socket(int port);
int create_client_socket(int port, char *server_ip);
void init_openssl();
void cleanup_openssl();
SSL_CTX *create_context();
int configure_context(SSL_CTX *ctx, char *cert_file, char *pri_key_file);
void ShowCerts(SSL* ssl);