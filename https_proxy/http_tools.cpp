#include "http_tools.h"

char *strstr_d(char *plain, int plen, char *word, int wlen){
    vector<int> failure(wlen);
    failure[0] = -1;
    for(int i= 1 ; i < wlen ; i++){
        int pos = failure[i - 1];
        while(pos != -1 && word[pos + 1] != word[i]) pos = failure[pos];
        if(word[pos + 1] == word[i]) failure[i] = pos + 1;
        else failure[i] = -1;
    }

    int pos = -1;
    for(int i = 0 ; i < plen ; i++){
        while(pos != -1 && word[pos + 1] != plain[i]) pos = failure[pos];
        if(word[pos + 1] == plain[i]) pos = pos + 1;
        if(pos == wlen - 1) return plain + (i - wlen + 1);
    }

    return 0;
}

int hostname_to_ip(char *hostname, char *ip){
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if((he = gethostbyname(hostname)) == NULL){
        herror("gethostbyname");
        return 0;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for(i= 0 ; addr_list[i] != NULL ; i++){
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 1;
    }
    return 0;
}

int get_hostname(char *message, int mlen, char *hostname){      //return hostname's length, if error then -1
    char *start_pos = strstr_d(message, mlen, "Host: ", strlen("Host: "));
    if(start_pos == 0) return -1;
    start_pos += strlen("Host: ");
    
    char *end_pos = strstr_d(start_pos, mlen - (start_pos - message), "\x0d\x0a", strlen("\x0d\x0a"));
    if(end_pos == 0) return -1;

    int hlen = end_pos - start_pos;
    for(int i = 0 ; i < hlen ; i++){
        if(start_pos[i] ==  ':') {
            hlen = i;
            break;
        }
    }

    strncpy(hostname, start_pos, hlen);  hostname[hlen] = 0;
    return hlen;
}

void print_error(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}