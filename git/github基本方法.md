#### git结构

![image-20200926164634315](.\pictures\git结构.png)

#### git和github

- github是git的**代码托管中心**。git是可以操作代码托管中心的工具。
- 代码托管中心：维护远程库
  - 在局域网中
    - 搭建gitlab服务器，作为代码托管中心
  - 在外网中
    - github
    - 码云



#### 本地库和远程库

- **团队内协作**

  ![image-20200926165819767](.\pictures\团队内协作.png)

- **跨团队协作**（比如你所在的团队没法解决某个问题，需要别的公司的其他团队帮忙）

![image-20200926170135366](.\pictures\团队之间协作.png)





### git命令行操作

------

#### 本地库操作

- **git init**
  
- 生成`.git`文件，里面包含了**本地库**相关的子目录和文件。
  
- **设置签名**

  - 形式

    - 用户名：tom
    - Email地址：goodMorning@atguigu.com

  - 作用

    - 区分不同开发人员的身份

  - 辨析：这里设置的签名和登录远程库（代码托管中心）的账号密码没有任何关系。

  - 命令

    - 项目级别/仓库级别：用户名和email仅在当前本地库范围内有效
      - **git config** user.name tom
      - **git config** user.email goodMorning@atguigu.com
      - 信息保存在：./.git/config

    - 系统用户级别：登录当前操作系统的用户范围。
      - **git config --global**
        - 信息保存在：~/.gitconfig
    - 就近原则，项目级别优先级>系统用户级别。不允许二者都没有。

- **添加提交与版本回退**

  - git status  查看当前库状态
  - git add <filename...>   把文件添加到暂存区   git add .
  - git rm --cached <filename...> 把文件从暂存区中删除
  - git commit -m "message"  <filename...>（filename可以带可以不带）
  - 文件在创建之后没有进行add时，必须先进行一次git add操作，把文件变成**track**。如果已经提交过一次了（已经被track了），也可以直接使用git commit命令

  

  - git log  显示提交日志

    - git log --pretty=oneline
    - git log --oneline
    - git reflog

  - **HEAD**是一个指针，指向当前的所在的版本

    ![image-20200927093322775](.\pictures\版本回退.png)

  - **基于索引值**的版本回退

    - **git reset --hard** b598e1f
    - git reset --hard HEAD

  - **基于^符号**

    - 只能回退，不能往前

    - 每一个^回退一个版本

      ![image-20200927094601595](.\pictures\head^.png)

  - **基于~符号**

    - 只能后退
    - git reset --hard HEAD~3（回退三步）

  - **reset命令的三个参数对比**

    - --soft参数

      ```
      Does not touch the index file(暂存区) or the working tree（工作区） at all (but resets the head to <commit>, just like all modes do). This leaves all your changed files "Changes to be committed", as git status would put it.
      ```

    - --mixed参数

      ```
      Resets the index but not the working tree (i.e., the changed files are preserved but not marked for commit) and reports what has not been updated. This is the default action.
      ```

    - --hard参数

      ```
      Resets the index and working tree. Any changes to tracked files in the working tree since <commit> are discarded.
      ```

- git只会增加版本，不会清除版本信息。所以即使删除文件，也是可以通过版本回退进行找回。前提就是：你删除工作区中的文件之前，已经提交到了本地库。

- **比较文件**

  - **git diff** test.md

    - 不带文件名就比较所有文件。

    - 默认：和暂存区中的版本进行比较

      ![image-20200927104521052](.\pictures\版本比较.png)

      - 和本地库中的某个版本进行比较

        ![image-20200927104635829](.\pictures\版本比较-工作区vs本地库.png)

        ![image-20200927104744469](.\pictures\版本比较head^.png)

        ![image-20200927104932566](.\pictures\版本比较-不带文件名.png)

