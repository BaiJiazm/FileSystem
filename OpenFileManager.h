#ifndef OPENFILEMANAGER_H
#define OPENFILEMANAGER_H

#include "File.h"
#include "FileSystem.h"

/*
* ���ļ�������(OpenFileManager)�����ں��жԴ��ļ������Ĺ�����Ϊ���̴��ļ�������
* �����ݽṹ֮��Ĺ�����ϵ��
* ������ϵָ����u���д��ļ�������ָ����ļ����е�File���ļ����ƽṹ���Լ���File
* �ṹָ���ļ���Ӧ���ڴ�INode��
*/
class OpenFileTable {
public:
    static const int MAX_FILES = 100;	/* ���ļ����ƿ�File�ṹ������ */

public:
    File m_File[MAX_FILES];			/* ϵͳ���ļ�����Ϊ���н��̹��������̴��ļ���������
                                �а���ָ����ļ����ж�ӦFile�ṹ��ָ�롣*/

public:
    /*
    * ��ϵͳ���ļ����з���һ�����е�File�ṹ
    */
    //File* FAlloc();

    /*
    * �Դ��ļ����ƿ�File�ṹ�����ü���f_count��1��
    * �����ü���f_countΪ0�����ͷ�File�ṹ��
    */
    //void CloseF(File* pFile);
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
    FileSystem* m_FileSystem;	/* ��ȫ�ֶ���g_FileSystem������ */

public:
    INodeTable();
    ~INodeTable();

    /*
    * @comment ����ָ���豸��dev�����INode��Ż�ȡ��Ӧ
    * INode�������INode�Ѿ����ڴ��У��������������ظ��ڴ�INode��
    * ��������ڴ��У���������ڴ�����������ظ��ڴ�INode
    */
    INode* IGet(int inumber);
    
    /*
    * @comment ���ٸ��ڴ�INode�����ü����������INode�Ѿ�û��Ŀ¼��ָ������
    * ���޽������ø�INode�����ͷŴ��ļ�ռ�õĴ��̿顣
    */
    void IPut(INode* pNode);

    /*
    * @comment �����б��޸Ĺ����ڴ�INode���µ���Ӧ���INode��
    */
    void UpdateINodeTable();

    /*
    * @comment ����豸dev�ϱ��Ϊinumber�����INode�Ƿ����ڴ濽����
    * ������򷵻ظ��ڴ�INode���ڴ�INode���е�����
    */
    int IsLoaded(int inumber);
    
    /*
    * @comment ���ڴ�INode����Ѱ��һ�����е��ڴ�INode
    */
    INode* GetFreeINode();
};

#endif