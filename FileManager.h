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
    FileSystem* m_FileSystem;

    /* ��ȫ�ֶ���g_INodeTable�����ã��ö������ڴ�INode��Ĺ��� */
    INodeTable* m_INodeTable;

    /* ��ȫ�ֶ���g_OpenFileTable�����ã��ö�������ļ�����Ĺ��� */
    OpenFileTable* m_OpenFileTable;

public:
    FileManager();
    ~FileManager();

    /*
    * @comment Open()ϵͳ���ô������
    */
    void Open();

    /*
    * @comment Creat()ϵͳ���ô������
    */
    void Creat();

    /*
    * @comment Open()��Creat()ϵͳ���õĹ�������
    */
    void Open1(INode* pINode, int mode, int trf);

    /*
    * @comment Close()ϵͳ���ô������
    */
    void Close();

    /*
    * @comment Seek()ϵͳ���ô������
    */
    void Seek();

    /*
    * @comment FStat()��ȡ�ļ���Ϣ
    */
    void FStat();

    /*
    * @comment FStat()��ȡ�ļ���Ϣ
    */
    void Stat();

    /* FStat()��Stat()ϵͳ���õĹ������� */
    void Stat1(INode* pINode, unsigned long statBuf);

    /*
    * @comment Read()ϵͳ���ô������
    */
    void Read();

    /*
    * @comment Write()ϵͳ���ô������
    */
    void Write();

    /*
    * @comment ��дϵͳ���ù������ִ���
    */
    void Rdwr(enum File::FileFlags mode);

    /*
    * @comment Ŀ¼��������·��ת��Ϊ��Ӧ��INode��
    * �����������INode
    */
    INode* NameI(enum DirectorySearchMode mode);

    /*
    * @comment ��Creat()ϵͳ����ʹ�ã�����Ϊ�������ļ������ں���Դ
    */
    INode* MakNode(unsigned int mode);

    /*
    * @comment ��Ŀ¼��Ŀ¼�ļ�д��һ��Ŀ¼��
    */
    void WriteDir(INode* pINode);

    /*
    * @comment ���õ�ǰ����·��
    */
    void SetCurDir(char* pathname);

    /* �ı��ļ�����ģʽ */
    void ChMod();

    /* �ı䵱ǰ����Ŀ¼ */
    void ChDir();
};

#endif