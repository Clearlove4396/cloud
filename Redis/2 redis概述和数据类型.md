### Redis入门

- **Redis**

  > （Remote Dictionary Server )，即远程字典服务，是一个开源的使用ANSI C语言编写、支持网络、可基于**内存**亦**可持久化**的日志型、**Key-Value**数据库，并提供多种语言的API。从2010年3月15日起，Redis的开发工作由VMware主持。Redis可以用作**数据库**、**缓存**和**消息中间件（MQ）**。
  >
  > redis会周期性的把更新的数据写入磁盘或者把修改操作写入追加的记录文件，并且在此基础上实现了master-slave(主从)同步

- **Redis能干嘛？**

  > 1. 内存存储，持久化，内存中是断电即失的，所以说持久化很重要（RDB, AOF)
  > 2. 效率高，可以用于高速缓存
  > 3. 发布订阅系统
  > 4. 地图信息分析
  > 5. 计时器、计数器（微博、微信浏览量等）

- **Redis特性**

  >1. 多样的数据类型
  >2. 持久化
  >3. 集群
  >4. 事务



### Redis的简单使用

![image-20200910092821477](.\pictures\redis的启动.png)



![image-20200910093401405](.\pictures\redis的关闭和查看进程.png)



### Redis性能测试工具——benchmark

- 暂时略过



### 基本的redis知识

> 1. 默认有16个数据库，默认使用第0个，可以使用select切换默认数据库

```bash
set name key  //向当前数据库插入kv对
get name     //获取key = name的value
select 3     //切换到第4个数据库
dbsize       //查看数据库大小
keys *       //查看数据库所有的key
flushdb      //清空当前数据库
flushall      //清空所有数据库
exist name   //查看name这个key是否存在，存在返回1，否则返回0
move name 1  //移除key等于name的这个kv对，1表示操作的是第二个数据库
expire name 10  //设置name的过期时间，10s
ttl name     //查看name还有多长时间过期。如果name永久有效，则返回-1
type name    //查看key的类型
```

> 2. redis的默认端口号是**6379**

> 3. Redis是**单线程**的！原因：Redis是基于内存操作，CPU不是Redis性能瓶颈，Redis的瓶颈是根据机器的内存和网络带宽

> 4. Redis为什么单线程还这么快？
>    - redis将所有的数据放在内存中，所以使用单线程去操作效率就是最高的（因为多线程会产生CPU上下文切换，这很耗时），对于内存系统来说，如果没有上下文切换效率就是最高的。多次读写都是在一个CPU上的。
>    - Redis是C语言编写的。



### Redis数据类型

---

- **基本类型**
  - string
  - list
  - set
  - hash
  - zset
- **其他类型**
  - Geospatial
  - Hyperloglog
  - Bitmap

#### String（字符串）

```bash
127.0.0.1:6379> set key1 v1
OK
127.0.0.1:6379> get key1
"v1"
127.0.0.1:6379> exists key1 
(integer) 1
127.0.0.1:6379> append key1 "hello"    #追加，如果当前key1不存在，相当于set key
(integer) 7
127.0.0.1:6379> get key1
"v1hello"
127.0.0.1:6379> strlen key1   #获得长度
(integer) 7
##############################################
127.0.0.1:6379> set views 0
OK
127.0.0.1:6379> get views
"0"
127.0.0.1:6379> incr views   #自增1
(integer) 1
127.0.0.1:6379> incr views
(integer) 2
127.0.0.1:6379> get views
"2"
127.0.0.1:6379> decr views   #--
(integer) 1
127.0.0.1:6379> get views
"1"
127.0.0.1:6379> incrby views 10   #+10
(integer) 11
127.0.0.1:6379> decrby views 5    #-5
(integer) 6
127.0.0.1:6379> get views
"6"

####################################################
# 截取某个范围的字符串
127.0.0.1:6379> set key1 "hello, clearlove"
OK
127.0.0.1:6379> getrange key1 0 3   #下标0~3
"hell"
127.0.0.1:6379> getrange key1 0 -1  #
"hello, clearlove"

####################################################
# 替换指定位置开始的字符串
127.0.0.1:6379> set key2 abcdefg
OK
127.0.0.1:6379> setrange key2 1 xx  # 下标
(integer) 7
127.0.0.1:6379> get key2
"axxdefg"

#####################################################
# setex(set with expire)   # 设置过期时间
# setnx(set if not exist)  # 如果不存在会干嘛

127.0.0.1:6379> setex key3 30 "hello"   # 30秒过期
OK
127.0.0.1:6379> ttl key3
(integer) 26
127.0.0.1:6379> setnx mykey redis
(integer) 1
127.0.0.1:6379> keys *
1) "mykey"
2) "key1"
3) "key2"
127.0.0.1:6379> setnx mykey mongodb   # 创建失败，因为mykey存在
(integer) 0
127.0.0.1:6379> ttl key3   #-2表示此值已经过期
(integer) -2

#####################################################
# mget批量获取
# mset批量设置值
127.0.0.1:6379> mset k1 v1 k2 v2 k3 v3
OK
127.0.0.1:6379> keys *
1) "k2"
2) "k1"
3) "k3"
127.0.0.1:6379> mget k1 k3
1) "v1"
2) "v3"
127.0.0.1:6379> msetnx k1 v11 k4 v4  # 创建失败， 说明msetnx是原子性的操作
(integer) 0
127.0.0.1:6379> keys *
1) "k2"
2) "k1"
3) "k3"
127.0.0.1:6379> msetnx k4 v4 k1 v11
(integer) 0
127.0.0.1:6379> keys *
1) "k2"
2) "k1"
3) "k3"

########################################################
# 设置对象
set user:1 {name:zhangsan,age:13}   # user:{id}作为key，对象的json信息对应value

#########################################################
#getset  #先get在set
127.0.0.1:6379> getset db redis
(nil)
127.0.0.1:6379> get db
"redis"
127.0.0.1:6379> getset db mongodb
"redis"
127.0.0.1:6379> get db
"mongodb"
```



