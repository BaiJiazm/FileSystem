#include "OpenFileManager.h"
#include "User.h"

extern BufferManager g_BufferManager;
extern FileSystem g_FileSystem;
extern User g_User;

INodeTable::INodeTable() {
    m_FileSystem = &g_FileSystem;
}

INodeTable::~INodeTable() {

}

int INodeTable::IsLoaded(int inumber) {
    for (int i = 0; i < NINODE; ++i) {
        if (m_INode[i].i_number == inumber&&m_INode[i].i_count) {
            return i;
        }
    }
    return -1;
}

INode* INodeTable::GetFreeINode() {
    for (int i = 0; i < INodeTable::NINODE; i++) {
        if (this->m_INode[i].i_count == 0) {
            return m_INode + i;
        }
    }
    return NULL;
}

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

/* close文件时会调用Iput
*  主要做的操作：内存i节点计数i_count--；若为0，释放内存i节点、若有改动写回磁盘
*  搜索文件途径的所有目录文件，搜索经过后都会Iput其内存i节点。路径名的倒数第2个路径分量一定是个
*  目录文件，如果是在其中创建新文件、或是删除一个已有文件；再如果是在其中创建删除子目录。那么
*  必须将这个目录文件所对应的内存 i节点写回磁盘。
*  这个目录文件无论是否经历过更改，我们必须将它的i节点写回磁盘。
* */
void INodeTable::IPut(INode* pINode) {
	if (pINode->i_count == 1) {
		if (pINode->i_nlink <= 0) {
			pINode->ITrunc();
		}
	}
}

void INodeTable::UpdateINodeTable() {
	for (int i = 0; i < INodeTable::NINODE; ++i) {
		if (this->m_INode[i].i_count) {
			this->m_INode[i].IUpdate(time(NULL));
		}
	}
}