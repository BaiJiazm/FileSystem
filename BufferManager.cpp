#include "BufferManager.h"
#include "Utility.h"

extern DeviceDriver g_DeviceDriver;

/* 
 *	Bufferֻ�õ���������־��B_DONE��B_DELWRI���ֱ��ʾ�Ѿ����IO���ӳ�д�ı�־��
 *	����Buffer���κα�־
*/
BufferManager::BufferManager() {
    bufferList = new Buffer;
    InitList();
    deviceDriver = &g_DeviceDriver;
}

BufferManager::~BufferManager() {
    Bflush();
    delete bufferList;
}

void BufferManager::FormatBuffer() {
    Buffer emptyBuffer;
    for (int i = 0; i < NBUF; ++i) {
        Utility::memcpy(nBuffer + i, &emptyBuffer, sizeof(Buffer));
    }
    InitList();
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
        nBuffer[i].no = i;
    }
}

/* ����LRU Cache �㷨��ÿ�δ�ͷ��ȡ����ʹ�ú�ŵ�β��
*/
void BufferManager::DetachNode(Buffer* pb) {
    if (pb->back == NULL) {
        return;
    }
    pb->forw->back = pb->back;
    pb->back->forw = pb->forw;
    pb->back = NULL;
    pb->forw = NULL;
}

void BufferManager::InsertTail(Buffer* pb) {
    if (pb->back != NULL) {
        return;
    }
    pb->forw = bufferList->forw;
    pb->back = bufferList;
    bufferList->forw->back = pb;
    bufferList->forw = pb;
}

/* ����һ�黺�棬�ӻ��������ȡ�������ڶ�д�豸�ϵĿ�blkno��*/
Buffer* BufferManager::GetBlk(int blkno) {
    Buffer* pb;
    if (map.find(blkno) != map.end()) {
        pb = map[blkno];
        DetachNode(pb);
        return pb;
    }
    pb = bufferList->back;
	if (pb == bufferList) {
		cout << "��Buffer�ɹ�ʹ��" << endl;
		return NULL;
	}
    DetachNode(pb);
	map.erase(pb->blkno);
    if (pb->flags&Buffer::B_DELWRI) {
        deviceDriver->write(pb->addr, BUFFER_SIZE, pb->blkno*BUFFER_SIZE);
    }
	pb->flags &= ~(Buffer::B_DELWRI | Buffer::B_DONE);
	pb->blkno = blkno;
	map[blkno] = pb;
    return pb;
}

/* �ͷŻ�����ƿ�buf */
void BufferManager:: Brelse(Buffer* pb) {
	InsertTail(pb);
}

/* ��һ�����̿飬blknoΪĿ����̿��߼���š� */
Buffer* BufferManager::Bread(int blkno) {
    Buffer* pb = GetBlk(blkno);
    //pb->debugMark();
    //pb->debugContent();
	if (pb->flags&(Buffer::B_DONE | Buffer::B_DELWRI)) {
		return pb;
	}
    deviceDriver->read(pb->addr, BUFFER_SIZE, pb->blkno*BUFFER_SIZE);
	pb->flags |= Buffer::B_DONE;
    return pb;
}

/* дһ�����̿� */
void BufferManager::Bwrite(Buffer *pb) {
    //pb->debugMark();
    //pb->debugContent();
	pb->flags &= ~(Buffer::B_DELWRI);
	deviceDriver->write(pb->addr, BUFFER_SIZE, pb->blkno*BUFFER_SIZE);
	pb->flags |= (Buffer::B_DONE);
	this->Brelse(pb);
}

/* �ӳ�д���̿� */
void BufferManager::Bdwrite(Buffer* bp) {
	bp->flags |= (Buffer::B_DELWRI | Buffer::B_DONE);
	this->Brelse(bp);
	return;
}

/* ��ջ��������� */
void BufferManager::Bclear(Buffer *bp) {
	Utility::memset(bp->addr, 0, BufferManager::BUFFER_SIZE);
	return;
}

/* ���������ӳ�д�Ļ���ȫ����������� */
void BufferManager::Bflush() {
    Buffer* pb = NULL;
    for (int i = 0; i < NBUF; ++i) {
        pb = nBuffer + i;
        if ((pb->flags & Buffer::B_DELWRI)) {
            pb->flags &= ~(Buffer::B_DELWRI);
            deviceDriver->write(pb->addr, BUFFER_SIZE, pb->blkno*BUFFER_SIZE);
            pb->flags |= (Buffer::B_DONE);
        }
    }
}

void BufferManager::debug() {
    for (Buffer* pb = bufferList->back; pb != bufferList; pb = pb->back) {
        pb->debugMark();
    }
    cout << endl;
}