#### List

- 所有的list命令都是“L”开头的。
- 想象成一个链表，可以在表头和表尾进行操作

```bash
#########################################################
# lpush 向链表头部插入数据
# rpush 向链表尾部插入数据
127.0.0.1:6379> lpush list one   # 将一个值或者多个值插入到链表的头部
(integer) 1
127.0.0.1:6379> lpush list two
(integer) 2
127.0.0.1:6379> lpush list three
(integer) 3
127.0.0.1:6379> lrange list 0 -1
1) "three"
2) "two"
3) "one"
127.0.0.1:6379> lrange list 0 1
1) "three"
2) "two"
127.0.0.1:6379> rpush list four
(integer) 4
127.0.0.1:6379> lrange list 0 -1
1) "three"
2) "two"
3) "one"
4) "four"

#########################################################
# lpop 从头移除
# rpop 从尾部移除
127.0.0.1:6379> lrange list 0 -1
1) "three"
2) "two"
3) "one"
4) "four"
127.0.0.1:6379> lpop list
"three"
127.0.0.1:6379> rpop list
"four"
127.0.0.1:6379> lrange list 0 -1
1) "two"
2) "one"

#########################################################
# lindex 查询某个下标的值，下标从0开始
# llen  获取长度

127.0.0.1:6379> lrange list 0 -1
1) "two"
2) "one"
127.0.0.1:6379> lindex list 1
"one"
127.0.0.1:6379> llen list
(integer) 2

#########################################################
# lrem  移除一个值，中间的count表示移除几个重复值
127.0.0.1:6379> lrange list 0 -1
1) "two"
2) "two"
3) "one"
127.0.0.1:6379> lrem list 1 one
(integer) 1
127.0.0.1:6379> lrange list 0 -1
1) "two"
2) "two"
127.0.0.1:6379> lrem list 2 two
(integer) 2
127.0.0.1:6379> llen list
(integer) 0

#########################################################
# ltrim  按照下标截取元素
127.0.0.1:6379> rpush list "h0"
(integer) 1
127.0.0.1:6379> rpush list "h1"
(integer) 2
127.0.0.1:6379> rpush list "h2"
(integer) 3
127.0.0.1:6379> rpush list "h3"
(integer) 4
127.0.0.1:6379> ltrim list 1 2
OK
127.0.0.1:6379> lrange list 0 -1
1) "h1"
2) "h2"

#########################################################
# rpoplpush  移除列表（source）的最后一个元素，移除到新的列表中（dest）
127.0.0.1:6379> lrange list 0 -1
1) "h1"
2) "h2"
127.0.0.1:6379> rpush olist "h8"
(integer) 1
127.0.0.1:6379> rpoplpush list olist
"h2"
127.0.0.1:6379> lrange olist 0 -1
1) "h2"
2) "h8"
127.0.0.1:6379> lrange list 0 -1
1) "h1"

#########################################################
# lset list index new_item 将列表中指定下标的值替换为目标值，相当于一个更行操作，如果列表不存在就出错
# linsert list [before|after] item new_item  在item前|后插入new_item
```



