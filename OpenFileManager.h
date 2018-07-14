#ifndef OPENFILEMANAGER_H
#define OPENFILEMANAGER_H

#include "File.h"
#include "FileSystem.h"

/*
 * 打开文件管理类(OpenFileManager)负责内核中对打开文件机构的管理，为进程打开文件建立内
 * 核数据结构之间的勾连关系。
 * 勾连关系指进程u区中打开文件描述符指向打开文件表中的File打开文件控制结构，以及从File
 * 结构指向文件对应的内存INode。
 */
class OpenFileTable {
public:
    static const int MAX_FILES = 100;	/* 打开文件控制块File结构的数量 */

public:
	/* 系统打开文件表，为所有进程共享，进程打开文件描述符表
	 * 中包含指向打开文件表中对应File结构的指针。
	 */
    File m_File[MAX_FILES];	

public:
	OpenFileTable();
	~OpenFileTable();

	/* 在系统打开文件表中分配一个空闲的File结构 */
    File* FAlloc();

    /* 对打开文件控制块File结构的引用计数count减1，若引用计数count为0，则释放File结构。*/
    void CloseF(File* pFile);

    void Format();
};

/*
 * 内存INode表(class INodeTable)
 * 负责内存INode的分配和释放。
 */
class INodeTable {
public:
    static const int NINODE = 100;	/* 内存INode的数量 */

private:
    INode m_INode[NINODE];		/* 内存INode数组，每个打开文件都会占用一个内存INode */
    FileSystem* fileSystem;	/* 对全局对象g_FileSystem的引用 */

public:
    INodeTable();
    ~INodeTable();

    /*
     * 根据外存INode编号获取对应INode。如果该INode已经在内存中，返回该内存INode；
     * 如果不在内存中，则将其读入内存后上锁并返回该内存INode
     */
    INode* IGet(int inumber);
    
    /*
     * 减少该内存INode的引用计数，如果此INode已经没有目录项指向它，
     * 且无进程引用该INode，则释放此文件占用的磁盘块。
     */
    void IPut(INode* pNode);

    /* 将所有被修改过的内存INode更新到对应外存INode中 */
    void UpdateINodeTable();

    /*
     * 检查编号为inumber的外存INode是否有内存拷贝，
     * 如果有则返回该内存INode在内存INode表中的索引
     */
    int IsLoaded(int inumber);
    
    /* 在内存INode表中寻找一个空闲的内存INode */
    INode* GetFreeINode();

    void Format();
};

#endif