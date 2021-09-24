#include <stdio.h>
#include <strings.h>
#include <assert.h>
#include <netdb.h>


#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define CACHE_BLOCK_SIZE 10

#define SBUF_SIZE 16
#define NTHREADS 4

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


typedef struct {
    int *buf;
    int n;
    int front;
    int rear;
    sem_t mutex;
    sem_t slots;
    sem_t items;
} sbuf_t;

typedef struct {
    char obj[MAX_OBJECT_SIZE];
    char url[MAXLINE];
    int lru;
    int is_empty;

    int reader_cnt;
    sem_t w_mutex;
    sem_t r_mutex;
} cache_block_t;

typedef struct {
    cache_block_t block[CACHE_BLOCK_SIZE];
} cache_t;

sbuf_t sbuf;
cache_t cache;
int global_lru;
sem_t lru_mutex;


/* sbuf helper */
void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);

/* cache helper */
void cache_insert(char *obj, char *url);
int cache_find(char *url);
void cache_init();
void cache_deinit();

/* thread function */
void *thread(void *vargp);

/* functions */
void doit(int clientfd);
int parse_uri(char *uri, char *hostname, char *port, char *filename, char *cgiargs);
void build_hdrs(char *hdrs, char *hostname, char *port, char *req_hdrs, rio_t *rio);


int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid[NTHREADS];


    /* check command line */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    sbuf_init(&sbuf, SBUF_SIZE);
    cache_init();
    for (int i = 0; i < NTHREADS; i++)
        Pthread_create(&tid[i], NULL, thread, NULL);

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accept connection from (%s, %s)\n", hostname, port);
        sbuf_insert(&sbuf, connfd);
    }

    sbuf_deinit(&sbuf);

    return 0;
}

void *thread(void *vargp) {
    Pthread_detach(Pthread_self());
    while (1) {
        int connfd = sbuf_remove(&sbuf);
        doit(connfd);
        Close(connfd);
    }
}

void doit(int clientfd) {
    int is_static, n, idx;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE], hostname[MAXLINE], port[MAXLINE];
    char hdrs[MAXLINE], req[MAXLINE];
    char obj[MAX_OBJECT_SIZE];
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

    if ((idx = cache_find(uri)) != -1) {
        Rio_writen(clientfd, cache.block[idx].obj, strlen(cache.block[idx].obj));
        P(&cache.block[idx].r_mutex);
        cache.block[idx].reader_cnt--;
        if (cache.block[idx].reader_cnt == 0)
            V(&cache.block[idx].w_mutex);
        V(&cache.block[idx].r_mutex);
    } else {
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
            strcat(obj, buf);
        }
        cache_insert(obj, uri);
        Close(serverfd);
    }
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

/**************
 * sbuf helper
 **************/
void sbuf_init(sbuf_t *sp, int n) {
    sp->buf = Calloc(n, sizeof(int));
    sp->n = n;
    sp->front = sp->rear = 0;
    Sem_init(&sp->mutex, 0, 1);
    Sem_init(&sp->slots, 0, n);
    Sem_init(&sp->items, 0, 0);
}
void sbuf_deinit(sbuf_t *sp) {
    Free(sp->buf);
}

void sbuf_insert(sbuf_t *sp, int item) {
    P(&sp->slots);
    P(&sp->mutex);
    sp->buf[(++sp->rear) % (sp->n)] = item;
    V(&sp->mutex);
    V(&sp->items);
}

int sbuf_remove(sbuf_t *sp) {
    int item;
    P(&sp->items);
    P(&sp->mutex);
    item = sp->buf[(++sp->front) % (sp->n)];
    V(&sp->mutex);
    V(&sp->slots);
    return item;
}

/****************
 * cache helper
 ****************/
void cache_init() {
    for (int i = 0; i < CACHE_BLOCK_SIZE; i++) {
        cache.block[i].lru = 0;
        cache.block[i].is_empty = 1;
        cache.block[i].reader_cnt = 0;
        Sem_init(&cache.block[i].r_mutex, 0, 1);
        Sem_init(&cache.block[i].w_mutex, 0, 1);
    }
    Sem_init(&lru_mutex, 0, 1);
    global_lru = 1;
}

void cache_insert(char *obj, char *url) {
    int empty_idx = -1, min_lru = 1e9;
    for (int i = 0; i < CACHE_BLOCK_SIZE; i++) {
        if (cache.block[i].is_empty) empty_idx = i;
    }
    if (empty_idx == -1) {
        for (int i = 0; i < CACHE_BLOCK_SIZE; i++)
            if (cache.block[i].lru < min_lru) {
                min_lru = cache.block[i].lru;
                empty_idx = i;
            }
    }
    P(&cache.block[empty_idx].w_mutex);
    P(&lru_mutex);
    strcpy(cache.block[empty_idx].obj, obj);
    strcpy(cache.block[empty_idx].url, url);
    cache.block[empty_idx].lru = global_lru++;
    cache.block[empty_idx].is_empty = 0;
    V(&lru_mutex);
    V(&cache.block[empty_idx].w_mutex);
}

int cache_find(char *url) {
    for (int i = 0; i < CACHE_BLOCK_SIZE; i++) {
        if (cache.block[i].is_empty) continue;
        P(&cache.block[i].r_mutex);
        if (strcmp(cache.block[i].url, url) == 0) {
            cache.block[i].reader_cnt++;
            if (cache.block[i].reader_cnt == 1)
                P(&cache.block[i].w_mutex);
            V(&cache.block[i].r_mutex);
            return i;
        }
        V(&cache.block[i].r_mutex);
    }
    return -1;
}