## 1. 数据结构与对象

### 1.1 简单动态字符串

![image-20201003195405830](.\pictures\SDS.png)

#### 1.1.1 SDS与C字符串的区别

1. **常数复杂度获取字符串长度**
2. **杜绝缓冲区溢出**
   
- 当API需要对SDS进行修改时，API会先检查SDS的空间是否满足修改所需的要求，如果不满足的话，API会自动将SDS的空间扩展至执行修改需要的大小。
   
3. **减少修改字符串时带来的内存重分配次数**

   - 空间预分配
   - 惰性空间释放

4. **二进制安全**

5. **兼容部分C字符串函数**

   

### 1.2 链表



### 1.3 字典

```c++
//hashtable 定义
typedef struct dictEntry{
    void *key;
    union{
        void *val;
        uint64 _tu64;
        int64 _ts64;
    }v;
    
    struct dictEntry *next;
}dictEntry;

typedef struct dictht{
    //哈希表数组
	dictEntry **table; 
    //哈希表大小
	unsigned long size;
    //哈希表大小掩码，用于计算索引值，和计算出的哈希值一起决定一个键应该被放到table数组的哪个索引上
    // = size - 1
    unsigned long sizemask;
    //哈希表已有节点的数量
    unsigned long used;
}dictht;
```

![image-20201003210117702](.\pictures\hashtable.png)

```c++
//字典
typedef struct dict{
    dictType *type;
    void *privdata;
    //哈希表
    dictht ht[2];
    
    //rehash索引
    //当rehash没有在进行时，值为-1
    int rehashidx;
}
```

![image-20201003210404301](.\pictures\没有进行rehash状态下的字典.png)

- 使用**开链**解决冲突

  - 执行rehash的时机：

    - 服务器没有执行BGSAVE或者BGREWRITEAOF命令，且哈希表的负载因子大于等于1；

    - 服务器正在执行BGSAVE或者BGREWRITEAOF命令，且哈希表的负载因子大于等于5；

      > 负载因子 = ht[0].used / ht[0].size

    - 当负载因子小于0.1时，收缩哈希表

- **渐进式rehash**

  1. 为ht[1]分配空间，让字典同时持有ht[0]和ht[1]两个哈希表
  2. 维持一个rehashidx计数器，将它设置为0，表示rehash开始
  3. 在rehash期间，每次对字典进行添加、删除、查找或者更新操作时，程序会**顺带**将ht[0]中在rehashidx索引上的所有键值对rehash到ht[1]，rehash之后，将rehashidx加1
  4. ht[0]中的键全部rehash到ht[1]上之后，将rehashidx设置为-1，表示rehash完成。

     **Notes**：在rehash期间，**添加**新值时，会被之间添加到ht[1]中。



### 1.4 跳跃表

https://mp.weixin.qq.com/s/Ok0laJMn4_OzL-LxPTHawQ



### 1.5 整数集合

```
集合键的底层实现之一。当一个集合只包含整数值元素，并且这个集合的元素数量不多时，集合键就会使用整数集合作为底层实现。
```

![image-20201003232240231](.\pictures\整数集合.png)

- contents中的数值按照从小到大排序，并且不包含重复的项。

#### 1.5.1 升级

- 当我们要将一个新元素添加到整数集合里面，并且新元素的类型比整数集合现有所有元素类型都要长时，整数集合需要先进行升级，然后才能添加新元素到整数集合中。
- Notes：新元素总是添加在数组头部或者尾部。



### 1.6 压缩列表

- 不太会。。



### 1.7 对象

![image-20201004091953620](.\pictures\redis对象.png)

- 基于**引用计数**的内存回收机制
- Redis只对包含**整数值**的字符串对象进行共享（因为这种类型验证起来更加快速O(1)）

- lru属性：记录了对象最后一次被命令程序访问的时间。当服务器占用的内存数超过了maxmemory选项设置的上限时，空转时长较高的那部分会优先被服务器释放，从而回收内存。

  ```c++
  typedef struct redisObject{
      //类型
      unsigned type: 4;
      
      //编码
      unsigned encoding: 4;
      
      //指向底层结构的指针
      void *ptr;
      
      //引用计数
      int refcount;
      
      //
      unsigned lru: 22;
      
  }robj;
  ```



## 2 单机数据库的实现

### 2.1 数据库

```c++
typedef struct redisDb{
    //.....
    
    //数据库键空间，保存着数据库中的所有键值对
    dict *dict;
    
    //过期字典，保存着键的过期时间
    dict *expires;
}redisDB;
```

#### 2.1.1 过期时间

> **设置过期时间？**
>
> - expire 和 pexpire设置键的过期时间
>
> - setex命令可以在设置一个键的同时为键设置过期时间

