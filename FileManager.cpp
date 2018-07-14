#include "Utility.h"
#include "FileManager.h"
#include "BufferManager.h"
#include "User.h"

extern BufferManager g_BufferManager;
extern FileSystem g_FileSystem;
extern INodeTable g_INodeTable;
extern OpenFileTable g_OpenFileTable;
extern User g_User;

FileManager::FileManager() {
    fileSystem = &g_FileSystem;
    openFileTable = &g_OpenFileTable;
    inodeTable = &g_INodeTable;
    rootDirINode = inodeTable->IGet(FileSystem::ROOT_INODE_NO);
    rootDirINode->i_count += 0xff;
	//rootDirINode = NULL;
}

FileManager::~FileManager() {

}

/*
* 功能：打开文件
* 效果：建立打开文件结构，内存i节点开锁 、i_count 为正数（i_count ++）
* */
void FileManager::Open() {
    User &u = g_User;
    INode* pINode;
    pINode = this->NameI(FileManager::OPEN);
    if (NULL == pINode) {
        return;
    }
    this->Open1(pINode, u.arg[1], 0);
}

void FileManager::Creat() {
    INode* pINode;
    User& u = g_User;
    //unsigned int newACCMode = u.arg[1] & (INode::IRWXU | INode::IRWXG | INode::IRWXO);
    unsigned int newACCMode = u.arg[1];

    /* 搜索目录的模式为1，表示创建；若父目录不可写，出错返回 */
    pINode = this->NameI(FileManager::CREATE);

    /* 没有找到相应的INode，或NameI出错 */
    if (NULL == pINode) {
        if (u.u_error)
            return;
        pINode = this->MakNode(newACCMode);
        if (NULL == pINode)
            return;

        /* 如果创建的名字不存在，使用参数trf = 2来调用open1()。*/
        this->Open1(pINode, File::FWRITE, 2);
        return;
    }

    /* 如果NameI()搜索到已经存在要创建的文件，则清空该文件（用算法ITrunc()）*/
    this->Open1(pINode, File::FWRITE, 1);
    pINode->i_mode |= newACCMode;
}

/* 返回NULL表示目录搜索失败，未找到u.dirp中指定目录全路径
 * 否则是根指针，指向文件的内存打开i节点
 */
