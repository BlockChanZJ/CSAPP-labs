#include <stdio.h>
#include <strings.h>
#include <assert.h>


#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";


void doit(int clientfd);
void client_error(int fd, char *msg);
void read_client_hdrs(rio_t *rp, char *hdrs);
int parse_uri(char *uri, char *hostname, char *port, char *filename, char *cgiargs);

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;


    /* check command line */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }


    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accept connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        Close(connfd);
    }
    return 0;
}

void doit(int clientfd) {
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE], hostname[MAXLINE], port[MAXLINE];
    char hdrs[MAXLINE], req[MAXLINE];
    char *ptr;
    rio_t client_rio, server_rio;

    int serverfd, connfd;
    socklen_t serverlen;
    struct sockaddr serveraddr;

    /* read client request */
    Rio_readinitb(&client_rio, clientfd);
    Rio_readlineb(&client_rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);
    is_static = parse_uri(uri, hostname, port, filename, cgiargs);

    /* read client headers and construct server headers */
    strcpy(hdrs, user_agent_hdr);
    read_client_hdrs(&client_rio, hdrs);

    /* construct request */
    ptr = uri;
    while (strstr(ptr, "/"))
        ptr = strstr(ptr, "/") + 1;
    sprintf(req, "%s %s %s", method, ptr - 1, version);


    FILE *f = fopen("qwq.txt", "a+");
    fprintf(f, "===============CONSTRUCT=============\n");
    fprintf(f, "REQUEST:\n");
    fprintf(f, "%s\n", req);
    fprintf(f, "HEADERS:\n");
    fprintf(f, "%s\n", hdrs);
    fclose(f);

    /* connect server */
    serverfd = Open_clientfd(hostname, port);
    serverlen = sizeof(serveraddr);
    Connect(serverfd, &serveraddr, serverlen);

    /* send request and headers to server */
    Rio_writen(serverfd, buf, strlen(buf));
    Rio_writen(serverfd, hdrs, strlen(hdrs));

    /* receive server answer */
    Rio_readinitb(&server_rio, serverfd);
    Rio_readlineb(&server_rio, buf, MAXLINE);

//    FILE *f = fopen("qwq.txt", "a+");
//    fprintf(f, "===============RECEIVE=============\n");
//    fprintf(f, "%s\n\n", buf);
//    fclose(f);

    /* send answer to client */
    Rio_writen(clientfd, buf, strlen(buf));
}

void client_error(int fd, char *msg) {
    char buf[MAXLINE];
    sprintf(buf, msg);
    Rio_writen(fd, buf, strlen(buf));
}

void read_client_hdrs(rio_t *rp, char *hdrs) {
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n")) {          //line:netp:readhdrs:checkterm
        Rio_readlineb(rp, buf, MAXLINE);
        strcat(hdrs, buf);
    }
}

int parse_uri(char *uri, char *hostname, char *port, char *filename, char *cgiargs) {
    char *ptr1, *ptr2;

    /* hostname */
    ptr1 = strstr(uri, "://") + 3;
    ptr2 = strstr(ptr1, ":");
    *ptr2 = '\0';
    strcpy(hostname, ptr1);
    *ptr2 = ':';

    /* port */
    ptr1 = strstr(ptr1, ":") + 1;
    ptr2 = strstr(ptr1, "/");
    *ptr2 = '\0';
    strcpy(port, ptr1);
    *ptr2 = '/';


    if (!strstr(uri, "cgi-bin")) { /* static content */
        ptr1 = strstr(ptr1, "/");
        strcpy(filename, ".");
        if (uri[strlen(uri) - 1] == '/')
            strcat(filename, "/home.html");
        else
            strcat(filename, ptr1);
        return 1;
    } else { /* dynamic content */
        ptr1 = strstr(ptr1, "/");
        ptr2 = strstr(ptr1, "?");
        if (ptr2) {
            *ptr2 = '\0';
            strcpy(filename, ".");
            strcat(filename, ptr1);
            *ptr2 = '?';
        } else {
            /* no args */
            assert(0);
        }
        ptr1 = index(uri, '?');
        if (ptr1) {
            strcpy(cgiargs, ptr1 + 1);
            *ptr1 = '\0';
        } else
            strcpy(cgiargs, "");
        return 0;
    }
}

//    FILE *f = fopen("qwq.txt", "a+");
//    fprintf(f, "method: %s, uri: %s, version: %s\n", method, uri, version);
//    fprintf(f, "uri: %s, filename: %s, cgiargs: %s, hostname: %s, port: %s\n", uri, filename, cgiargs, hostname, port);
//    fclose(f);