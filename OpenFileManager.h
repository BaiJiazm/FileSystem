#ifndef OPENFILEMANAGER_H
#define OPENFILEMANAGER_H

#include "File.h"
#include "FileSystem.h"

/*
 * ���ļ�������(OpenFileManager)�����ں��жԴ��ļ������Ĺ���Ϊ���̴��ļ�������
 * �����ݽṹ֮��Ĺ�����ϵ��
 * ������ϵָ����u���д��ļ�������ָ����ļ����е�File���ļ����ƽṹ���Լ���File
 * �ṹָ���ļ���Ӧ���ڴ�INode��
 */
class OpenFileTable {
public:
    static const int MAX_FILES = 100;	/* ���ļ����ƿ�File�ṹ������ */

public:
	/* ϵͳ���ļ���Ϊ���н��̹������̴��ļ���������
	 * �а���ָ����ļ����ж�ӦFile�ṹ��ָ�롣
	 */
    File m_File[MAX_FILES];	

public:
	OpenFileTable();
	~OpenFileTable();

	/* ��ϵͳ���ļ����з���һ�����е�File�ṹ */
    File* FAlloc();

    /* �Դ��ļ����ƿ�File�ṹ�����ü���count��1�������ü���countΪ0�����ͷ�File�ṹ��*/
    void CloseF(File* pFile);

    void Format();
};

/*
 * �ڴ�INode��(class INodeTable)
 * �����ڴ�INode�ķ�����ͷš�
 */
class INodeTable {
public:
    static const int NINODE = 100;	/* �ڴ�INode������ */

private:
    INode m_INode[NINODE];		/* �ڴ�INode���飬ÿ�����ļ�����ռ��һ���ڴ�INode */
    FileSystem* fileSystem;	/* ��ȫ�ֶ���g_FileSystem������ */

public:
    INodeTable();
    ~INodeTable();

    /*
     * �������INode��Ż�ȡ��ӦINode�������INode�Ѿ����ڴ��У����ظ��ڴ�INode��
     * ��������ڴ��У���������ڴ�����������ظ��ڴ�INode
     */
    INode* IGet(int inumber);
    
    /*
     * ���ٸ��ڴ�INode�����ü����������INode�Ѿ�û��Ŀ¼��ָ������
     * ���޽������ø�INode�����ͷŴ��ļ�ռ�õĴ��̿顣
     */
    void IPut(INode* pNode);

    /* �����б��޸Ĺ����ڴ�INode���µ���Ӧ���INode�� */
    void UpdateINodeTable();

    /*
     * �����Ϊinumber�����INode�Ƿ����ڴ濽����
     * ������򷵻ظ��ڴ�INode���ڴ�INode���е�����
     */
    int IsLoaded(int inumber);
    
    /* ���ڴ�INode����Ѱ��һ�����е��ڴ�INode */
    INode* GetFreeINode();

    void Format();
};

#endif