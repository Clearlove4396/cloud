#### http/1.0

- 短连接：每次发送数据都需要先建立TCP连接，进行完一次“请求/响应”这样的过程过程之后，就断开连接。



#### http/1.1

> 相比于http/1.0的改进

- 使用TCP**长连接**的方式改善了Http/1.0短连接造成的开销。
  - 每次客户端和服务端进行通信的时候建立TCP连接，直到某一方要求断开TCP连接，才进行TCP4次挥手过程。
- 支持**管道**进行网络传输
  - 只要第一个请求发出去之后，不必等其回来，就可以发第二个请求出去，可以减少整体的响应时间。但是服务器响应的时候，需要按照请求的顺序进行响应。

![image-20200912152833278](.\markdown-picture\管道.png)

> http/1.1存在的问题

- **没有头部压缩**。发送相同的首部造成浪费较多
- 服务器都是按请求的顺序响应的，如果服务器响应慢，就会发生**队头阻塞**。
- **没有请求优先级**控制
- **不支持服务器推送**。请求只能从客户端开始，服务器只能被动响应



#### http/2.0

- http/2.0是建立在**TLS层**之上的，所以保证了发送数据的**安全性**。

![image-20200912153426049](.\markdown-picture\http2.0.png)

- **头部压缩（HPACK算法）**
  - 通信方法都维护了一张索引表，用于记录已经出现过的header，后面在传输的过程中，就可以传输索引表的key，而不需要传输完整的header。
- **二进制格式：帧**
  - http/2.0不在使用纯文本形式的报文，而是采用**二进制格式**。头信息和数据体都是二进制格式，称为帧：**头信息帧**和**数据帧**。这增加了传输的效率，因为少了从纯文本到二进制格式的转化，接收方可以直接处理二进制格式的报文。

- **流**
  - 帧是最小的数据单位，每个帧会标识出该帧属于哪个流，流是多个帧组成的数据流
  - 客户端发出的数据流编号为**奇数**，服务器发出的数据流编号为**偶数**。
- **多路复用**
  - 因为有了“流”和“帧”的概念，所以让多路复用成为可能。
  - 在http/1.1中，增加了“长连接”特性，但是仍然存在“队头阻塞”。“队头阻塞”的关键是服务器无法并发的处理请求，因为“请求--响应”是必须完全对应的。这导致了：**客户端在需要发起多次请求的时候，会采用建立多个连接来较少因为队头阻塞带来的延迟**。
  - **帧是最小的数据单位，每个帧会标识出该帧属于哪个流，流是多个帧组成的数据流**。所谓“多路复用”，即在一个TCP连接中存在多个流，即可以同时发送多个请求，对端可以通过帧中的表示知道这个帧属于哪个流（请求）。
  - 在客户端，这些帧可以乱序发送（不同流中的帧可以乱序发送，同一个流中的帧还是需要顺序发送，否则可能引发**TCP的队头阻塞**），到对端后在根据每个帧首部的流标识符重新组装。通过这个技术，服务端可以**摆脱“请求--响应”的顺序限制**，**并发地**处理请求，避免（http）头部阻塞问题。



#### http/3.0

>  http/2.0多路复用的问题？
>
> - **队头阻塞有两种情况：**
>   - **http队头阻塞**：http/2.0可以处理，因为http/2.0摆脱了“请求--响应”的顺序限制。
>   - **TCP队头阻塞**：http/2.0无法处理，因为这是TCP本身的局限性
>     - TCP协议在收到数据包之后，这部分数据可能是乱序到达的，但是TCP必须将所有数据收集排序之后，才会向上层发送。**如果某个流中的某个包丢失，就会触发TCP重传机制，所有的流必须等待这个丢失的包重传成功，从而阻塞整个连接的使用**。

- **htt/3.0拓扑图**

  ![image-20200912155253087](.\markdown-picture\http3.0.png)

- **QUIC**
  
  - QUIC是基于**UDP**实现的，**在一条链路上可以有多个流，流与流之间是互不影响的**，当一个流出现丢包时，影响范围非常小。
  - **每个流有独立的拥塞控制，避免单个流中的丢包阻塞其它所有流**。
  - 内建安全性，集成TLS
  - 连接建立过程和TLS协商过程合并，减少往返请求次数，提高连接速度
  - QUIC协议可以实现在**第一个包就可以含有有效的应用数据**。第一个包包含：加密协商，应用数据。
    - 首次连接：客户端和服务端的**密钥协商**和**数据传输**过程。（密钥协商使用DH密钥交换算法）
    - 非首次连接

![image-20200912161600553](.\markdown-picture\QUIC连接过程.png)

