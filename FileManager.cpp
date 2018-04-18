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

/* ����NULL��ʾĿ¼����ʧ�ܣ������Ǹ�ָ�룬ָ���ļ����ڴ��i�ڵ� ���������ڴ�i�ڵ�  */
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
	
	/* �����ͼ���ĺ�ɾ����ǰĿ¼�ļ������ */
	/*if ('\0' == curchar && mode != FileManager::OPEN)
	{
		u.u_error = User::ENOENT;
		goto out;
	}*/

	/* ���ѭ��ÿ�δ���pathname��һ��·������ */
	while (1) {
		if (u.u_error != User::U_NOERROR) {
			break;
		}
		if (index++ >= u.u_dirp.length()) {
			return pINode;
		}

		/* ���Ҫ���������Ĳ���Ŀ¼�ļ����ͷ����Inode��Դ���˳� */
		if ((pINode->i_mode&INode::IFMT) != INode::IFDIR) {
			u.u_error = User::U_ENOTDIR;
			break;
		}
		nindex = u.u_dirp.find_first_of('/', index);
		memset(u.u_dbuf, 0, sizeof(u.u_dbuf));
		memcpy(u.u_dbuf, u.u_dirp.data() + index, nindex - index);
		index = nindex;

		/* �ڲ�ѭ�����ֶ���u.u_dbuf[]�е�·���������������Ѱƥ���Ŀ¼�� */
		u.u_IOParam.m_Offset = 0;
		/* ����ΪĿ¼����� �����հ׵�Ŀ¼��*/
		u.u_IOParam.m_Count = pINode->i_size / sizeof(DirectoryEntry);
		freeEntryOffset = 0;
		pBuffer = NULL;

		/* ��һ��Ŀ¼��Ѱ�� */
		while (1) {
			/* ��Ŀ¼���Ѿ�������� */
			if (0 == u.u_IOParam.m_Count) {
				if (NULL != pBuffer) {
					bufferManager.Brelse(pBuffer);
				}
				/* ����Ǵ������ļ� */
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

			/* �Ѷ���Ŀ¼�ļ��ĵ�ǰ�̿飬��Ҫ������һĿ¼�������̿� */
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

			/* ����ǿ���Ŀ¼���¼����λ��Ŀ¼�ļ���ƫ���� */
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

		/* �����ɾ���������򷵻ظ�Ŀ¼Inode����Ҫɾ���ļ���Inode����u.u_dent.m_ino�� */
		if (FileManager::DELETE == mode && index >= u.u_dirp.length()) {
			return pINode;
		}

		/*
		* ƥ��Ŀ¼��ɹ������ͷŵ�ǰĿ¼Inode������ƥ��ɹ���
		* Ŀ¼��m_ino�ֶλ�ȡ��Ӧ��һ��Ŀ¼���ļ���Inode��
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
* trf == 0��open����
* trf == 1��creat���ã�creat�ļ���ʱ��������ͬ�ļ������ļ�
* trf == 2��creat���ã�creat�ļ���ʱ��δ������ͬ�ļ������ļ��������ļ�����ʱ��һ������
* mode���������ļ�ģʽ����ʾ�ļ������� ����д���Ƕ�д
*/
void FileManager::Open1(INode* pINode, int mode, int trf) {
	User& u = g_User;

	/* ��creat�ļ���ʱ��������ͬ�ļ������ļ����ͷŸ��ļ���ռ�ݵ������̿� */
	if (1 == trf) {
		pINode->ITrunc();
	}

	/* ������ļ����ƿ�File�ṹ */
	File* pFile = this->m_OpenFileTable->FAlloc();
	if (NULL == pFile) {
		this->m_INodeTable->IPut(pINode);
		return;
	}
	/* ���ô��ļ���ʽ������File�ṹ���ڴ�Inode�Ĺ�����ϵ */
	pFile->f_flag = mode & (File::FREAD | File::FWRITE);
	pFile->f_inode = pINode;

}

INode* FileManager::MakNode(unsigned int mode) {
	INode* pINode;
	User& u = g_User;

	/* ����һ������DiskInode������������ȫ����� */
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