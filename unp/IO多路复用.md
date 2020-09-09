https://www.bilibili.com/video/BV1qJ411w7du?from=search&seid=17065485911481473383

#### 应用场景

- 我们要设计一个高性能的网络服务器，在有多个客户连接上来之后，要能够及时的知道到底哪个客户有数据发过来，服务器要read这些数据，并处理。
- 如果用**多线程**解决：则会有CPU上下文切换的开销，会有很大的时间损耗。
- 所以采用IO多路复用



#### 轮询

- 在用户空间写一个for循环，轮询访问每一个描述符，然后如果有数据则读取。

  ```c++
  while(1){
      for(fd in fd_set){
          if(fd has data){
              //处理
          }
      }
  }
  ```



#### select

```c++
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
```



```c++
int main()
{
  char buffer[MAXBUF];
  int fds[5];
  struct sockaddr_in addr;
  struct sockaddr_in client;
  int addrlen, n,i,max=0;;
  int sockfd, commfd;
  fd_set rset;
  for(i=0;i<5;i++)
  {
  	if(fork() == 0)
  	{
  		child_process();
  		exit(0);
  	}
  }
 
    //创建描述符，并加入到fds中
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(2000);
  addr.sin_addr.s_addr = INADDR_ANY;
  bind(sockfd,(struct sockaddr*)&addr ,sizeof(addr));
  listen (sockfd, 5); 
 
  for (i=0;i<5;i++) 
  {
    memset(&client, 0, sizeof (client));
    addrlen = sizeof(client);
    fds[i] = accept(sockfd,(struct sockaddr*)&client, &addrlen);
    if(fds[i] > max)
    	max = fds[i];
  }
  
    //调用select
  while(1){
	FD_ZERO(&rset);  //rset不可以被重用，所以需要每次调用select时，都不要重置rset
  	for (i = 0; i< 5; i++ ) {
  		FD_SET(fds[i],&rset);
  	}
 
   	puts("round again");
	select(max+1, &rset, NULL, NULL, NULL);
 
	for(i=0;i<5;i++) {
		if (FD_ISSET(fds[i], &rset)){
			memset(buffer,0,MAXBUF);
			read(fds[i], buffer, MAXBUF);
			puts(buffer);
		}
	}	
  }
  return 0;
}
```

- **rset**：是一个bitmap，表明那一个文件描述符被启用，被启用就设置成1（也就是把fds中的文件描述符对应的位设置成1）。默认大小是**1024**位

- 调用select时，系统会**把rset拷贝到内核**，在内核中进行轮询判断，这会比在用户空间轮询要快，**因为在用户空间中的每次查询，都需要陷入到内核态**。select是一个**阻塞函数**。

- 当有数据时：有数据的FD置位，select返回
- **max+1**：内核只会在0~max（而不是0~1023）这些位轮询。

- **总结**：将关心的文件描述符收集过来，并使用一个bitmap数组表示，传送给select，内核把bitmap数组拷贝到内核进行轮询。当没有数据时，select阻塞，当有一个或多个文件描述符有数据时，内核把相应的文件描述符置位，select返回。最后用户在遍历文件描述符，看看究竟哪个被置位了。
- **select的缺点：**
  - bitmap的默认大小是**1024**；
  - 每次都需要重新创建文件描述符的bitmap。
  - 用户态到内核态的切换需要开销
  - select返回之后，用户不知道具体哪一个文件描述符有数据，还是需要遍历一遍文件描述符



#### poll

![image-20200909114703437](.\pictures\poll.png)

- **pollfd**：
  - fd：文件描述符
  - events：用户关心的事件
  - revents：实际发生的事件

- **poll是阻塞函数。先把数组从用户态拷贝到内核中。然后当事件发生时，会把revents置位。（revents初始值为0）**
- **poll返回之后，用户还是需要使用一个for循环，判断究竟哪一个文件描述符被置位。记住：revents需要被设置为0。**所以可以重用这个数组
- **解决的问题**
  - 文件描述符上限不止1024大小。提高了上限，更加灵活
  - 文件描述符数组可以重用



#### epoll

![image-20200909115454191](.\pictures\epoll.png)

- epoll_create：创建一个“白板”（epoll events）
- epoll_ctl：可以控制“白板”，比如这里就是向白板中添加描述符和事件。（没有revent，只有用户关心的event）
- epfd这个东西**被用户和内核共享**。当描述符有数据到来的时候，内核会进行**重排**，把有数据的描述符放到开头，并返回有数据的描述符的个数（nfds）。





#### 总结

select
fd_set 使用数组实现
1.fd_size 有限制 1024 bitmap
fd【i】 = accept()
2.fdset不可重用，新的fd进来，重新创建
3.用户态和内核态拷贝产生开销
4.O(n)时间复杂度的轮询
成功调用返回结果大于 0，出错返回结果为 -1，超时返回结果为 0
具有超时时间

poll
基于结构体存储fd
struct pollfd{
int fd;
short events;
short revents; //可重用
}
解决了select的1,2两点缺点

epoll
解决select的1，2，3，4
不需要轮询，时间复杂度为O(1)
epoll_create 创建一个白板 存放fd_events
epoll_ctl 用于向内核注册新的描述符或者是改变某个文件描述符的状态。已注册的描述符在内核中会被维护在一棵红黑树上
epoll_wait 通过回调函数内核会将 I/O 准备好的描述符加入到一个链表中管理，进程调用 epoll_wait() 便可以得到事件完成的描述符

两种触发模式：
LT:水平触发
当 epoll_wait() 检测到描述符事件到达时，将此事件通知进程，进程可以不立即处理该事件，下次调用 epoll_wait() 会再次通知进程。是默认的一种模式，并且同时支持 Blocking 和 No-Blocking。
ET:边缘触发
和 LT 模式不同的是，通知之后进程必须立即处理事件。
下次再调用 epoll_wait() 时不会再得到事件到达的通知。很大程度上减少了 epoll 事件被重复触发的次数，
因此效率要比 LT 模式高。只支持 No-Blocking，以避免由于一个文件句柄的阻塞读/阻塞写操作把处理多个文件描述符的任务饿死。

