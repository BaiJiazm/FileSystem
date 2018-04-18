#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include "Buffer.h"
#include "DeviceDriver.h"
#include <bits/stdc++.h>
#include <unordered_map>
using namespace std;

class BufferManager {
public:
    static const int NBUF = 100;			/* ������ƿ顢������������ */
    static const int BUFFER_SIZE = 512;	/* ��������С�� ���ֽ�Ϊ��λ */

private:
    Buffer* bufferList;						/* ������п��ƿ� */
    Buffer nBuffer[NBUF];					/* ������ƿ����� */
    unsigned char buffer[NBUF][BUFFER_SIZE];	/* ���������� */
    unordered_map<int, Buffer*> map;
    DeviceDriver* deviceDriver;

public:
    BufferManager();
    ~BufferManager();

    Buffer* GetBlk(int blkno);	/* ����һ�黺�棬���ڶ�д�豸�ϵĿ�blkno��*/
    void Brelse(Buffer* bp);				/* �ͷŻ�����ƿ�buf */

    Buffer* Bread(int blkno);	/* ��һ�����̿飬blknoΪĿ����̿��߼���š� */
    void Bwrite(Buffer* bp);				/* дһ�����̿� */
    void Bdwrite(Buffer* bp);				/* �ӳ�д���̿� */

    //void Bclear(Buffer* bp);				/* ��ջ��������� */
    //void Bflush();				/* ���������ӳ�д�Ļ���ȫ����������� */
    //Buffer& GetFreeBuffer();				/* ��ȡ���п��ƿ�Buf�������� */
	void ClrBuf(Buffer *bp);
	
private:
    void InitList();
    void DetachNode(Buffer* pb);
    void InsertTail(Buffer* pb);
};

#endif