#### Set（集合）

- set中的值不可重复

- set的命令开头都是s

  ```bash
  #########################################################
  # sadd       添加元素
  # smembers   所有元素
  # sismember  是否存在在集合中
  # scard      获取集合中元素的个数
  # srem       移除元素
  
  127.0.0.1:6379> sadd myset "hello"
  (integer) 1
  127.0.0.1:6379> sadd myset "lt"
  (integer) 1
  127.0.0.1:6379> smembers myset
  1) "hello"
  2) "lt"
  127.0.0.1:6379> sismember myset hello
  (integer) 1
  127.0.0.1:6379> sismember myset world
  (integer) 0
  127.0.0.1:6379> scard myset
  (integer) 2
  
  ######################################################
  # srandmember 随机获取一个元素
  # spop        随机删除一个元素
  127.0.0.1:6379> srandmember myset
  "hello"
  127.0.0.1:6379> srandmember myset
  "hello"
  127.0.0.1:6379> srandmember myset
  "lt"
  127.0.0.1:6379> srandmember myset
  "hello"
  
  ######################################################
  # 集合操作
  # sunion  集合并集
  # sdiff   集合差集
  # sinter  集合交集
  127.0.0.1:6379> smembers myset
  1) "hello"
  2) "lt"
  127.0.0.1:6379> sadd set "hello" "tt"
  (integer) 2
  127.0.0.1:6379> sunion myset set
  1) "hello"
  2) "tt"
  3) "lt"
  127.0.0.1:6379> sdiff myset set
  1) "lt"
  127.0.0.1:6379> sinter myset set
  1) "hello"
  ```



#### Hash（哈希）

- key-map，现在值是以map的形式存在。本质和string类型没有很大区别，还是简单的key-value
- **其实key是一个map的名字，可以在这个map中插入不同的kv对。**
- hash更加适合**对象**的存储

```bash
127.0.0.1:6379> hset myhash field1 lt   # <field1 lt> 就是value，相当于map
(integer) 1
127.0.0.1:6379> hget myhash field1
"lt"
127.0.0.1:6379> hmset myhash field1 hello field2 world
OK
127.0.0.1:6379> hmget myhash field1 field2
1) "hello"
2) "world"
127.0.0.1:6379> hgetall myhash
1) "field1"
2) "hello"
3) "field2"
4) "world"
127.0.0.1:6379> hdel myhash field1
(integer) 1
127.0.0.1:6379> hgetall myhash
1) "field2"
2) "world"
127.0.0.1:6379> hkeys myhash
1) "field2"
127.0.0.1:6379> hvals myhash
1) "world"
```



#### Zset（有序集合）





#### Hyperloglog

- 做**计数**，做统计

>  什么是基数（不重复的元素）?
>
> ​	e.g.  A（1，3,5,7,8,7）   B（1,，3,5,7,8）
>
> ​	基数（不重复的元素） = 5，（可以接受误差！）

- **Hyperloglog是用来做基数统计的算法**！

  - 优点：**占用的内存是固定的，而且极少**，2^64不同的元素的基数，只需要费**12kb**的内存！但是可能会有一定的**错误率**。

  > 如何统计网站的UV（访问量）（一个用户如果访问多次，则只能算作一个人！）
  >
  > - 传统的方法，用set保存用户的ID，然后就可以去重了

```bash
127.0.0.1:6379> pfadd mykey a b c d e
(integer) 1
127.0.0.1:6379> pfcount mykey   # 统计基数数量
(integer) 5
127.0.0.1:6379> pfadd mykey2 g f e a d v g
(integer) 1
127.0.0.1:6379> pfcount mykey2
(integer) 6
127.0.0.1:6379> pfmerge mykey3 mykey mykey2  # 并集
OK
127.0.0.1:6379> pfcount mykey3
(integer) 8
```



#### Bitmaps

> 位存储

- **两个状态的，都可以使用Bitmaps**

  ```bash
  # setbit
  # getbit
  # bitcount
  127.0.0.1:6379> setbit sign 0 1
  (integer) 0
  127.0.0.1:6379> setbit sign 1 0
  (integer) 0
  127.0.0.1:6379> setbit sign 2 1
  (integer) 0
  127.0.0.1:6379> getbit sign 1
  (integer) 0
  127.0.0.1:6379> getbit sign 2
  (integer) 1
  127.0.0.1:6379> bitcount sign
  (integer) 2
  
  ```

  



