INode* FileManager::NameI(enum DirectorySearchMode mode) {
    //if (NULL == rootDirINode) {
    //    rootDirINode = inodeTable->IGet(FileSystem::ROOT_INODE_NO);
    //    //rootDirINode多加一次，始终存在以免被IPut
    //    rootDirINode->i_count++;
    //    //open file table ?
    //    return rootDirINode;
    //}

    BufferManager& bufferManager = g_BufferManager;
    User &u = g_User;
    INode* pINode = u.cdir;
    Buffer* pBuffer;
    int freeEntryOffset;
    unsigned int index = 0, nindex = 0;

    if ('/' == u.dirp[0]) {
        nindex = ++index + 1;
        pINode = this->rootDirINode;
    }

    /* 如果试图更改和删除当前目录文件则出错 */
    /*if ('\0' == curchar && mode != FileManager::OPEN)
    {
        u.u_error = User::ENOENT;
        goto out;
    }*/

    /* 外层循环每次处理pathname中一段路径分量 */
    while (1) {
        if (u.u_error != User::U_NOERROR) {
            break;
        }
        if (nindex >= u.dirp.length()) {
            return pINode;
        }

        /* 如果要进行搜索的不是目录文件，释放相关INode资源则退出 */
        if ((pINode->i_mode&INode::IFMT) != INode::IFDIR) {
            u.u_error = User::U_ENOTDIR;
            break;
        }

        nindex = u.dirp.find_first_of('/', index);
        Utility::memset(u.dbuf, 0, sizeof(u.dbuf));
        Utility::memcpy(u.dbuf, u.dirp.data() + index, (nindex == (unsigned int)string::npos ? u.dirp.length() : nindex) - index);
        index = nindex + 1;

        /* 内层循环部分对于u.dbuf[]中的路径名分量，逐个搜寻匹配的目录项 */
        u.IOParam.offset = 0;
        /* 设置为目录项个数 ，含空白的目录项*/
        u.IOParam.count = pINode->i_size / sizeof(DirectoryEntry);
        freeEntryOffset = 0;
        pBuffer = NULL;

        /* 在一个目录下寻找 */
        while (1) {

            /* 对目录项已经搜索完毕 */
            if (0 == u.IOParam.count) {
                if (NULL != pBuffer) {
                    bufferManager.Brelse(pBuffer);
                }

                /* 如果是创建新文件 */
                if (FileManager::CREATE == mode && nindex >= u.dirp.length()) {
                    u.pdir = pINode;
                    if (freeEntryOffset) {
                        u.IOParam.offset = freeEntryOffset - sizeof(DirectoryEntry);
                    }
                    else {
                        pINode->i_flag |= INode::IUPD;
                    }
                    return NULL;
                }
                u.u_error = User::U_ENOENT;
                goto out;
            }

            /* 已读完目录文件的当前盘块，需要读入下一目录项数据盘块 */
            if (0 == u.IOParam.offset%INode::BLOCK_SIZE) {
                if (pBuffer) {
                    bufferManager.Brelse(pBuffer);
                }
                int phyBlkno = pINode->Bmap(u.IOParam.offset / INode::BLOCK_SIZE);
                pBuffer = bufferManager.Bread(phyBlkno);
                //pBuffer->debug();
            }

            Utility::memcpy(&u.dent, pBuffer->addr + (u.IOParam.offset % INode::BLOCK_SIZE), sizeof(u.dent));
            u.IOParam.offset += sizeof(DirectoryEntry);
            u.IOParam.count--;

            /* 如果是空闲目录项，记录该项位于目录文件中偏移量 */
            if (0 == u.dent.m_ino) {
                if (0 == freeEntryOffset) {
                    freeEntryOffset = u.IOParam.offset;
                }
                continue;
            }

            if (!Utility::memcmp(u.dbuf, &u.dent.name, sizeof(DirectoryEntry) - 4)) {
                break;
            }
        }

        if (pBuffer) {
            bufferManager.Brelse(pBuffer);
        }

        /* 如果是删除操作，则返回父目录INode，而要删除文件的INode号在u.dent.m_ino中 */
        if (FileManager::DELETE == mode && nindex >= u.dirp.length()) {
            return pINode;
        }

        /*
        * 匹配目录项成功，则释放当前目录INode，根据匹配成功的
        * 目录项m_ino字段获取相应下一级目录或文件的INode。
        */
        this->inodeTable->IPut(pINode);
        pINode = this->inodeTable->IGet(u.dent.m_ino);

        if (NULL == pINode) {
            return NULL;
        }
    }

out:
    this->inodeTable->IPut(pINode);
    return NULL;
}

/*
* trf == 0由open调用
* trf == 1由creat调用，creat文件的时候搜索到同文件名的文件
* trf == 2由creat调用，creat文件的时候未搜索到同文件名的文件，这是文件创建时更一般的情况
* mode参数：打开文件模式，表示文件操作是 读、写还是读写
*/
void FileManager::Open1(INode* pINode, int mode, int trf) {
    User& u = g_User;

    /* 在creat文件的时候搜索到同文件名的文件，释放该文件所占据的所有盘块 */
    if (1 == trf) {
        pINode->ITrunc();
    }

    /* 分配打开文件控制块File结构 */
    File* pFile = this->openFileTable->FAlloc();
    if (NULL == pFile) {
        this->inodeTable->IPut(pINode);
        return;
    }

    /* 设置打开文件方式，建立File结构和内存INode的勾连关系 */
    pFile->flag = mode & (File::FREAD | File::FWRITE);
    pFile->inode = pINode;

    /* 为打开或者创建文件的各种资源都已成功分配，函数返回 */
    if (u.u_error == 0) {
        return;
    }
    else {  /* 如果出错则释放资源 */
        /* 释放打开文件描述符 */
        int fd = u.ar0[User::EAX];
        if (fd != -1) {
            u.ofiles.SetF(fd, NULL);
            /* 递减File结构和INode的引用计数 ,File结构没有锁 f_count为0就是释放File结构了*/
            pFile->count--;
        }
        this->inodeTable->IPut(pINode);
    }
}

