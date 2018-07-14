## File System (Unix)

### 项目简介

本项目是一个类似`Unix`文件系统的二级文件系统，即使用一个普通的大文件（`unix-fs.img` ，称之为一级文件）模拟`UNIX V6++`文件系统的一个文件卷。在完成本项目之前较为详细的阅读了Unix V6++系统的源代码，特别是文件系统相关部分。本文件系统为用户提供命令行界面，基本功能模块包括用户界面模块、文件打开表管理模块、系统访问管理模块、系统盘块管理模块、高速缓存管理模块、设备驱动模块等，可跨平台运行。所有工作由一人独立完成。

### 模块说明

* DeviceDriver：设备驱动模块，直接负责磁盘文件直接读写。
* BufferManager：高速缓存管理模块，主要负责管理系统中所有的缓存块，包括申请、释放、读写、清空一块缓存的功能函数接口，以及系统退出时刷新所有缓存块。
* FileSystem：系统盘块管理模块，主要负责对镜像文件的存储空间管理，包括SuperBlock空间占用、DiskINode空间分布、数据块区空间分布的管理。需要提供分配、回收DiskINode节点、数据块节点以及格式化磁盘文件的接口。
* FileManager：系统文件操作功能实现模块，主要封装文件系统中对文件处理的操作过程，负责对文件系统访问的具体细节。包括打开文件、创建文件、关闭文件、Seek文件指针、读取文件、写入文件、删除文件等系统功能实现。
* OpenFileManager：打开文件管理模块，负责对打开文件的管理，为用户打开文件建立数据结构之间的勾连关系，为用户提供直接操作文件的文件描述符接口。
* User：用户操作接口模块，主要将用户的界面执行命令转化为对相应函数的调用，同时对输出进行处理，也包含检查用户输入的正确性与合法性。

### 详细设计

*   [见文档](https://github.com/BaiJiazm/FileSystem/blob/master/1453381-%E6%9B%BE%E9%B8%A3-%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F%E8%AF%BE%E7%A8%8B%E8%AE%BE%E8%AE%A1%E6%8A%A5%E5%91%8A.pdf)

### 使用说明

##### 项目结构

    [user@sdn os]$ tree
    .
    |-- Buffer.cpp
    |-- Buffer.h
    |-- BufferManager.cpp
    |-- BufferManager.h
    |-- DeviceDriver.cpp
    |-- DeviceDriver.h
    |-- file11-100000.txt
    |-- file1-2000.txt
    |-- File.cpp
    |-- File.h
    |-- FileManager.cpp
    |-- FileManager.h
    |-- FileSystem.cpp
    |-- FileSystem.h
    |-- INode.cpp
    |-- INode.h
    |-- main.cpp
    |-- Makefile
    |-- OpenFileManager.cpp
    |-- OpenFileManager.h
    |-- readOut1.txt
    |-- User.cpp
    |-- User.h
    |-- Utility.cpp
    `-- Utility.h
    
    0 directories, 25 files

##### 编译方法

可以将源程序放在集成环境下编译，也可利用GNU编译工具，通过写好的Makefile进行编译，make如下：

    [user@sdn os]$ make
    g++ -std=c++11 -w -c -o main.o main.cpp
    g++ -std=c++11 -w -c -o Buffer.o Buffer.cpp
    g++ -std=c++11 -w -c -o BufferManager.o BufferManager.cpp
    g++ -std=c++11 -w -c -o DeviceDriver.o DeviceDriver.cpp
    g++ -std=c++11 -w -c -o File.o File.cpp
    g++ -std=c++11 -w -c -o FileManager.o FileManager.cpp
    g++ -std=c++11 -w -c -o FileSystem.o FileSystem.cpp
    g++ -std=c++11 -w -c -o INode.o INode.cpp
    g++ -std=c++11 -w -c -o OpenFileManager.o OpenFileManager.cpp
    g++ -std=c++11 -w -c -o User.o User.cpp
    g++ -std=c++11 -w -c -o Utility.o Utility.cpp
    g++ -o unix-fs main.o Buffer.o BufferManager.o DeviceDriver.o File.o FileManager.o FileSystem.o INode.o OpenFileManager.o User.o Utility.o

##### 运行说明

在Windows下直接点击生成的exe文件执行，在Linux平台直接`./unix-fs`即可运行。运行界面为控制台的命令行方式，命令较为简单，通俗易懂，初始界面如下：

    ++++++++++++++++++++ Unix文件系统模拟 ++++++++++++++++++++
    Command       :  man -显示在线帮助手册 
    Description   :  帮助用户使用相应命令 
    Usage         :  man [命令] 
    Parameter     :  [命令] 如下：  
                     man          :  手册 
                     fformat      :  格式化 
                     exit         :  正确退出 
                     mkdir        :  新建目录 
                     cd           :  改变目录 
                     ls           :  列出目录及文件 
                     create       :  新建文件 
                     delete       :  删除文件 
                     open         :  打开文件 
                     close        :  关闭文件 
                     seek         :  移动读写指针 
                     write        :  写入文件 
                     read         :  读取文件 
                     at|autoTest  :  自动测试 
    Usage Demo    :  man mkdir

若对任何命令有不太清楚的地方，可直接man [Command]查看详细说明，以read命令为例：

    [unix-fs / ]$ man read
    Command       :  read -读取文件 
    Description   :  类Unix|Linux函数read，从一个已经打开的文件中读取。若出现错误，会有相应错误提示! 
    Usage         :  read <file descriptor> [-o <OutFileName>] <size> 
    Parameter     :  <file descriptor> open返回的文件描述符 
                     [-o <OutFileName>] -o 指定输出方式为文件，文件名为 <OutFileName> 默认为shell 
                     <size> 指定读取字节数，大小为 <size> 
    Usage Demo    :  read 1 -o OutFileName 123 
                     read 1 123 
    Error Avoided :  文件描述符不存在或超出范围，未正确指定参数

正确退出：
若要退出程序，最好通过exit命令。此时正常退出会调用析构函数。若有在内存中未更新到磁盘上的缓存会及时更新，保证正确性。若点 击窗口关闭按钮，属于给当前程序发信号强制退出，不会调用析构函 数，未写回部分信息，再次启动时可能出现错误!

格式化：
格式化命令为fformat，运行命令后系统会进行文件系统格式化，然后正常退出，再次进入时为初始环境。
