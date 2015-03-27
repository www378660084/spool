/*
 * poll.c
 *
 *  Created on: 2015-3-27
 *      Author: Administrator
 */


#include <linux/stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "spoll.h"

#define POLL_DEFAULT_SIZE 256


typedef struct poll_item{
    int fd;
    poll_cbk_t on_data_in;
    poll_cbk_t on_data_out;
    poll_cbk_t on_error;

    struct poll_item* next;
}poll_item_t;

typedef struct{
    int fd;
    int size;
    int count;
    int cur;
    poll_item_t* items;
    struct epoll_event events[0];
}poll_t;

#define EVENTS(ev) ev.events
#define FD(ev) ev.data.fd

static poll_t* default_poll = NULL;

void* poll_get_default(){
    if(default_poll == NULL){
        default_poll = poll_new(POLL_DEFAULT_SIZE);
    }
    return default_poll;
}

poll_item_t* poll_item_for_fd(void* poll,int fd){
    if(!poll)return NULL;
    poll_t* _poll = poll;
    poll_item_t* item = _poll->items;
    while(item){
        if(item->fd == fd)return item;
        item = item->next;
    }
    return NULL;
}

int poll_register_default(int fd,poll_cbk_t on_data_in,poll_cbk_t on_data_out,poll_cbk_t on_error){
    return poll_register(poll_get_default(), fd, on_data_in, on_data_out, on_error);
}

int poll_register(void* poll,int fd,poll_cbk_t on_data_in,poll_cbk_t on_data_out,poll_cbk_t on_error){
    poll_item_t* item = poll_item_for_fd(poll,fd);
    poll_t* _poll = poll;

    if(!item){
        item = malloc(sizeof(poll_item_t));
        if(!item)return POLL_MALLOC_ERR;
        memset(item,0,sizeof(poll_item_t));
        item->fd = fd;
        item->next = _poll->items;
        _poll->items = item;
    }
    if(on_data_in)item->on_data_in = on_data_in;
    if(on_data_out)item->on_data_out = on_data_out;
    if(on_error)item->on_error = on_error;

    int events = 0;

    if(item->on_data_in)events |= EPOLLIN;
    if(item->on_data_out)events |= EPOLLOUT;
    if(item->on_error)events |= (EPOLLHUP|EPOLLERR);

    poll_add_events(poll,fd,events);

    return 0;
}

void poll_loop_default(){
    poll_loop(poll_get_default());
}

void poll_run_once(void* poll,int timeout){
    if(!poll)return;
    poll_event_t ev;
    poll_item_t* item;

    if(!poll_wait_event(poll,&ev,timeout)){
        item = poll_item_for_fd(poll,ev.fd);
        if(!item)return;
        if(IS_EVENT_IN(ev) && item->on_data_in)item->on_data_in(ev.fd);
        else if(IS_EVENT_OUT(ev) && item->on_data_out)item->on_data_out(ev.fd);
        else if(IS_EVENT_ERR(ev) && item->on_error)item->on_error(ev.fd);
    }
}

void poll_loop(void* poll){
    while(1){
        poll_run_once(poll,POLL_FOREVER);
    }
}

void* poll_new(int size){
    poll_t* poll = malloc(sizeof(poll_t) + sizeof(struct epoll_event)*size);
    if(poll == NULL)return NULL;
    memset(poll,0,sizeof(poll_t) + sizeof(struct epoll_event)*size);
    poll->fd =  epoll_create(size);
    if(poll->fd < 0){
        free(poll);
        return NULL;
    }
    poll->size = size;
    return poll;
}

int poll_add_events(void* poll,int fd,int events){
    if(!poll)return POLL_NULL;
    poll_t* _poll = poll;

    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    return epoll_ctl (_poll->fd, EPOLL_CTL_ADD, fd, &ev);
}

int poll_add(void* poll,int fd){
    if(!poll)return POLL_NULL;
    return poll_add_events(poll,fd,EPOLLIN);
}

int poll_delete(void* poll){
    if(!poll)return POLL_NULL;

    poll_t* _poll = poll;
    close(_poll->fd);
    free(poll);

    return 0;
}

int poll_wait_event(void* poll,poll_event_t* ev,int timeout){
    poll_t* _poll = poll;
    if((!poll)||(!ev)) return POLL_NULL;

    if((_poll->count == 0)||(_poll->cur >= _poll->count)){
        _poll->cur = 0;
        _poll->count = epoll_wait (_poll->fd, _poll->events, _poll->size, timeout);
        if(_poll->count == 0)return POLL_TIMEOUT;
    }

    ev->events = _poll->events[_poll->cur].events;
    ev->fd = _poll->events[_poll->cur].data.fd;
    _poll->cur += 1;
    return 0;
}
