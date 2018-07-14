#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "INode.h"
#include "DeviceDriver.h"
#include "BufferManager.h"

/*
 * �ļ�ϵͳ�洢��Դ�����(Super Block)�Ķ��塣
 */
class SuperBlock {
public:
    const static int MAX_NFREE = 100;
    const static int MAX_NINODE = 100;

public:
    int	s_isize;		        // ���INode��ռ�õ��̿���
    int	s_fsize;		        // �̿�����

    int	s_nfree;		        // ֱ�ӹ���Ŀ����̿�����
    int	s_free[MAX_NFREE];	    // ֱ�ӹ���Ŀ����̿�������

    int	s_ninode;		        // ֱ�ӹ���Ŀ������INode����
    int	s_inode[MAX_NINODE];	// ֱ�ӹ���Ŀ������INode������

    int	s_flock;		        // ���������̿��������־
    int	s_ilock;		        // ��������INode���־

    int	s_fmod;			        // �ڴ���super block�������޸ı�־����ζ����Ҫ��������Ӧ��Super Block
    int	s_ronly;		        // ���ļ�ϵͳֻ�ܶ���
    int	s_time;			        // ���һ�θ���ʱ��
    int	padding[47];	        // ���ʹSuperBlock���С����1024�ֽڣ�ռ��2������
};

class DirectoryEntry {
public:
    static const int DIRSIZ = 28;	/* Ŀ¼����·�����ֵ�����ַ������� */

public:
    int m_ino;		    /* Ŀ¼����INode��Ų��� */
    char name[DIRSIZ];	/* Ŀ¼����·�������� */
};

class FileSystem {
public:
    // Block���С
    static const int BLOCK_SIZE = 512;

    // ����������������
    static const int DISK_SIZE = 16384;

    // ����SuperBlockλ�ڴ����ϵ������ţ�ռ����������
    static const int SUPERBLOCK_START_SECTOR = 0;


    // ���INode��λ�ڴ����ϵ���ʼ������
    static const int INODE_ZONE_START_SECTOR = 2;

    // ���������INode��ռ�ݵ�������
    static const int INODE_ZONE_SIZE = 1022;

    // ���INode���󳤶�Ϊ64�ֽڣ�ÿ�����̿���Դ��512/64 = 8�����INode
    static const int INODE_NUMBER_PER_SECTOR = BLOCK_SIZE / sizeof(DiskINode);

    // �ļ�ϵͳ��Ŀ¼���INode���
    static const int ROOT_INODE_NO = 0;

    // ���INode���ܸ���
    static const int INode_NUMBERS = INODE_ZONE_SIZE* INODE_NUMBER_PER_SECTOR;


    // ����������ʼ������
    static const int DATA_ZONE_START_SECTOR = INODE_ZONE_START_SECTOR + INODE_ZONE_SIZE;

    // �����������������
    static const int DATA_ZONE_END_SECTOR = DISK_SIZE - 1;

    // ������ռ�ݵ���������
    static const int DATA_ZONE_SIZE = DISK_SIZE - DATA_ZONE_START_SECTOR;

public:
    DeviceDriver* deviceDriver;
    SuperBlock* superBlock;
	BufferManager* bufferManager;

public:
    FileSystem();
    ~FileSystem();

	/* ��ʽ��SuperBlock */
    void FormatSuperBlock();

	/* ��ʽ�������ļ�ϵͳ */
    void FormatDevice();

    /* ϵͳ��ʼ��ʱ����SuperBlock */
    void LoadSuperBlock();

    /* ��SuperBlock������ڴ渱�����µ��洢�豸��SuperBlock��ȥ */
    void Update();

    /* �ڴ洢�豸dev�Ϸ���һ���������INode��һ�����ڴ����µ��ļ���*/
    INode* IAlloc();

    /* �ͷű��Ϊnumber�����INode��һ������ɾ���ļ���*/
    void IFree(int number);

    /* �ڴ洢�豸�Ϸ�����д��̿� */
    Buffer* Alloc();

    /* �ͷŴ洢�豸dev�ϱ��Ϊblkno�Ĵ��̿� */
    void Free(int blkno);

};

#endif