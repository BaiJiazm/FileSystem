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
    static const int BUFFER_SIZE = 512;		/* ��������С�� ���ֽ�Ϊ��λ */

private:
    Buffer* bufferList;							/* ������п��ƿ� */
    Buffer nBuffer[NBUF];						/* ������ƿ����� */
    unsigned char buffer[NBUF][BUFFER_SIZE];	/* ���������� */
    unordered_map<int, Buffer*> map;
    DeviceDriver* deviceDriver;

public:
    BufferManager();
    ~BufferManager();

	/* ����һ�黺�棬���ڶ�д�豸�ϵĿ�blkno��*/
    Buffer* GetBlk(int blkno);
    
	/* �ͷŻ�����ƿ�buf */
	void Brelse(Buffer* bp);

	/* ��һ�����̿飬blknoΪĿ����̿��߼���š� */
    Buffer* Bread(int blkno);

	/* дһ�����̿� */
    void Bwrite(Buffer* bp);
    
	/* �ӳ�д���̿� */
	void Bdwrite(Buffer* bp);

	/* ��ջ��������� */
    void Bclear(Buffer* bp);
    
	/* ���������ӳ�д�Ļ���ȫ����������� */
	void Bflush();
    
    /* ��ȡ���п��ƿ�Buf�������� */
	//Buffer& GetFreeBuffer();				
	
    /* ��ʽ������Buffer */
    void FormatBuffer();

private:
    void InitList();
    void DetachNode(Buffer* pb);
    void InsertTail(Buffer* pb);
    void debug();
};

#endif