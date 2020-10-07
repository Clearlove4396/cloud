## 各个文件夹的作用







## 用户和组

- 如果不指定用户所在的组，则在创建用户时，自动创建一个与用户名同名的组。

- 每一个用户都属于一个组

- /etc/passwd  用户配置文件

- /etc/group    组配置文件（组信息）

- /etc/shadow      口令配置文件

  ![image-20201007154130193](.\pictures\用户和组.png)

  

  ```shell
  useradd [用户名]
  useradd -d [家目录名] [用户名]  #指定用户家目录
  useradd -g [组名] [用户名]  # 指定用户所在的组
  
  userdel [用户名]  # 删除用户，当时保留家目录
  userdel -r [用户名] #删除用户，同时删除家目录
  
  usermod -g [组名] [用户名]  # 修改用户所在的组
  
  groupadd [组名]
  groupdel [组名]
  
  id [用户名]  #查看用户所在的组信息等
  
  whoami  # 输出当前用户名
  
  passwd [用户名]
  ```

  

## 常用命令







## shell

