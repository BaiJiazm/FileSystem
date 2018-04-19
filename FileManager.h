#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "FileSystem.h"
#include "OpenFileManager.h"

/*
* 文件管理类(FileManager)
* 封装了文件系统的各种系统调用在核心态下处理过程，
* 如对文件的Open()、Close()、Read()、Write()等等
* 封装了对文件系统访问的具体细节。
*/
class FileManager
{
public:
    /* 目录搜索模式，用于NameI()函数 */
    enum DirectorySearchMode
    {
        OPEN = 0,		/* 以打开文件方式搜索目录 */
        CREATE = 1,		/* 以新建文件方式搜索目录 */
        DELETE = 2		/* 以删除文件方式搜索目录 */
    };

public:
    /* 根目录内存INode */
    INode* rootDirINode;

    /* 对全局对象g_FileSystem的引用，该对象负责管理文件系统存储资源 */
    FileSystem* m_FileSystem;

    /* 对全局对象g_INodeTable的引用，该对象负责内存INode表的管理 */
    INodeTable* m_INodeTable;

    /* 对全局对象g_OpenFileTable的引用，该对象负责打开文件表项的管理 */
    OpenFileTable* m_OpenFileTable;

public:
    FileManager();
    ~FileManager();

    /*
    * @comment Open()系统调用处理过程
    */
    void Open();

    /*
    * @comment Creat()系统调用处理过程
    */
    void Creat();

    /*
    * @comment Open()、Creat()系统调用的公共部分
    */
    void Open1(INode* pINode, int mode, int trf);

    /*
    * @comment Close()系统调用处理过程
    */
    void Close();

    /*
    * @comment Seek()系统调用处理过程
    */
    void Seek();

    /*
    * @comment FStat()获取文件信息
    */
    void FStat();

    /*
    * @comment FStat()获取文件信息
    */
    void Stat();

    /* FStat()和Stat()系统调用的共享例程 */
    void Stat1(INode* pINode, unsigned long statBuf);

    /*
    * @comment Read()系统调用处理过程
    */
    void Read();

    /*
    * @comment Write()系统调用处理过程
    */
    void Write();

    /*
    * @comment 读写系统调用公共部分代码
    */
    void Rdwr(enum File::FileFlags mode);

    /*
    * @comment 目录搜索，将路径转化为相应的INode，
    * 返回上锁后的INode
    */
    INode* NameI(enum DirectorySearchMode mode);

    /*
    * @comment 被Creat()系统调用使用，用于为创建新文件分配内核资源
    */
    INode* MakNode(unsigned int mode);

    /*
    * @comment 向父目录的目录文件写入一个目录项
    */
    void WriteDir(INode* pINode);

    /*
    * @comment 设置当前工作路径
    */
    void SetCurDir(char* pathname);

    /* 改变文件访问模式 */
    void ChMod();

    /* 改变当前工作目录 */
    void ChDir();
};

#endif