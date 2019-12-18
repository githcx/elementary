#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;

#define MYPORT 1234 //连接时使用的端口

#define MAXCLINE 5 //连接队列中的个数

#define BUF_SIZE 200

int main(void)
{
  int sock_fd = -1; 
  char buf[BUF_SIZE];
  //建立sock_fd套接字
  if((sock_fd = open("./chan", O_RDONLY | O_NONBLOCK))==-1)
  {
    perror("setsockopt");
    return 1;
  }

  fd_set fdsr; //文件描述符集的定义
  struct timeval tv;
  tv.tv_sec = 30;
  tv.tv_usec =0;

  int total_bytes = 0;
  
  while(1)
  {
    //初始化文件描述符集合
    //超时的设定
    //如果文件描述符中有连接请求 会做相应的处理，实现I/O的复用 多用户的连接通讯
    FD_ZERO(&fdsr); //清除描述符集
    FD_SET(sock_fd,&fdsr); //把sock_fd加入描述符集
    int ret = select(sock_fd +1,&fdsr,NULL,NULL,&tv);
    if(ret < 0) //没有找到有效的连接 失败
    {
      perror("select error!\n");
      break;
    }

    int total_read = 0;
    //循环判断有效的连接是否有数据到达
    for(int i=0;i<ret;i++)
    {
      if(FD_ISSET(sock_fd,&fdsr))
      {
        again:
        ret = read(sock_fd, buf,sizeof(buf));
        total_read += 1;
        if(ret == 0) //客户端连接关闭，清除文件描述符集中的相应的位
        {
          close(sock_fd);
          sock_fd = -1;
          cout << "EOF received" << endl;
          cout << "total bytes = " << total_bytes << endl;
          return 0;
        }

        if( ret == -1 )
        {
          if( errno == EAGAIN)
          {
            cout << "read " << total_read << " times for this select" << endl;
            continue;
          }
            
          perror("exception");
          return -1;
        }
        //否则有相应的数据发送过来 ，进行相应的处理
        else
        {
          //for(int j = 0; j < ret; j++) putchar(buf[j]);
          //putchar('\n');
          total_bytes += ret;
          goto again;
        }
      }
    }
  }

  return 0;
}

