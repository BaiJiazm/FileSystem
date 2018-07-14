#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "FileSystem.h"
#include "OpenFileManager.h"

/*
* �ļ�������(FileManager)
* ��װ���ļ�ϵͳ�ĸ���ϵͳ�����ں���̬�´�����̣�
* ����ļ���Open()��Close()��Read()��Write()�ȵ�
* ��װ�˶��ļ�ϵͳ���ʵľ���ϸ�ڡ�
*/
class FileManager
{
public:
    /* Ŀ¼����ģʽ������NameI()���� */
    enum DirectorySearchMode
    {
        OPEN = 0,		/* �Դ��ļ���ʽ����Ŀ¼ */
        CREATE = 1,		/* ���½��ļ���ʽ����Ŀ¼ */
        DELETE = 2		/* ��ɾ���ļ���ʽ����Ŀ¼ */
    };

public:
    /* ��Ŀ¼�ڴ�INode */
    INode* rootDirINode;

    /* ��ȫ�ֶ���g_FileSystem�����ã��ö���������ļ�ϵͳ�洢��Դ */
    FileSystem* fileSystem;

    /* ��ȫ�ֶ���g_INodeTable�����ã��ö������ڴ�INode��Ĺ��� */
    INodeTable* inodeTable;

    /* ��ȫ�ֶ���g_OpenFileTable�����ã��ö�������ļ�����Ĺ��� */
    OpenFileTable* openFileTable;

public:
    FileManager();
    ~FileManager();

    /* Open()ϵͳ���ô������ */
    void Open();

    /* Creat()ϵͳ���ô������ */
    void Creat();

    /* Open()��Creat()ϵͳ���õĹ������� */
    void Open1(INode* pINode, int mode, int trf);

    /* Close()ϵͳ���ô������ */
    void Close();

    /* Seek()ϵͳ���ô������ */
    void Seek();

    /* Read()ϵͳ���ô������ */
    void Read();

    /* Write()ϵͳ���ô������ */
    void Write();

    /* ��дϵͳ���ù������ִ��� */
    void Rdwr(enum File::FileFlags mode);

    /* Ŀ¼��������·��ת��Ϊ��Ӧ��INode�����������INode */
    INode* NameI(enum DirectorySearchMode mode);

    /* ��Creat()ϵͳ����ʹ�ã�����Ϊ�������ļ������ں���Դ */
    INode* MakNode(unsigned int mode);

    /* ȡ���ļ� */
    void UnLink();

    /* ��Ŀ¼��Ŀ¼�ļ�д��һ��Ŀ¼�� */
    void WriteDir(INode* pINode);

    /* �ı��ļ�����ģʽ */
    //void ChMod();

    /* �ı䵱ǰ����Ŀ¼ */
    void ChDir();

    /* �г���ǰINode�ڵ���ļ��� */
    void Ls();
};

#endif