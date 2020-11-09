//服务器代码
//echoServer.c
 
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
 
#include <event2/event.h>
#include <event2/bufferevent.h>
 
const int LISTEN_PORT = 9999;
const int LISTEN_BACKLOG = 32;
 
void do_accept(evutil_socket_t listener, short event, void *arg);
void read_cb(struct bufferevent *bev, void *arg);
void error_cb(struct bufferevent *bev, short event, void *arg);
void write_cb(struct bufferevent *bev, void *arg);
 
int main(int argc, char *argv[])
{
    evutil_socket_t listener;
    struct sockaddr_in sin;
 
 
    listener = socket(AF_INET, SOCK_STREAM, 0);
    assert(listener > 0);
    evutil_make_listen_socket_reuseable(listener);
 
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(LISTEN_PORT);
 
    if(bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0)
    {
        perror("listen");
        return 1;
    }
     
    if (listen(listener, LISTEN_BACKLOG) < 0)
    {
        perror("listen");
        return 1;
  }
   
  printf ("Listening...\n");
     
    evutil_make_socket_nonblocking(listener);
    struct event_base *main_base = event_base_new();
    assert(main_base != NULL);
    struct event *listen_event = NULL;
    listen_event = event_new(main_base, listener, EV_READ|EV_PERSIST, do_accept, (void*)main_base);
    event_add(listen_event, NULL);
    event_base_dispatch(main_base);
 
    return 0;
}
 
void do_accept(evutil_socket_t listener, short event, void *arg)
{
    struct event_base *base = (struct event_base*)arg;
    evutil_socket_t connfd;
    struct sockaddr_in sin;
    socklen_t slen = sizeof(struct sockaddr_in);
    connfd = accept(listener, (struct sockaddr *)&sin, &slen);
    if(connfd < 0)
    {
        perror("accept");
        return;
    }
    if(connfd > FD_SETSIZE)
    {
        perror("connfd > FD_SETSIZE");
        return;
    }
 
    struct bufferevent *bev = bufferevent_socket_new(base, connfd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, read_cb, NULL, error_cb, arg);
    bufferevent_enable(bev, EV_READ|EV_WRITE|EV_PERSIST);
}
 
 
void read_cb(struct bufferevent *bev, void *arg)
{
    const int MAX_LINE = 255;
    char line[MAX_LINE + 1];
    int n;
    while(n = bufferevent_read(bev, line, MAX_LINE), n > 0)
    {
        line[n] = '\0';
        bufferevent_write(bev, line, n);
        printf("%s\n",line);
    }  
}
 
void write_cb(struct bufferevent *bev, void *arg){}
 
void error_cb(struct bufferevent *bev, short event, void *arg)
{
    bufferevent_free(bev);
}