> **判断一个键是否过期？**
>
> - 检查给定键是否存在于过期字典：如果存在，那么取得键的过期时间
> - 检查当前unix时间戳是否大于键的过期时间：如果是的话，那么键已经过期，否则没有过期。

#### 2.1.2 过期键的删除策略

- 使用**惰性删除**和**定期删除**两种策略。

- **惰性删除**

  ```
  所有读写数据库的Redis命令在执行之前都会调用expireIfNeeded函数对输入键进行检查：
  	1. 如果键已经过期，那么expiredIfNeeded函数将键从数据库中删除；
  	2. 如果键未过期，那么expireIfNeeded函数不做动作。
  expireIfNeeded函数就像一个过滤器，在命令真正执行之前，过滤掉过期的键，从而避免命令接触到过期键。
  ```

  - 缺点：对内存不友好：大量过期键得不到释放，可能长时间占用内存空间
  - 优点：对CPU友好。

- **定期删除**

  ```
  由activeExpireCycle函数实现，当服务器周期函数serverCron函数被调用时，该函数就会被调用
  
  在规定的时间内，分多次遍历服务器中的各个数据库，从expires字典中随机检查一部分键的过期时间，并删除其中的过期键。
  ```

```
Notes:
1. 在使用SAVE和BGSAVE生成RDB文件时，已经过期的键不会出现在RDB中；
2. 如果服务器以主服务器模式运行，那么在载人RDB文件时，程序会对文件中保存的键进行检查，未过期的键会被载人到数据库中，而过期键则会被忽略，所以过期键对载人RDB文件的主服务器不会造成影响。
3. 如果服务器以从服务器模式运行，那么在载入RDB文件时，文件中保存的所有键，不论是否过期，都会被载人到数据库中。不过，因为主从服务器在进行数据同步的时候，从服务器的数据库就会被清空，所以一般来讲，过期键对载人RDB文件的从服务器也不会造成影响。

4. 当过期键被惰性删除或者被定期删除后，程序会向AOF文件追加一条DEL命令，来显示的删除该键。
5. 在执行AOF重写时，已过期的键不会保存到重写后的AOF文件中。

6. 当服务器运行在复制模式下时，从服务器的过期键删除动作由主服务器控制:主服务器在删除一个过期键之后，会显式地向所有从服务器发送一个DEL命令，告知从服务器删除这个过期键。从服务器在执行客户端发送的读命令时，即使碰到过期键也不会将过期键删除，而是继续像处理未过期的键一样来处理过期键。从服务器只有在接到主服务器发来的DEL命令之后，才会删除过期键。
```

![image-20201004101855058](.\pictures\主从服务器删除过期的键1.png)

![image-20201004101920482](.\pictures\主从服务器删除过期的键2.png)

### 2.2 RDB持久化

- **保存数据库中的键值对**来记录数据库状态。

- RDB文件是经过**压缩的二进制文件**
- **SAVE**命令会阻塞Redis服务器进程，直到RDB文件创建完毕，此期间，服务器无法处理任何其他命令；**BGSAVE**派生出一个子进程

- AOF文件的更新频率比RDB文件高，所以如果检测到AOF文件，数据库会优先使用AOF文件还原状态。

- **RDB原理**

  ```c++
  struct redisServer{
  	//...
  	//记录保存条件的数组
  	struct saveparam *saveparams;
      
      //修改计数器
      long long dirty;
      
      //上一次执行保存的时间
      time_t lastsave;
  };
  
  struct saveparam{
      //秒数（时间）
      time_t seconds;
      //修改次数
      int changes;
  };
  ```

  ```python
  def serverCron():
      # 遍历所有条件
      for saveparam in server.saveparams:
          save_interval = unixtime_now() - server.lastsave
          
          # 时间和修改次数均满足条件，则执行BGSAVE
          if server.dirty >= saveparam.changes and 
          	save_interval > saveparam.seconds:
                  BGSAVE()
  ```

  ![image-20201004103626567](.\pictures\RDB文件格式.png)

### 2.3 AOF持久化

- 保存Redis**所执行的写命令**来记录数据库的状态。
- AOF是**纯文本**文件

#### 2.3.1 AOF持久化的实现

1. 命令追加：服务器在执行完一条**写**命令之后，会把该写命令追加到aof_buf缓冲区末尾

   ```c++
   struct redisServer{
   	//AOF缓冲区
   	sds aof_buf;
   };
   ```

