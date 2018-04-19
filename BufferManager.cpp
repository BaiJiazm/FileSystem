#include "BufferManager.h"
#include "Utility.h"

extern DeviceDriver g_DeviceDriver;

/* 
 *	Buffer 只用到了两个标志，B_DONE和B_DELWRI，分别表示已经完成IO和延迟写的标志。
 *	空闲Buffer无任何标志
*/
BufferManager::BufferManager() {
    bufferList = new Buffer;
    InitList();
    deviceDriver = &g_DeviceDriver;
}

BufferManager::~BufferManager() {
    delete bufferList;
}

void BufferManager::InitList() {
    for (int i = 0; i < NBUF; ++i) {
        if (i) {
            nBuffer[i].forw = nBuffer + i - 1;
        }
        else {
            nBuffer[i].forw = bufferList;
            bufferList->back = nBuffer + i;
        }

        if (i + 1 < NBUF) {
            nBuffer[i].back = nBuffer + i + 1;
        }
        else {
            nBuffer[i].back = bufferList;
            bufferList->forw = nBuffer + i;
        }
        nBuffer[i].addr = buffer[i];
    }
}

void BufferManager::DetachNode(Buffer* pb) {
    pb->forw->back = pb->back;
    pb->back->forw = pb->forw;
}

void BufferManager::InsertTail(Buffer* pb) {
    pb->forw = bufferList->forw;
    pb->back = bufferList;
    pb->forw->back = pb;
    bufferList->forw = pb;
}

/* 申请一块缓存，从缓存队列中取出，用于读写设备上的块blkno。*/
Buffer* BufferManager::GetBlk(int blkno) {
    Buffer* pb;
    if (map.find(blkno) != map.end()) {
        pb = map[blkno];
        DetachNode(pb);
        return pb;
    }
    pb = bufferList->back;
	if (pb == bufferList) {
		cout << "无Buffer可供使用" << endl;
		return NULL;
	}
    DetachNode(pb);
	map.erase(pb->blkno);
    if (pb->flags&Buffer::B_DELWRI) {
        deviceDriver->write(pb->addr, BUFFER_SIZE, pb->blkno*BUFFER_SIZE);
    }
	pb->flags &= ~(Buffer::B_DELWRI | Buffer::B_DONE);
	pb->blkno = blkno;
	map[pb->blkno] = pb;
    return pb;
}

/* 释放缓存控制块buf */
void BufferManager:: Brelse(Buffer* pb) {
	InsertTail(pb);
}

/* 读一个磁盘块，blkno为目标磁盘块逻辑块号。 */
Buffer* BufferManager::Bread(int blkno) {
    Buffer* pb = GetBlk(blkno);
	if (pb->flags&(Buffer::B_DONE | Buffer::B_DELWRI)) {
		return pb;
	}
    deviceDriver->read(pb->addr, BUFFER_SIZE, pb->blkno*BUFFER_SIZE);
	pb->flags |= Buffer::B_DONE;
    return pb;
}

/* 写一个磁盘块 */
void BufferManager::Bwrite(Buffer *pb) {
	pb->flags &= ~(Buffer::B_DELWRI);
	deviceDriver->write(pb->addr, BUFFER_SIZE, pb->blkno*BUFFER_SIZE);
	pb->flags |= (Buffer::B_DONE);
	this->Brelse(pb);
}

/* 延迟写磁盘块 */
void BufferManager::Bdwrite(Buffer* bp) {
	bp->flags |= (Buffer::B_DELWRI | Buffer::B_DONE);
	this->Brelse(bp);
	return;
}

/* 清空缓冲区内容 */
void BufferManager::Bclear(Buffer *bp) {
	Utility::memset(bp->addr, 0, sizeof(BufferManager::BUFFER_SIZE));
	return;
}

/* 将队列中延迟写的缓存全部输出到磁盘 */
void BufferManager::Bflush() {
	Buffer* pb;
	for (pb = bufferList->forw; pb != bufferList; pb = pb->forw) {
		if ((pb->flags & Buffer::B_DELWRI)) {
			this->Bwrite(pb);
		}
	}
}