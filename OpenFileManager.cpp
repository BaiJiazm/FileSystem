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

/* close�ļ�ʱ�����Iput
*  ��Ҫ���Ĳ������ڴ�i�ڵ����i_count--����Ϊ0���ͷ��ڴ�i�ڵ㡢���иĶ�д�ش���
*  �����ļ�;��������Ŀ¼�ļ������������󶼻�Iput���ڴ�i�ڵ㡣·�����ĵ�����2��·������һ���Ǹ�
*  Ŀ¼�ļ�������������д������ļ�������ɾ��һ�������ļ���������������д���ɾ����Ŀ¼����ô
*  ���뽫���Ŀ¼�ļ�����Ӧ���ڴ� i�ڵ�д�ش��̡�
*  ���Ŀ¼�ļ������Ƿ��������ģ����Ǳ��뽫����i�ڵ�д�ش��̡�
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