2. 服务器进程在每次**事件循环**结束时，考虑是否把aof_buf中的内容同步到AOF文件中

   ![image-20201004105403180](.\pictures\appendfsync.png)

   ```
   Notes:
   文件写入与文件同步
   	为了提高文件的写入效率，在现代操作系统中，当用户调用write函数，将一些数据写入到文件的时候，os通常会将写入数据暂时保存在一个内存缓冲区里面（例如，unix系统实现在内核中设有缓冲区高速缓存或页高速缓存，当我们向文件写入数据时，内核通常先将数据复制到缓冲区中，然后排入队列，晚些时候再写入磁盘），这种方式称为延迟写，等到缓冲区的空间被填满，或者超过了指定的时限，或者内核需要重用缓冲区存放其它磁盘块数据时，才会真正将缓冲区中的所有数据写入到磁盘里面。
   
   	这种做法虽然提高了效率，但也为写入数据带来了安全问题，如果计算机停机，则保存在缓冲区中的写入数据将丢失。为了保持一致性，即向文件写入数据立即真正的写入到磁盘上的文件中，而不是先写到内存缓冲区里面，则我们需要采取文件同步。fsync和fdatasync函数
   ```

   

3. **AOF载入与数据还原**

   ![image-20201004105543644](.\pictures\AOF文件载入与还原.png)

#### 2.3.2 AOF重写

- BGREWRITEAOF

- **读取服务器当前的数据库状态**来实现AOF重写

- 使用子进程（因为进程比线程更加安全可靠，保证数据的安全性），完成AOF重写工作。

