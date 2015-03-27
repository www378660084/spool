/*
 * poll.h
 *
 *  Created on: 2015-3-27
 *      Author: Administrator
 */

#ifndef SPOLL_H_
#define SPOLL_H_
#include <sys/epoll.h>

/***********************loop mode example ************
 * int count;
 * char buffer[512];
 * void* poll = poll_create(1024);
 * poll_add(poll,fd) < 0);
 *
 * while(1){
 *     if((poll_wait_event(poll,&ev,-1) == 0) && IS_EVENT_IN(ev)){
 *          if(ev.fd == fd){
 *              count = read(fd,buffer,sizeof(buffer));
 *          }
 *       }
 *  }
 *  poll_close(poll);
 ********************end example*********************/

#define POLL_FOREVER (-1)

#define POLL_NULL   (-20)
#define POLL_MALLOC_ERR (-21)
#define POLL_TIMEOUT (-22)
#define POLL_OVER_SIZE (-23)

#define IS_EVENT_IN(ev) ((ev).events&EPOLLIN)
#define IS_EVENT_OUT(ev) ((ev).events&EPOLLOUT)

#define IS_EVENT_ERR(ev) ((ev).events&EPOLLERR)
#define IS_EVENT_HUP(ev) ((ev).events&EPOLLHUP)

typedef struct{
    int fd;
    int events;
}poll_event_t;

void* poll_new(int size);

int poll_add(void* poll,int fd);
int poll_add_events(void* poll,int fd,int events);

int poll_delete(void* poll);
/**
 *  wait for an event
 *  poll:
 *  ev:
 *  timeout:ms  >0 or POLL_FOREVER
 * */
int poll_wait_event(void* poll,poll_event_t* ev,int timeout);



typedef void (*poll_cbk_t)(int fd);
void* poll_get_default();
int poll_register(void* poll,int fd,poll_cbk_t on_data_in,poll_cbk_t on_data_out,poll_cbk_t on_error);
int poll_register_default(int fd,poll_cbk_t on_data_in,poll_cbk_t on_data_out,poll_cbk_t on_error);
void poll_loop(void* poll);
void poll_run_once(void* poll,int timeout);
void poll_loop_default();

/***********************event mode example***********************************
//void on_data_in(int fd){
//    char buffer[256];
//    read(fd,buffer,sizeof(buffer));
//}
//poll_register_default(STDIN_FILENO,on_data_in,NULL,NULL);
//poll_loop_default();
**********************end example***********************************/

#endif /* SPOLL_H_ */