- **分支管理**
  - 在版本控制中，使用多条线同时推进多个任务。 

  - 查看分支信息：**git branch -v**

  - 创建分支，【hot_fix是分支名】：**git branch** hot_fix

  - 切换分支：**git checkout** hot_fix

  - 合并分支

    - 切换到接收修改的分支上（被合并的分支）：git checkout master
    - 执行merge命令：git merge hot_fix

  - **产生冲突**

    - 两个分支对同一个文件的同一个地方进行了不一样的修改。

    - 分支冲突的表现：在发生冲突的文件中，是这样的

      ![image-20200927113154028](.\pictures\分支冲突.png)

    - 解决方法：直接在冲突文件中进行修改，然后git add <文件名>   git commit -m "message"。此状态下git commit后面不需要文件名。

    - 合并完了之后，只会影响到master中的文件和版本信息，hot_fix中的不改变。

    - 有趣的点：当在master分支时，打开windows的文件管理器，查看文件，发现是master本地库中的文件内容，但是当切换到hot_fix分支时，查看文件，发现是hot_fix本地库中的文件内容。也就是说每当切换版本的时候，或者切换分支的时候，都会把工作区对应到当前版本上来。

- git采用sha1哈希算法

  

#### 远程库操作

##### 团队内合作

![image-20200927135151086](.\pictures\团队内合作.png)

- `git remote -v`查看远程库。
- `git remote add origin https://github.com/ylembot/huashan.git` 用origin这个名字替代远程库的地址。

![image-20200927164545766](.\pictures\添加远程库.png)

- 使用`git push origin master`来提交本地分支到远程库。

- **把远程库克隆到本地**

  - `git clone [url]`
    - 完整的把远程库下载下来
    - 创建origin远程地址别名
    - 初始化本地库（下载的库文件中，有.git文件）
  - 需要仓库拥有者把团队其他成员加入到这个仓库中，成为该仓库的一员，此时这个非仓库拥有者才可以写这个仓库。

- **拉取（下载）远程库**

  - pull = fetch + merge

  - `git fetch origin master`   把远程库中的文件下载到本地，但是还没有修改本地文件。这里的master是远程地址的分支名。`git fetch [远程库别名] [远程分支名]`

    - `git checkout origin/master`  可以切换到下载的这个快照上面来。

  - `git merge origin/master` 把origin/master合并到当前所在的分支

    ![image-20200927181848759](.\pictures\merge.png)

  - `git pull origin master`  可以直接抓取远程库，并且合并到本地库中。

- **团队协作中的冲突**

  - 如果两个人修改同一个文件中的同一个位置，那么先推送到远程库的人可以推送成功，后推送的人推送不成功，必须先把远程库上面的内容拉取下来（即先把自己的更新到最新版）。git pull之后一般会有冲突，然后我们修改冲突文件，在重新push。
  - **要点**
    - 如果不是基于github远程库的最新版所做的修改，不能推送，必须先拉取。
    - 拉取下来后如果进入冲突状态，则按照“分支冲突解决”操作解决冲突即可。然后在重新push。

##### 团队之间协作

![image-20200927232930570](.\pictures\团队之间协作1.png)

![image-20200927233147659](.\pictures\fork.png)

![image-20200927233237672](.\pictures\fork1.png)

- 1. 本地修改，然后推送到远程库

- 2. 然后

  ![](.\pictures\pullrequest.png)

- 3. 在然后

![image-20200927234310346](.\pictures\pullrequest1.png)

- 4. 在然后

  ![image-20200927234446955](.\pictures\pullrequest2.png)

- 在然后

  ![image-20200927234705021](.\pictures\pullrequest3.png)

- 在然后

  ![image-20200927235119256](.\pictures\pullrequest合并代码.png)

##### ssh

- 避免总是输入账号密码

- `rm -r .ssh`    删除ssh文件

- `ssh-keygen -t rsa -C 【邮箱】`

- 然后把生成的.ssh文件夹中的id_rsa.pub中的代码复制到这里

  ​	  ![image-20200928000828373](.\pictures\ssh1.png)

- 添加ssh别名：`git remote add origin_ssh git@github.com:ylembot/huashan.git`

- 然后使用`git push origin_ssh master`就不需要输入账号密码啦。

- **说明**：在使用https时，windows10  里面会记录登录状态，`控制面板-用户账户-凭据管理器`。



