- 在子进程生成AOF重写文件期间，写命令同时写入**AOF重写缓冲区**

  ![image-20201004111002361](.\pictures\AOF重写缓冲区.png)

  - 当子进程完成AOF重写工作之后，它会向父进程发送一个**信号**，父进程在接到该信号之后，会调用一个信号处理函数，并执行以下工作:
    1. 将AOF重写缓冲区中的所有内容写人到新AOF文件中，这时新AOF文件所保存的
       数据库状态将和服务器当前的数据库状态一致。
    2. 对新的AOF文件进行改名，**原子地**( atomic）覆盖现有的AOF文件，完成新旧两
       个AOF文件的替换。
  - **Notes**：父进程接收到子进程的信号之后，会阻塞。（相当于阻塞在了信号处理函数处）

> **RDB和AOF**
>
> **RDB**
>
> - RDB文件紧凑，全量备份，非常适合用于进行备份和灾难恢复。
>
> - RDB 在恢复大数据集时的速度比 AOF 的恢复速度要快。
>
> - RDB快照是一次全量备份，当进行快照持久化时，会开启一个**子进程**专门负责快照持久化，子进程会拥有父进程的内存数据，父进程修改内存子进程不会反应出来（写时复制），所以**在快照持久化期间修改的数据不会被保存，可能丢失数据**。
>
> **AOF**
>
> - AOF可以更好的保护数据不丢失，一般AOF会每隔1秒，通过一个后台线程执行一次fsync操作，最多丢失1秒钟的数据。
> - AOF日志文件没有任何磁盘寻址的开销，写入性能非常高，文件不容易破损。
> - AOF以文件追加的方式写入和同步AOF文件，所以不会占用过大的磁盘带宽。
>
> - AOF日志文件的命令通过纯文本追加的方式进行记录，这个特性非常适合做**灾难性的误删除的紧急恢复**。比如某人不小心用flushall命令清空了所有数据，只要这个时候后台rewrite还没有发生，那么就可以立即拷贝AOF文件，将最后一条flushall命令给删了，然后再将该AOF文件放回去，就可以通过恢复机制，自动恢复所有数据
>
> - 对于同一份数据来说，AOF日志文件通常比RDB数据快照文件更大
> - 总的来说：AOF实时性更好，数据保存的更加完全。

| 命令       | RDB              | AOF                |
| ---------- | ---------------- | ------------------ |
| 启动优先级 | 低               | 高                 |
| 体积       | 小               | 大                 |
| 恢复速度   | 快               | 慢                 |
| 数据安全性 | 可能丢失较多数据 | 能保证数据的完备性 |
| 轻重       | 重（完全备份）   | 轻（增量备份）     |



### 2.4 客户端

```c++
服务器用链表保存所有的客户端
struct redisServer{
	// ...
	
	//用链表保存所有的客户端状态
	list *clients;
};
```

![image-20201004111757384](.\pictures\客户端.png)

- 伪客户端的fd属性为-1

![image-20201004112330538](.\pictures\redisclient.png)



## 3 多机数据库的实现

### 3.1 复制

- SLAVEOF

#### 3.1.1 旧版本复制功能

- **同步**
  1. 从服务器向主服务器发送SYNC命令
  2. 收到SYNC命令的主服务器执行BGSAVE命令，在后台生成一个RDB文件，**并使用一个缓冲区记录从现在开始执行的所有写命令**。
  3. 当主服务器的BGSAVE命令执行完毕时，主服务器会将BGSAVE命令生成的RDB
     文件发送给从服务器，从服务器接收并载人这个RDB文件，将自己的数据库状态更新至主
     服务器执行BGSAVE命令时的数据库状态。
  4. 主服务器将记录在缓冲区里面的所有写命令发送给从服务器，从服务器执行这些写
     命令，将自己的数据库状态更新至主服务器数据库当前所处的状态。

![同步](.\pictures\同步.png)

- **命令传播**
  - 主服务器将自己执行的**写命令**，也即造成主从服务器不一致的命令，发送给从服务器执行。

> 缺陷：断线后重复制代价太大（同初次复制一样，没有进行区分）

#### 3.1.2 新版复制功能的实现

- **PSYNC**
  - **完整重同步**：同旧版本一样，为了应对初次复制的情况
  - **部分重同步**：为了应对断线后重复制的情况

![image-20201004115447758](.\pictures\psync.png)

- **部分重同步的实现**

  - **复制偏移量**

    ![image-20201004115835407](.\pictures\复制偏移量.png)

  - **复制积压缓冲区**：**固定长度**的**先进先出**的队列

  - **服务器运行ID**：因为主服务器可能崩溃了，被哨兵替换过。

![image-20201004115918049](.\pictures\复制积压缓冲区.png)

>​	如果offset偏移量之后的数据（也即是偏移量offset+1开始的数据）仍然存在于复制积压缓冲区里面，那么主服务器将对从服务器执行**部分重同步**操作。
>
>​	相反，如果offset偏移量之后的数据已经不存在于复制积压缓冲区，那么主服务器将对从服务器执行**完整重同步**操作。

![image-20201004120307562](.\pictures\PSYNC的实现.png)



### 3.2 Sentinel



### 3.3 集群



## 4 独立功能的实现

### 4.1 发布与订阅

- PUBLISH、SUBSCRIBE、PSUBSCRIBE

- 客户端可以**订阅**一个或多个频道（或者模式），从而成为这些频道的订阅者：当有其他客户端向被订阅的频道发送消息时，频道的所有订阅者都会受到这条消息。

  ![image-20201004131618024](.\pictures\订阅与发布.png)

#### 4.1.1 频道

```c++
struct redisServer{
    // ...
    
    //保存所有频道的订阅关系
    //key - 某个被订阅的频道
    //value - 链表，记录所有订阅这个频道的客户端。
    dict *pubsub_channels;
};
```

![image-20201004132158032](.\pictures\subscribe.png)

#### 4.1.2 模式

```c++
struct redisServer{
    //...
    
    //保存所有模式订阅关系
    list *pubsub_patterns;
};

typedef struct pubsubPattern{
    //订阅的客户端
    redisClient *client;
    //被订阅的模式
    robj *pattern;
}pubsubPattern;
```

![image-20201004132519258](.\pictures\模式的订阅.png)



### 4.2 事务

- MULTI、EXEC、WATCH

  > 事务提供了一种将**多个命令请求打包**，然后一次性、按顺序地执行多个命令的机制，并且在事务执行期间，服务器不会中断事务而该去执行其他客户端的命令请求，（**串行化**）它会将事务中所有命令都执行完毕，然后才去处理其他客户端的命令请求。

- 事务从开始到结束的三个阶段
  - 事务开始
  - 命令入队
  - 事务执行

- **事务队列**

  ```c++
  typedef struct redisClient{
      //事务状态
      multiState mstate;
  }redisClient;
  
  typedef struct multiState{
      //事务队列
      multiCmd *commands;
      //已入队命令计数
      int count;
  }multiState;
  ```

  ![image-20201004133509895](.\pictures\事务状态.png)

- **WATCH命令**

  - **乐观锁**
  - 在执行EXEC命令之前，**监视**任意数量的数据库键，在调用EXEC时，检查被监视的键是否至少有一个已经被修改了，如果是，服务器拒绝执行此事务。

  ```c++
  typedef struct redisDb{
  	// ...
  	
  	//正在被WATCH命令监视的键
      //key - 被监视的键
      //value - 链表，客户端
  	dict *watched_keys;
  }redisDb;
  ```

  ![image-20201004134114542](.\pictures\watch.png)

- **ACID**
  - **Redis事务具有原子性**。
    - **入队错误**：当命令本身语法有问题时，命令入队出错而被拒绝，事务中的所有命令都不会被执行。
    - **执行错误**：当命令在运行时出错时，Redis不支持回滚，整个事务还是会运行下去，直到事务中的所有命令都被执行完毕。
  - **Redis事务具有一致性**
  - **Redis具有隔离性，因为单进程。**
  - **Redis只有工作在AOF持久化下，且always同步AOF文件时，才具有持久性**。在一个事务最后加上SAVE（生成RDB文件）总是可以保持事物的持久性。