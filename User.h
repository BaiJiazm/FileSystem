#ifndef  USER_H
#define USER_H

#include "FileManager.h"
#include <string>
using namespace std;

class User {
public:
    static const int EAX = 0;	/* u.ar0[EAX]�������ֳ���������EAX�Ĵ�����ƫ���� */

    enum ErrorCode {
        U_NOERROR = 0,  	/* No u_error */
        //U_EPERM = 1,	    /* Operation not permitted */
        U_ENOENT = 2,	    /* No such file or directory */
        //U_ESRCH = 3,	    /* No such process */
        //U_EINTR = 4,	    /* Interrupted system call */
        //U_EIO = 5,	    /* I/O u_error */
        //U_ENXIO = 6,	    /* No such device or address */
        //U_E2BIG = 7,	    /* Arg list too long */
        //U_ENOEXEC = 8,	/* Exec format u_error */
        U_EBADF = 9,	    /* Bad file number */
        //U_ECHILD = 10,	/* No child processes */
        //U_EAGAIN = 11,	/* Try again */
        //U_ENOMEM = 12,	/* Out of memory */
        U_EACCES = 13,	    /* Permission denied */
        //U_EFAULT = 14,	/* Bad address */
        //U_ENOTBLK = 15,	/* Block device required */
        //U_EBUSY = 16,	    /* Device or resource busy */
        //U_EEXIST = 17,	/* File exists */
        //U_EXDEV = 18,	    /* Cross-device link */
        //U_ENODEV = 19,	/* No such device */
        U_ENOTDIR = 20,	    /* Not a directory */
        //U_EISDIR = 21,	/* Is a directory */
        //U_EINVAL = 22,	/* Invalid argument */
        U_ENFILE = 23,	    /* File table overflow */
        U_EMFILE = 24,	    /* Too many open files */
        //U_ENOTTY = 25,	/* Not a typewriter(terminal) */
        //U_ETXTBSY = 26,	/* Text file busy */
        U_EFBIG = 27,	    /* File too large */
        U_ENOSPC = 28,	    /* No space left on device */
        //U_ESPIPE = 29,	/* Illegal seek */
        //U_EROFS = 30,	    /* Read-only file system */
        //U_EMLINK = 31,	/* Too many links */
        //U_EPIPE = 32,	    /* Broken pipe */
        //U_ENOSYS = 100,
        //EFAULT	= 106
    };

public:
    INode* cdir;		/* ָ��ǰĿ¼��Inodeָ�� */
    INode* pdir;		/* ָ��Ŀ¼��Inodeָ�� */

    DirectoryEntry dent;					/* ��ǰĿ¼��Ŀ¼�� */
    char dbuf[DirectoryEntry::DIRSIZ];	    /* ��ǰ·������ */
    string curDirPath;						/* ��ǰ����Ŀ¼����·�� */

    string dirp;				/* ϵͳ���ò���(һ������Pathname)��ָ�� */
    long arg[5];				/* ��ŵ�ǰϵͳ���ò��� */
                                /* ϵͳ������س�Ա */
    unsigned int	ar0[5];	    /* ָ�����ջ�ֳ���������EAX�Ĵ���
                                    ��ŵ�ջ��Ԫ�����ֶδ�Ÿ�ջ��Ԫ�ĵ�ַ��
                                    ��V6��r0���ϵͳ���õķ���ֵ���û�����
                                    x86ƽ̨��ʹ��EAX��ŷ���ֵ�����u.ar0[R0] */
    ErrorCode u_error;			/* ��Ŵ����� */

    OpenFiles ofiles;		    /* ���̴��ļ������������ */

    IOParameter IOParam;	    /* ��¼��ǰ����д�ļ���ƫ�������û�Ŀ�������ʣ���ֽ������� */

    FileManager* fileManager;

    string ls;

public:
    User();
    ~User();

    void Ls();
    void Cd(string dirName);
    void Mkdir(string dirName);
    void Create(string fileName, string mode);
    void Delete(string fileName);
    void Open(string fileName, string mode);
    void Close(string fd);
    void Seek(string fd, string offset, string origin);
    void Write(string fd, string inFile, string size);
    void Read(string fd, string outFile, string size);
    //void Pwd();
    
private:
    bool IsError();
    void EchoError(enum ErrorCode err);
    int INodeMode(string mode);
    int FileMode(string mode);
    bool checkPathName(string path);
    
};

#endif