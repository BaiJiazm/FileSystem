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
		/* f_count==0��ʾ������� */
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

/* �Դ��ļ����ƿ�File�ṹ�����ü���f_count��1�������ü���f_countΪ0�����ͷ�File�ṹ��*/
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
 * �����Ϊinumber�����INode�Ƿ����ڴ濽����
 * ������򷵻ظ��ڴ�INode���ڴ�INode���е�����
 */
int INodeTable::IsLoaded(int inumber) {
    for (int i = 0; i < NINODE; ++i) {
        if (m_INode[i].i_number == inumber&&m_INode[i].i_count) {
            return i;
        }
    }
    return -1;
}

/* ���ڴ�INode����Ѱ��һ�����е��ڴ�INode */
INode* INodeTable::GetFreeINode() {
    for (int i = 0; i < INodeTable::NINODE; i++) {
        if (this->m_INode[i].i_count == 0) {
            return m_INode + i;
        }
    }
    return NULL;
}

/*
 * �������INode��Ż�ȡ��ӦINode�������INode�Ѿ����ڴ��У����ظ��ڴ�INode��
 * ��������ڴ��У���������ڴ�����������ظ��ڴ�INode
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
 * ���ٸ��ڴ�INode�����ü����������INode�Ѿ�û��Ŀ¼��ָ������
 * ���޽������ø�INode�����ͷŴ��ļ�ռ�õĴ��̿顣
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
		/* �����ڴ�inode���еı�־֮һ����һ����i_count == 0 */
		pINode->i_number = -1;
	}
	pINode->i_count--;
}

/* �����б��޸Ĺ����ڴ�INode���µ���Ӧ���INode�� */
void INodeTable::UpdateINodeTable() {
	for (int i = 0; i < INodeTable::NINODE; ++i) {
		if (this->m_INode[i].i_count) {
			this->m_INode[i].IUpdate(time(NULL));
		}
	}
}