#include "csapp.h"

void *do_http(void *vargp);
void do_get(int fd, int is_static, const char *filenaem, const char *cgiargs);
void serve_static(int fd, const char *filename, int filesize);
void serve_dynamic(int fd, const char *filename, const char *cgiargs);

int parse_url(const char *url, char *filename, char *cgiargs);
void print_request_headers(rio_t *rptr);
void clienterror(int fd, const char *cause, const char *code, const char *shortmsg, const char* longmsg);
void get_filetype(const char *filename, char *filetype);
void reap_handler(int sig);

char ROOT_DIR[MAXLINE];
char DEFAULT_PAGE[MAXLINE];

int main(int argc, char **argv)
{
    if (argc != 4) {
        //没有指定端口
        fprintf(stderr, "usage: %s <port> <ROOT_DIR> <DEFAULT_PAGE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    strcpy(ROOT_DIR, argv[2]);
    strcpy(DEFAULT_PAGE, argv[3]);

    if (signal(SIGCHLD, reap_handler) == SIG_ERR)
        unix_error("signal error");
    int listenfd = Open_listenfd(argv[1]);
    int *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char hostname[MAXLINE], port[MAXLINE];
    pthread_t tid;
    printf("TinyWebServer is working!\n");

    while (1) {
        printf("----------Begin-----------\n");
        clientlen = sizeof(clientaddr);
        connfdp = Malloc(sizeof(int)); // 这里一定要使用malloc在堆上分配内存, 避免race 冒险
        *connfdp = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        Getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE, 
                port, MAXLINE, NI_NUMERICHOST | NI_NUMERICSERV);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        Pthread_create(&tid, NULL, do_http, connfdp);
        printf("-----------End------------\n");
    }
}
/*
 * 线程例程 thread routine
 * 处理HTTP事务
 * 参数: 指向建立连接的文件描述符的指针
 * 注意: vargp指向的内存区域是malloc的, 切记要free, 连接描述符也要记得关闭
 */
void* do_http(void *vargp)
{
    int cfd = *((int*)vargp);
    Free(vargp); // 注意
    Pthread_detach(pthread_self());
    rio_t rio; //RIO包的缓冲区
    Rio_readinitb(&rio, cfd);
    char buff[MAXLINE];
    if (Rio_readlineb(&rio, buff, MAXLINE) == 0) return NULL;
    printf("%s", buff); //打印请求行
    char method[MAXLINE], url[MAXLINE], version[MAXLINE];
    sscanf(buff, "%s %s %s", method, url, version);
    char filename[MAXLINE], cgiargs[MAXLINE];
    int is_static = parse_url(url, filename, cgiargs);
    print_request_headers(&rio);
    if (strcasecmp(method, "GET") == 0)
        do_get(cfd, is_static, filename, cgiargs);
    else clienterror(cfd, method, "501", 
            "Not Implemented",
            "TinyWebServer does not implement this method!");
    Close(cfd);
    return NULL;
}

/*
 * 解析请求的url
 * 输入: url
 * 输出: filename 请求的文件
 *       cgiargs 请求的参数
 * 返回: 是否为静态服务, 请求/cgi-bin/下的CGI服务为非静态服务
 */
int parse_url(const char* url, char* filename, char* cgiargs)
{
    strcpy(filename, ROOT_DIR);
    if (strstr(url, "/cgi-bin") == url) {
        // 动态服务
        char *p = index(url, '?');
        if (p) {
            //有参数
            strcpy(cgiargs, p+1);
        } else strcpy(cgiargs, "");
        strncat(filename, url, strlen(url) - strlen(p));
        return 0;
    }
    // 静态服务
    strcpy(cgiargs, "");
    strncat(filename, url, strlen(url));
    if (url[strlen(url)-1] == '/') strcat(filename, DEFAULT_PAGE);
    return 1;
}

/*
 * 打印请求头
 */
void print_request_headers(rio_t *rptr)
{
    char buff[MAXLINE];

    Rio_readlineb(rptr, buff, MAXLINE);
    printf("%s", buff);
    while (strcmp(buff, "\r\n")) {
        Rio_readlineb(rptr, buff, MAXLINE);
        printf("%s", buff);
    }
}

/*
 * 实现GET请求
 */
void do_get(int fd, int is_static, const char *filename, const char *cgiargs)
{
    struct stat sbuf;
    if (stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not found",
                "TinyWebServer couldn't find this file");
        return;
    }

    if (is_static) {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "TinyWebServer couldn't read this file!");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
        return;
    }

    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
        clienterror(fd, filename, "403", "Forbidden", "TinyWebServer couldn't run the CGI program!");
        return;
    }
    serve_dynamic(fd, filename, cgiargs);
}

/*
 * 获取请求文件的类型
 */
void get_filetype(const char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else 
        strcpy(filetype, "text/plain");
}

/*
 * 静态服务
 */
void serve_static(int fd, const char *filename, int filesize)
{
    int srcfd;
    char *srcp;
    char filetype[MAXLINE], buff[MAXBUF];
    get_filetype(filename, filetype);
    sprintf(buff, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buff, strlen(buff));
    sprintf(buff, "Server: TinyWebServer\r\n");
    Rio_writen(fd, buff, strlen(buff));
    sprintf(buff, "Content-length: %d\r\n", filesize);
    Rio_writen(fd, buff, strlen(buff));
    sprintf(buff, "Content-type: %s\r\n\r\n", filetype);
    Rio_writen(fd, buff, strlen(buff));
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
}

/*
 * 动态服务 CGI
 */
void serve_dynamic(int fd, const char *filename, const char *cgiargs)
{
    char buff[MAXLINE];
    sprintf(buff, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buff, strlen(buff));
    sprintf(buff, "Server: TinyWebServer\r\n");
    Rio_writen(fd, buff, strlen(buff));

    if (Fork() == 0) {
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO);
        char *emptylist[] = {NULL};
        Execve(filename, emptylist, environ);
    }
}

/*
 * 回收子进程
 */
void reap_handler(int sig)
{
    int olderrno = errno; //保存旧的的errno
    while (waitpid(-1, NULL, 0) > 0) {
        Sio_puts("reaped a chiled!\n");
    }
    if (errno != ECHILD)
        sio_error("waitpid error!\n");
    errno = olderrno;
}

/*
 * 处理错误
 */
void clienterror(int fd, const char *cause, const char *errnum, \
        const char *shortmsg, const char *longmsg) 
{
    char buf[MAXLINE];
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
