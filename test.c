#include <stdio.h>         //标准输入输出定义
#include <stdlib.h>        //标准函数库定义
#include <string.h>
#include <unistd.h>       //Unix标准函数定义
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>          //文件控制定义
#include <termios.h>     //POSIX中断控制定义
#include <errno.h>        //错误号定义
#include <pthread.h>
#include "spoll.h"

int fd;

void on_data_in(int _fd){
    char buffer[256];
    int count;

    if(_fd == fd){
        count = read(fd,buffer,sizeof(buffer));
        if(count > 0)write(STDOUT_FILENO,buffer,count);
    }else if(_fd == STDIN_FILENO){
        count = read(STDIN_FILENO,buffer,sizeof(buffer));
        if(count > 0)write(fd,buffer,count);
    }else{
        printf("unkown fd:%d\n",_fd);
    }
}

int main(int argc,char* argv[]) {
	if(argc < 2){
		printf("usage:\n\t%s tty\n",argv[0]);
		return -2;
	}else{
	  printf("open %s\n",argv[1]);
		fd = open(argv[1], O_RDWR|O_NOCTTY);
	}
	if (fd == -1) {
		printf("open serial %s error！\n",argv[1]);
		return -1;
	}
	struct termios opt;    

	if (tcgetattr(fd, &opt) != 0) {
		printf("tcgetattr fd");
		return -EXIT_FAILURE;
	}

	cfsetispeed(&opt, B115200);
	cfsetospeed(&opt,B115200);

	opt.c_cflag &= ~CSIZE;
	opt.c_cflag |= CS8;

	opt.c_cflag &= ~PARENB;       
	opt.c_iflag &= ~INPCK;        

	opt.c_cflag &= ~CSTOPB;
	opt.c_cflag |= (CLOCAL | CREAD);

	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	opt.c_oflag &= ~OPOST;
	opt.c_oflag &= ~(ONLCR | OCRNL);    

	opt.c_iflag &= ~(ICRNL | INLCR);
	opt.c_iflag &= ~(IXON | IXOFF | IXANY);    

	tcflush(fd, TCIFLUSH);
	opt.c_cc[VTIME] = 0;        
	opt.c_cc[VMIN] = 1;       
	printf("welcome to serial assistant\n");

	if(tcsetattr(fd, TCSANOW, &opt) != 0)
	{
		perror("tcsetattr fd");
		return -EXIT_FAILURE;
	}
	tcflush(fd, TCIFLUSH);

	poll_register_default(fd,on_data_in,NULL,NULL);
	poll_register_default(STDIN_FILENO,on_data_in,NULL,NULL);

	poll_loop_default();

	return EXIT_SUCCESS;
}

