#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include "Buffer.h"
#include "DeviceDriver.h"
#include <bits/stdc++.h>
#include <unordered_map>
using namespace std;

class BufferManager {
public:
    static const int NBUF = 100;			/* 缓存控制块、缓冲区的数量 */
    static const int BUFFER_SIZE = 512;		/* 缓冲区大小。 以字节为单位 */

private:
    Buffer* bufferList;							/* 缓存队列控制块 */
    Buffer nBuffer[NBUF];						/* 缓存控制块数组 */
    unsigned char buffer[NBUF][BUFFER_SIZE];	/* 缓冲区数组 */
    unordered_map<int, Buffer*> map;
    DeviceDriver* deviceDriver;

public:
    BufferManager();
    ~BufferManager();

	/* 申请一块缓存，用于读写设备上的块blkno。*/
    Buffer* GetBlk(int blkno);
    
	/* 释放缓存控制块buf */
	void Brelse(Buffer* bp);

	/* 读一个磁盘块，blkno为目标磁盘块逻辑块号。 */
    Buffer* Bread(int blkno);

	/* 写一个磁盘块 */
    void Bwrite(Buffer* bp);
    
	/* 延迟写磁盘块 */
	void Bdwrite(Buffer* bp);

	/* 清空缓冲区内容 */
    void Bclear(Buffer* bp);
    
	/* 将队列中延迟写的缓存全部输出到磁盘 */
	void Bflush();
    
    /* 获取空闲控制块Buf对象引用 */
	//Buffer& GetFreeBuffer();				
	
    /* 格式化所有Buffer */
    void FormatBuffer();

private:
    void InitList();
    void DetachNode(Buffer* pb);
    void InsertTail(Buffer* pb);
    void debug();
};

#endif