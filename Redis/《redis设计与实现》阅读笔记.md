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



