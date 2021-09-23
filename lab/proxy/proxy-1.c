#include <stdio.h>
#include <strings.h>
#include <assert.h>
#include <netdb.h>


#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: Keep-Alive\r\n";
static const char *proxy_hdr = "Proxy-Connection: Keep-Alive\r\n";
static const char *host_hdr_format = "Host: %s\r\n";
static const char *requestlint_hdr_format = "GET %s HTTP/1.0\r\n";
static const char *endof_hdr = "\r\n";

static const char *connection_key = "Connection";
static const char *user_agent_key = "User-Agent";
static const char *proxy_connection_key = "Proxy-Connection";
static const char *host_key = "Host";

void doit(int clientfd);
void client_error(int fd, char *msg);
int parse_uri(char *uri, char *hostname, char *port, char *filename, char *cgiargs);
void build_hdrs(char *hdrs, char *hostname, char *port, char *req_hdrs, rio_t *rio);

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
        printf("Accept connection from (%s, %s), connfd: %d\n", hostname, port, connfd);
        doit(connfd);
        Close(connfd);
    }
    return 0;
}

void doit(int clientfd) {
    int is_static, n;
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


    /* build headers */
    ptr = uri;
    while (strstr(ptr, "/"))
        ptr = strstr(ptr, "/") + 1;

    sprintf(req, requestlint_hdr_format, ptr - 1);
    build_hdrs(hdrs, hostname, port, req, &client_rio);

    printf("%s\n", hdrs);

    /* connect server */
    serverfd = Open_clientfd(hostname, port);
    if (serverfd < 0)
        unix_error("connection failed!");

    /* send request and headers to server */
    Rio_writen(serverfd, hdrs, strlen(hdrs));


    /* receive server answer and send to client */
    Rio_readinitb(&server_rio, serverfd);
    while ((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0) {
        printf("proxy received %d bytes,then send\n", n);
        Rio_writen(clientfd, buf, n);
    }

    Close(serverfd);
}

void client_error(int fd, char *msg) {
    char buf[MAXLINE];
    sprintf(buf, msg);
    Rio_writen(fd, buf, strlen(buf));
}

void build_hdrs(char *hdrs, char *hostname, char *port, char *req_hdrs, rio_t *rio) {
    char buf[MAXLINE], host_hdrs[MAXLINE], other_hdrs[MAXLINE];
    while (Rio_readlineb(rio, buf, MAXLINE) > 0) {
        /* EOF */
        if (strcmp(buf, "\r\n") == 0) break;
        if (!strncasecmp(buf, host_key, strlen(host_key))) {
            strcpy(host_hdrs, buf);
            continue;
        }
        if (!strncasecmp(buf, connection_key, strlen(connection_key)) &&
            !strncasecmp(buf, proxy_connection_key, strlen(proxy_connection_key)) &&
            !strncasecmp(buf, user_agent_key, strlen(user_agent_key)))
            strcat(other_hdrs, buf);
    }
    if (strlen(host_hdrs) == 0)
        sprintf(host_hdrs, host_hdr_format, hostname);
    sprintf(hdrs, "%s%s%s%s%s%s%s",
            req_hdrs,
            host_hdrs,
            conn_hdr,
            proxy_hdr,
            user_agent_hdr,
            other_hdrs,
            endof_hdr);
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