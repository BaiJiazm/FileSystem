#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "INode.h"
#include "DeviceDriver.h"
#include "BufferManager.h"

/*
 * 文件系统存储资源管理块(Super Block)的定义。
 */
class SuperBlock {
public:
    const static int MAX_NFREE = 100;
    const static int MAX_NINODE = 100;

public:
    int	s_isize;		        // 外存INode区占用的盘块数
    int	s_fsize;		        // 盘块总数

    int	s_nfree;		        // 直接管理的空闲盘块数量
    int	s_free[MAX_NFREE];	    // 直接管理的空闲盘块索引表

    int	s_ninode;		        // 直接管理的空闲外存INode数量
    int	s_inode[MAX_NINODE];	// 直接管理的空闲外存INode索引表

    int	s_flock;		        // 封锁空闲盘块索引表标志
    int	s_ilock;		        // 封锁空闲INode表标志

    int	s_fmod;			        // 内存中super block副本被修改标志，意味着需要更新外存对应的Super Block
    int	s_ronly;		        // 本文件系统只能读出
    int	s_time;			        // 最近一次更新时间
    int	padding[47];	        // 填充使SuperBlock块大小等于1024字节，占据2个扇区
};

class DirectoryEntry {
public:
    static const int DIRSIZ = 28;	/* 目录项中路径部分的最大字符串长度 */

public:
    int m_ino;		    /* 目录项中INode编号部分 */
    char name[DIRSIZ];	/* 目录项中路径名部分 */
};

class FileSystem {
public:
    // Block块大小
    static const int BLOCK_SIZE = 512;

    // 磁盘所有扇区数量
    static const int DISK_SIZE = 16384;

    // 定义SuperBlock位于磁盘上的扇区号，占据两个扇区
    static const int SUPERBLOCK_START_SECTOR = 0;


    // 外存INode区位于磁盘上的起始扇区号
    static const int INODE_ZONE_START_SECTOR = 2;

    // 磁盘上外存INode区占据的扇区数
    static const int INODE_ZONE_SIZE = 1022;

    // 外存INode对象长度为64字节，每个磁盘块可以存放512/64 = 8个外存INode
    static const int INODE_NUMBER_PER_SECTOR = BLOCK_SIZE / sizeof(DiskINode);

    // 文件系统根目录外存INode编号
    static const int ROOT_INODE_NO = 0;

    // 外存INode的总个数
    static const int INode_NUMBERS = INODE_ZONE_SIZE* INODE_NUMBER_PER_SECTOR;


    // 数据区的起始扇区号
    static const int DATA_ZONE_START_SECTOR = INODE_ZONE_START_SECTOR + INODE_ZONE_SIZE;

    // 数据区的最后扇区号
    static const int DATA_ZONE_END_SECTOR = DISK_SIZE - 1;

    // 数据区占据的扇区数量
    static const int DATA_ZONE_SIZE = DISK_SIZE - DATA_ZONE_START_SECTOR;

public:
    DeviceDriver* deviceDriver;
    SuperBlock* superBlock;
	BufferManager* bufferManager;

public:
    FileSystem();
    ~FileSystem();

	/* 格式化SuperBlock */
    void FormatSuperBlock();

	/* 格式化整个文件系统 */
    void FormatDevice();

    /* 系统初始化时读入SuperBlock */
    void LoadSuperBlock();

    /* 将SuperBlock对象的内存副本更新到存储设备的SuperBlock中去 */
    void Update();

    /* 在存储设备dev上分配一个空闲外存INode，一般用于创建新的文件。*/
    INode* IAlloc();

    /* 释放编号为number的外存INode，一般用于删除文件。*/
    void IFree(int number);

    /* 在存储设备上分配空闲磁盘块 */
    Buffer* Alloc();

    /* 释放存储设备dev上编号为blkno的磁盘块 */
    void Free(int blkno);

};

#endif