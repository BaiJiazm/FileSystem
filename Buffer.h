#ifndef BUFFER_H
#define BUFFER_H

#include <bits/stdc++.h>
using namespace std;

class Buffer {
public:
	
    /* flags�б�־λ */
	enum BufferFlag {
		//B_WRITE = 0x1,		/* д�������������е���Ϣд��Ӳ����ȥ */
		//B_READ = 0x2,		    /* �����������̶�ȡ��Ϣ�������� */
		B_DONE = 0x4,		    /* I/O�������� */
		//B_ERROR = 0x8,		/* I/O��������ֹ */
		//B_BUSY = 0x10,		/* ��Ӧ��������ʹ���� */
		//B_WANTED = 0x20,	    /* �н������ڵȴ�ʹ�ø�buf�������Դ����B_BUSY��־ʱ��Ҫ�������ֽ��� */
		//B_ASYNC = 0x40,		/* �첽I/O������Ҫ�ȴ������ */
		B_DELWRI = 0x80		    /* �ӳ�д������Ӧ����Ҫ��������ʱ���ٽ�������д����Ӧ���豸�� */
	};

public:
	unsigned int flags;	    /* ������ƿ��־λ */

	Buffer*	forw;
	Buffer*	back;

	int		wcount;		    /* �贫�͵��ֽ��� */
	unsigned char* addr;	/* ָ��û�����ƿ�������Ļ��������׵�ַ */
	int		blkno;		    /* �����߼���� */
	int		u_error;	    /* I/O����ʱ��Ϣ */
	int		resid;		    /* I/O����ʱ��δ���͵�ʣ���ֽ��� */
    int no;
public:
	Buffer();
	~Buffer();

    void debugMark();
    void debugContent();
};

#endif