/* 由creat调用。
 * 为新创建的文件写新的i节点和父目录中新的目录项(相应参数在User结构中)
 * 返回的pINode是上了锁的内存i节点，其中的i_count是 1。
 */
INode* FileManager::MakNode(unsigned int mode) {
	INode* pINode;
	User& u = g_User;

	/* 分配一个空闲DiskInode，里面内容已全部清空 */
	pINode = this->fileSystem->IAlloc();
	if (NULL == pINode) {
		return NULL;
	}

	pINode->i_flag |= (INode::IACC | INode::IUPD);
	pINode->i_mode = mode | INode::IALLOC;
	pINode->i_nlink = 1;

    /* 将目录项写入u.u_u_dent，随后写入目录文件 */
	this->WriteDir(pINode);
	return pINode;
}

/* 由creat子子调用。
 * 把属于自己的目录项写进父目录，修改父目录文件的i节点 、将其写回磁盘。
 */
void FileManager::WriteDir(INode* pINode) {
    User& u = g_User;
    //cout << "i_number=" << pINode->i_number << endl;
    /* 设置目录项中INode编号部分 */
    u.dent.m_ino = pINode->i_number;

    /* 设置目录项中pathname分量部分 */
    Utility::memcpy(u.dent.name, u.dbuf, DirectoryEntry::DIRSIZ);

    u.IOParam.count = DirectoryEntry::DIRSIZ + 4;
    u.IOParam.base = (unsigned char *)&u.dent;

    /* 将目录项写入父目录文件 */
    u.pdir->WriteI();
    this->inodeTable->IPut(u.pdir);
}

void FileManager::Close() {
    User& u = g_User;
    int fd = u.arg[0];

    /* 获取打开文件控制块File结构 */
    File* pFile = u.ofiles.GetF(fd);
    if (NULL == pFile) {
        return;
    }

    /* 释放打开文件描述符fd，递减File结构引用计数 */
    u.ofiles.SetF(fd, NULL);
    this->openFileTable->CloseF(pFile);
}

void FileManager::UnLink() {
    //注意删除文件夹有磁盘泄露
    INode* pINode;
    INode* pDeleteINode;
    User& u = g_User;

    pDeleteINode = this->NameI(FileManager::DELETE);
    if (NULL == pDeleteINode) {
        return;
    }

    pINode = this->inodeTable->IGet(u.dent.m_ino);
    if (NULL == pINode) {
        return;
    }

    /* 写入清零后的目录项 */
    u.IOParam.offset -= (DirectoryEntry::DIRSIZ + 4);
    u.IOParam.base = (unsigned char *)&u.dent;
    u.IOParam.count = DirectoryEntry::DIRSIZ + 4;

    u.dent.m_ino = 0;
    pDeleteINode->WriteI();

    /* 修改inode项 */
    pINode->i_nlink--;
    pINode->i_flag |= INode::IUPD;

    this->inodeTable->IPut(pDeleteINode);
    this->inodeTable->IPut(pINode);
}

void FileManager::Seek() {
    File* pFile;
    User& u = g_User;
    int fd = u.arg[0];

    pFile = u.ofiles.GetF(fd);
    if (NULL == pFile) {
        return;  /* 若FILE不存在，GetF有设出错码 */
    }

    int offset = u.arg[1];

    switch (u.arg[2]) {
        /* 读写位置设置为offset */
    case 0:
        pFile->offset = offset;
        break;
        /* 读写位置加offset(可正可负) */
    case 1:
        pFile->offset += offset;
        break;
        /* 读写位置调整为文件长度加offset */
    case 2:
        pFile->offset = pFile->inode->i_size + offset;
        break;
    default:
        cout << " origin " << u.arg[2] << " is undefined ! \n";
        break;
    }
}

