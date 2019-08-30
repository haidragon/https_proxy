#include "ssl_func.h"
#include "http_tools.h"
#include <pthread.h>
#define MESSAGE_SIZE 3010

void *client_to_server(void *arg);
void *server_to_client(void *arg);
char message[MESSAGE_SIZE];
int main(int argc, char **argv)
{
    pthread_t t_id;

    int sock;
    SSL_CTX *ctx;

    init_openssl();
    ctx = create_context();

    if(configure_context(ctx, "default.pem", "default.pem") == -1) {
        print_error("configure_context error!\n");
    }

    sock = create_server_socket(4433);
    if(sock == -1) print_error("create_srever_socket error!\n");

    char server_ip[200];
    /* Handle connections */
    while(1) {
        struct sockaddr_in addr;
        uint len = sizeof(addr);
        SSL *ssl_client, *ssl_server;
        int client = accept(sock, (struct sockaddr*)&addr, &len);
        if (client < 0) print_error("client socket accept error!\n");

        ssl_client = SSL_new(ctx);
        if(ssl_client == 0) print_error("SSL_new error!");

        SSL_set_fd(ssl_client, client);
         
        if (SSL_accept(ssl_client) <= 0) print_error("SSL_accept(ssl_client) error!\n");
        printf("hi\n");
        
        int mlen = SSL_read(ssl_client, message, sizeof(message));
        printf("%s\n", message);
        char *hostname = (char *)malloc(100);
        get_hostname(message, mlen, hostname);
        hostname_to_ip(hostname, server_ip);

        int server = create_client_socket(443, server_ip);
        if(server == -1) print_error("create_client_socket error!\n");

        ssl_server = SSL_new(ctx);
        if(ssl_server == 0) print_error("SSL_new(ssl_server) error!\n");

        SSL_set_fd(ssl_server, server);
        if(SSL_connect(ssl_server) == 0) print_error("SSL_connect error!\n");

        SSL *arg[2] = {ssl_client, ssl_server};
        pthread_create(&t_id, NULL, client_to_server, arg);
		pthread_detach(t_id);

        pthread_create(&t_id, NULL, server_to_client, arg);
		pthread_detach(t_id);

        SSL_free(ssl_client);   SSL_free(ssl_server);
        close(client);
    }

    close(sock);
    SSL_CTX_free(ctx);
    cleanup_openssl();
}

void *client_to_server(void *arg) {
	SSL *client = (SSL *)arg;
    SSL *server = ((SSL *)arg) + 1;

	printf("-----------------------------------\n");
	printf("c2s connect_success!!\n");
	printf("-----------------------------------\n");

    int mlen = 0;
	while((mlen = SSL_read(client, message, sizeof(message))) != 0) SSL_write(server, message, mlen);

	SSL_free(client);   SSL_free(server);
}

void *server_to_client(void *arg) {
	SSL *client = (SSL *)arg;
    SSL *server = ((SSL *)arg) + 1;

	printf("-----------------------------------\n");
	printf("s2c connect_success!!\n");
	printf("-----------------------------------\n");

    int mlen = 0;
	while((mlen = SSL_read(server, message, sizeof(message))) != 0) SSL_write(client, message, mlen);

	SSL_free(client);   SSL_free(server);
}