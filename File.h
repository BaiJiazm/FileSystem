#ifndef FILE_H
#define FILE_H

#include "INode.h"

/*
 * ���ļ����ƿ�File�ࡣ
 * �ýṹ��¼�˽��̴��ļ��Ķ���д�������ͣ��ļ���дλ�õȵȡ�
 */
class File {
public:
	enum FileFlags {
		FREAD = 0x1,			/* ���������� */
		FWRITE = 0x2,			/* д�������� */
	};

public:
	File();
	~File();

	unsigned int f_flag;		/* �Դ��ļ��Ķ���д����Ҫ�� */
	int		f_count;			/* ��ǰ���ø��ļ����ƿ�Ľ������� */
	INode*	f_inode;			/* ָ����ļ����ڴ�INodeָ�� */
	int		f_offset;			/* �ļ���дλ��ָ�� */
};

/*
 * ���̴��ļ���������(OpenFiles)�Ķ���
 * ά���˵�ǰ���̵����д��ļ���
 */
class OpenFiles {
public:
    static const int MAX_FILES = 15;		/* ��������򿪵�����ļ��� */

private:
    File *processOpenFileTable[MAX_FILES];	/* File�����ָ�����飬ָ��ϵͳ���ļ����е�File���� */

public:
    OpenFiles();
    ~OpenFiles();

    /* ����������ļ�ʱ���ڴ��ļ����������з���һ�����б��� */
    int AllocFreeSlot();

    /* �����û�ϵͳ�����ṩ���ļ�����������fd���ҵ���Ӧ�Ĵ��ļ����ƿ�File�ṹ */
    File* GetF(int fd);

    /* Ϊ�ѷ��䵽�Ŀ���������fd���ѷ���Ĵ��ļ����п���File������������ϵ */
    void SetF(int fd, File* pFile);
};

/*
 * �ļ�I/O�Ĳ�����
 * ���ļ�����дʱ���õ��Ķ���дƫ�������ֽ����Լ�Ŀ�������׵�ַ������
 */
class IOParameter {
public:
    unsigned char* m_Base;	/* ��ǰ����д�û�Ŀ��������׵�ַ */
    int m_Offset;			/* ��ǰ����д�ļ����ֽ�ƫ���� */
    int m_Count;			/* ��ǰ��ʣ��Ķ���д�ֽ����� */
};

#endif