void FileManager::Read() {
    /* 直接调用Rdwr()函数即可 */
    this->Rdwr(File::FREAD);
}

void FileManager::Write() {
    /* 直接调用Rdwr()函数即可 */
    this->Rdwr(File::FWRITE);
}

void FileManager::Rdwr(enum File::FileFlags mode) {
    File* pFile;
    User& u = g_User;

    /* 根据Read()/Write()的系统调用参数fd获取打开文件控制块结构 */
    pFile = u.ofiles.GetF(u.arg[0]);	/* fd */
    if (NULL == pFile) {
        /* 不存在该打开文件，GetF已经设置过出错码，所以这里不需要再设置了 */
        /*	u.u_error = User::EBADF;	*/
        return;
    }

    /* 读写的模式不正确 */
    if ((pFile->flag & mode) == 0) {
        u.u_error = User::U_EACCES;
        return;
    }

    u.IOParam.base = (unsigned char *)u.arg[1];     /* 目标缓冲区首址 */
    u.IOParam.count = u.arg[2];		/* 要求读/写的字节数 */

    u.IOParam.offset = pFile->offset;   /* 设置文件起始读位置 */
    if (File::FREAD == mode) {
        pFile->inode->ReadI();
    }
    else {
        pFile->inode->WriteI();
    }

    /* 根据读写字数，移动文件读写偏移指针 */
    pFile->offset += (u.arg[2] - u.IOParam.count);

    /* 返回实际读写的字节数，修改存放系统调用返回值的核心栈单元 */
    u.ar0[User::EAX] = u.arg[2] - u.IOParam.count;
}

void FileManager::Ls() {
    User &u = g_User;
    BufferManager& bufferManager = g_BufferManager;

    INode* pINode = u.cdir;
    Buffer* pBuffer = NULL;
    u.IOParam.offset = 0;
    u.IOParam.count = pINode->i_size / sizeof(DirectoryEntry);

    while (u.IOParam.count) {
        if (0 == u.IOParam.offset%INode::BLOCK_SIZE) {
            if (pBuffer) {
                bufferManager.Brelse(pBuffer);
            }
            int phyBlkno = pINode->Bmap(u.IOParam.offset / INode::BLOCK_SIZE);
            pBuffer = bufferManager.Bread(phyBlkno);
        }
        Utility::memcpy(&u.dent, pBuffer->addr+ (u.IOParam.offset % INode::BLOCK_SIZE), sizeof(u.dent));
        u.IOParam.offset += sizeof(DirectoryEntry);
        u.IOParam.count--;

        if (0 == u.dent.m_ino)
            continue;
        u.ls += u.dent.name;
        u.ls += "\n";
    }

    if (pBuffer) {
        bufferManager.Brelse(pBuffer);
    }
}

/* 改变当前工作目录 */
void FileManager::ChDir() {
    INode* pINode;
    User& u = g_User;

    pINode = this->NameI(FileManager::OPEN);
    if (NULL == pINode) {
        return;
    }

    /* 搜索到的文件不是目录文件 */
    if ((pINode->i_mode & INode::IFMT) != INode::IFDIR) {
        u.u_error = User::U_ENOTDIR;
        this->inodeTable->IPut(pINode);
        return;
    }

    u.cdir = pINode;
    /* 路径不是从根目录'/'开始，则在现有u.u_curdir后面加上当前路径分量 */
    if (u.dirp[0] != '/') {
        u.curDirPath += u.dirp;
    }
    else {
        /* 如果是从根目录'/'开始，则取代原有工作目录 */
        u.curDirPath = u.dirp;
    }
    if (u.curDirPath.back() != '/')
        u.curDirPath.push_back('/');
}