#include "FileManager.h"
#include "BufferManager.h"
#include "User.h"

extern BufferManager g_BufferManager;
extern FileSystem g_FileSystem;
extern INodeTable g_INodeTable;
extern OpenFileTable g_OpenFileTable;
extern User g_User;

FileManager::FileManager() {
    m_FileSystem = &g_FileSystem;
    m_OpenFileTable = &g_OpenFileTable;
    m_INodeTable = &g_INodeTable;
    //rootDirINode = m_INodeTable->IGet(FileSystem::ROOT_INODE_NO);
	rootDirINode = NULL;
}

FileManager::~FileManager() {

}

void FileManager::Open() {
    INode* pINode;
    pINode = this->NameI(FileManager::OPEN);
    if (NULL == pINode) {
        return;
    }
    this->Open1(pINode, 0, 0);
}

void FileManager::Creat() {
	INode* pINode;
	User& u = g_User;

	pINode = this->NameI(FileManager::CREATE);
	if (NULL == pINode) {
		if (u.u_error)
			return;
		pINode=this->MakNode()
	}
}

/* 返回NULL表示目录搜索失败，否则是根指针，指向文件的内存打开i节点 ，上锁的内存i节点  */
INode* FileManager::NameI(enum DirectorySearchMode mode) {
    if (NULL == rootDirINode) {
        rootDirINode = m_INodeTable->IGet(FileSystem::ROOT_INODE_NO);
		//open file table ?
		return rootDirINode;
    }

	int freeEntryOffset;
	Buffer* pBuffer;
	BufferManager& bufferManager=g_BufferManager;
	User &u = g_User;
	INode* pINode = u.u_cdir;
	unsigned int index = 0, nindex;
	if ('/' == u.u_dirp[0]) {
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
		if (index++ >= u.u_dirp.length()) {
			return pINode;
		}

		/* 如果要进行搜索的不是目录文件，释放相关Inode资源则退出 */
		if ((pINode->i_mode&INode::IFMT) != INode::IFDIR) {
			u.u_error = User::U_ENOTDIR;
			break;
		}
		nindex = u.u_dirp.find_first_of('/', index);
		memset(u.u_dbuf, 0, sizeof(u.u_dbuf));
		memcpy(u.u_dbuf, u.u_dirp.data() + index, nindex - index);
		index = nindex;

		/* 内层循环部分对于u.u_dbuf[]中的路径名分量，逐个搜寻匹配的目录项 */
		u.u_IOParam.m_Offset = 0;
		/* 设置为目录项个数 ，含空白的目录项*/
		u.u_IOParam.m_Count = pINode->i_size / sizeof(DirectoryEntry);
		freeEntryOffset = 0;
		pBuffer = NULL;

		/* 在一个目录下寻找 */
		while (1) {
			/* 对目录项已经搜索完毕 */
			if (0 == u.u_IOParam.m_Count) {
				if (NULL != pBuffer) {
					bufferManager.Brelse(pBuffer);
				}
				/* 如果是创建新文件 */
				if (FileManager::CREATE == mode && index >= u.u_dirp.length()) {
					u.u_pdir = pINode;
					if (freeEntryOffset) {
						u.u_IOParam.m_Offset = freeEntryOffset - sizeof(DirectoryEntry);
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
			if (0 == u.u_IOParam.m_Offset%INode::BLOCK_SIZE) {
				if (pBuffer) {
					bufferManager.Brelse(pBuffer);
				}
				int phyBlkno = pINode->Bmap(u.u_IOParam.m_Offset / INode::BLOCK_SIZE);
				pBuffer = bufferManager.Bread(phyBlkno);
			}
			memcpy(&u.u_dent, pBuffer->addr, sizeof(u.u_dent));
			u.u_IOParam.m_Offset += sizeof(DirectoryEntry);
			u.u_IOParam.m_Count--;

			/* 如果是空闲目录项，记录该项位于目录文件中偏移量 */
			if (0 == u.u_dent.m_ino) {
				if (0 == freeEntryOffset) {
					freeEntryOffset = u.u_IOParam.m_Offset;
				}
				continue;
			}
			if (!memcpy(u.u_dbuf, &u.u_dent, sizeof(DirectoryEntry))) {
				break;
			}
		}

		if (pBuffer) {
			bufferManager.Brelse(pBuffer);
		}

		/* 如果是删除操作，则返回父目录Inode，而要删除文件的Inode号在u.u_dent.m_ino中 */
		if (FileManager::DELETE == mode && index >= u.u_dirp.length()) {
			return pINode;
		}

		/*
		* 匹配目录项成功，则释放当前目录Inode，根据匹配成功的
		* 目录项m_ino字段获取相应下一级目录或文件的Inode。
		*/
		this->m_INodeTable->IPut(pINode);
		pINode = this->m_INodeTable->IGet(u.u_dent.m_ino);

		if (NULL == pINode) {
			return NULL;
		}
	}

out:
	this->m_INodeTable->IPut(pINode);
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
	File* pFile = this->m_OpenFileTable->FAlloc();
	if (NULL == pFile) {
		this->m_INodeTable->IPut(pINode);
		return;
	}
	/* 设置打开文件方式，建立File结构和内存Inode的勾连关系 */
	pFile->f_flag = mode & (File::FREAD | File::FWRITE);
	pFile->f_inode = pINode;

}

INode* FileManager::MakNode(unsigned int mode) {
	INode* pINode;
	User& u = g_User;

	/* 分配一个空闲DiskInode，里面内容已全部清空 */
	pINode = this->m_FileSystem->IAlloc();
	if (NULL == pINode) {
		return NULL;
	}

	pINode->i_flag |= (INode::IACC | INode::IUPD);
	pINode->i_mode = mode | INode::IALLOC;
	pINode->i_nlink = 1;
	this->WriteDir(pINode);
	return pINode;
}

void FileManager::WriteDir(INode* pINode) {

}