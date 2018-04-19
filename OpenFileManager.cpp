#include "Utility.h"
#include "OpenFileManager.h"
#include "User.h"

extern BufferManager g_BufferManager;
extern FileSystem g_FileSystem;
extern INodeTable g_INodeTable;
extern User g_User;

OpenFileTable::OpenFileTable() {
}

OpenFileTable::~OpenFileTable() {
}

File* OpenFileTable::FAlloc() {
	User& u = g_User;
	int fd = u.u_ofiles.AllocFreeSlot();
	if (fd < 0) {
		return NULL;
	}
	for (int i = 0; i < OpenFileTable::MAX_FILES; ++i) {
		/* f_count==0表示该项空闲 */
		if (this->m_File[i].f_count == 0) {
			u.u_ofiles.SetF(fd, &this->m_File[i]);
			this->m_File[i].f_count++;
			this->m_File[i].f_offset = 0;
			return (&this->m_File[i]);
		}
	}
	cout << "No Free File Struct\n";
	u.u_error = User::U_ENFILE;
	return NULL;
}

/* 对打开文件控制块File结构的引用计数f_count减1，若引用计数f_count为0，则释放File结构。*/
void OpenFileTable::CloseF(File* pFile) {
	pFile->f_count--;
	if (pFile->f_count <= 0) {
		g_INodeTable.IPut(pFile->f_inode);
	}
}

INodeTable::INodeTable() {
    m_FileSystem = &g_FileSystem;
}

INodeTable::~INodeTable() {

}

/*
 * 检查编号为inumber的外存INode是否有内存拷贝，
 * 如果有则返回该内存INode在内存INode表中的索引
 */
int INodeTable::IsLoaded(int inumber) {
    for (int i = 0; i < NINODE; ++i) {
        if (m_INode[i].i_number == inumber&&m_INode[i].i_count) {
            return i;
        }
    }
    return -1;
}

/* 在内存INode表中寻找一个空闲的内存INode */
INode* INodeTable::GetFreeINode() {
    for (int i = 0; i < INodeTable::NINODE; i++) {
        if (this->m_INode[i].i_count == 0) {
            return m_INode + i;
        }
    }
    return NULL;
}

/*
 * 根据外存INode编号获取对应INode。如果该INode已经在内存中，返回该内存INode；
 * 如果不在内存中，则将其读入内存后上锁并返回该内存INode
 */
INode* INodeTable::IGet(int inumber) {
    INode* pINode;
    int index = IsLoaded(inumber);
    if (index >= 0) {
        pINode = m_INode + index;
        ++pINode->i_count;
        return pINode;
    }

    pINode = GetFreeINode();
    if (NULL == pINode) {
        cout << "INode Table Overflow !" << endl;
        g_User.u_error = User::U_ENFILE;
        return NULL;
    }

    pINode->i_number = inumber;
    pINode->i_count++;
    BufferManager& bmg = g_BufferManager;
    Buffer* pBuf = bmg.Bread(FileSystem::INODE_ZONE_START_SECTOR + inumber / FileSystem::INODE_NUMBER_PER_SECTOR);
    pINode->ICopy(pBuf, inumber);
    return pINode;
}

/*
 * 减少该内存INode的引用计数，如果此INode已经没有目录项指向它，
 * 且无进程引用该INode，则释放此文件占用的磁盘块。
 */
void INodeTable::IPut(INode* pINode) {
	if (pINode->i_count == 1) {
		if (pINode->i_nlink <= 0) {
			pINode->ITrunc();
			pINode->i_mode = 0;
			this->m_FileSystem->IFree(pINode->i_number);
		}
		pINode->IUpdate(Utility::time(NULL));
		pINode->i_flag = 0;
		/* 这是内存inode空闲的标志之一，另一个是i_count == 0 */
		pINode->i_number = -1;
	}
	pINode->i_count--;
}

/* 将所有被修改过的内存INode更新到对应外存INode中 */
void INodeTable::UpdateINodeTable() {
	for (int i = 0; i < INodeTable::NINODE; ++i) {
		if (this->m_INode[i].i_count) {
			this->m_INode[i].IUpdate(time(NULL));
		}
	}
}