# spool

##a simple epool client for multi file read&write in single thread

###loop mode example
---------------------------------------------
    
    
      int count;
      char buffer[512];
      void* poll = poll_create(1024);
      poll_add(poll,fd) < 0);
     
      while(1){
          if((poll_wait_event(poll,&ev,-1) == 0) && IS_EVENT_IN(ev)){
               if(ev.fd == fd){
                   count = read(fd,buffer,sizeof(buffer));
               }
            }
       }
       poll_close(poll);

###event mode example
------------------------------
    
    
    void on_data_in(int fd){
        char buffer[256];
        read(fd,buffer,sizeof(buffer));
    }
    poll_register_default(STDIN_FILENO,on_data_in,NULL,NULL);
    poll_loop_default();


