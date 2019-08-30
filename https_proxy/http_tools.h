#include <stdio.h>
#include <vector>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>

using namespace std;

char *strstr_d(char *plain, int plen, char *word, int wlen);
int hostname_to_ip(char *hostname, char *server_ip = 0);
int get_hostname(char *meesage, int mlen, char *hostname);
void print_error(char *message);