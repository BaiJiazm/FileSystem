#include "Utility.h"
#include "INode.h"
#include "FileSystem.h"
#include "User.h"

extern BufferManager g_BufferManager;
extern FileSystem g_FileSystem;
extern User g_User;

DiskINode::DiskINode() {
    this->d_mode = 0;
    this->d_nlink = 0;
    this->d_uid = -1;
    this->d_gid = -1;
    this->d_size = 0;
	Utility::memset(d_addr, 0, sizeof(d_addr));
    this->d_atime = 0;
    this->d_mtime = 0;
}

DiskINode::~DiskINode() {
}

INode::INode() {
	this->i_flag = 0;
	this->i_mode = 0;
	this->i_count = 0;
	this->i_nlink = 0;
	this->i_number = -1;
	this->i_uid = -1;
	this->i_gid = -1;
	this->i_size = 0;
	this->i_lastr = -1;
	Utility::memset(i_addr, 0, sizeof(i_addr));
}

INode::~INode() {
}

/* ����Inode�����е�������̿���������ȡ��Ӧ���ļ����� */
void INode::ReadI() {
	User& u = g_User;
	BufferManager& bufferManager = g_BufferManager;
	int lbn, bn;
	int offset, nbytes;
	Buffer* pBuffer;

	/* ��Ҫ���ֽ���Ϊ�㣬�򷵻� */
	if (0 == u.u_IOParam.m_Count) {
		return;
	}
	this->i_flag |= INode::IACC;

	while (User::U_NOERROR == u.u_error && u.u_IOParam.m_Count) {
		lbn = bn = u.u_IOParam.m_Offset / INode::BLOCK_SIZE;
		offset = u.u_IOParam.m_Offset % INode::BLOCK_SIZE;
		
		/* ���͵��û������ֽ�������ȡ�������ʣ���ֽ����뵱ǰ�ַ�������Ч�ֽ�����Сֵ */
		nbytes = Utility::min(INode::BLOCK_SIZE - offset /* ������Ч�ֽ��� */, u.u_IOParam.m_Count);
		int remain = this->i_size - u.u_IOParam.m_Offset;
		if (remain <= 0) {
			return;
		}
		
		/* ���͵��ֽ�������ȡ����ʣ���ļ��ĳ��� */
		nbytes = Utility::min(nbytes, remain);
		if ((bn = this->Bmap(lbn)) == 0) {
			return;
		}
		
		pBuffer = bufferManager.Bread(bn);
		this->i_lastr = lbn;

		/* ������������ʼ��λ�� */
		unsigned char* start = pBuffer->addr + offset;
		Utility::memcpy(u.u_IOParam.m_Base, start, nbytes);
		u.u_IOParam.m_Base += nbytes;
		u.u_IOParam.m_Offset += nbytes;
		u.u_IOParam.m_Count -= nbytes;

		bufferManager.Brelse(pBuffer);
	}
}

/* ����Inode�����е�������̿�������������д���ļ� */
void INode::WriteI() {
	int lbn, bn;
	int offset, nbytes;
	Buffer* pBuffer;
	User& u = g_User;
	BufferManager& bufferManager = g_BufferManager;

	this->i_flag |= (INode::IACC | INode::IUPD);

	/* ��Ҫд�ֽ���Ϊ�㣬�򷵻� */
	if (0 == u.u_IOParam.m_Count) {
		return;
	}

	while (User::U_NOERROR == u.u_error && u.u_IOParam.m_Count) {
		lbn = u.u_IOParam.m_Offset / INode::BLOCK_SIZE;
		offset = u.u_IOParam.m_Offset % INode::BLOCK_SIZE;
		nbytes = Utility::min(INode::BLOCK_SIZE - offset, u.u_IOParam.m_Count);
		if ((bn = this->Bmap(lbn)) == 0) {
			return;
		}

		if (INode::BLOCK_SIZE == nbytes) {
			/* ���д������������һ���ַ��飬��Ϊ����仺�� */
			pBuffer = bufferManager.GetBlk(bn);
		}
		else {
			/* д�����ݲ���һ���ַ��飬�ȶ���д���������ַ����Ա�������Ҫ��д�����ݣ� */
			pBuffer = bufferManager.Bread(bn);
		}

		/* ���������ݵ���ʼдλ�� д����: ���û�Ŀ�����������ݵ������� */
		unsigned char* start = pBuffer->addr + offset;
		Utility::memcpy(start, u.u_IOParam.m_Base, nbytes);
		u.u_IOParam.m_Base += nbytes;
		u.u_IOParam.m_Offset += nbytes;
		u.u_IOParam.m_Count -= nbytes;

		if (u.u_error != User::U_NOERROR) {
			bufferManager.Brelse(pBuffer);
		}

		/* ��������Ϊ�ӳ�д�������ڽ���I/O�������ַ�������������� */
		bufferManager.Bdwrite(pBuffer);

		/* ��ͨ�ļ��������� */
		if (this->i_size < u.u_IOParam.m_Offset) {
			this->i_size = u.u_IOParam.m_Offset;
		}
		this->i_flag |= INode::IUPD;
	}
}

/* ���������Inode�ַ�������Ϣ�������ڴ�Inode�� */
void INode::ICopy(Buffer* pb, int inumber) {
    DiskINode& dINode = *(DiskINode*)(pb->addr + (inumber%FileSystem::INODE_NUMBER_PER_SECTOR)*sizeof(DiskINode));
    i_mode = dINode.d_mode;
    i_nlink = dINode.d_nlink;
    i_uid = dINode.d_uid;
    i_gid = dINode.d_gid;
    i_size = dINode.d_size;
    Utility::memcpy(i_addr, dINode.d_addr, sizeof(i_addr));
}

/* ���ļ����߼����ת���ɶ�Ӧ�������̿�� */
int INode::Bmap(int lbn) {
	/*
	* Unix V6++���ļ������ṹ��(С�͡����ͺ;����ļ�)
	* (1) i_addr[0] - i_addr[5]Ϊֱ���������ļ����ȷ�Χ��0 - 6���̿飻
	*
	* (2) i_addr[6] - i_addr[7]���һ�μ�����������ڴ��̿�ţ�ÿ���̿�
	* �ϴ��128���ļ������̿�ţ������ļ����ȷ�Χ��7 - (128 * 2 + 6)���̿飻
	*
	* (3) i_addr[8] - i_addr[9]��Ŷ��μ�����������ڴ��̿�ţ�ÿ�����μ��
	* �������¼128��һ�μ�����������ڴ��̿�ţ������ļ����ȷ�Χ��
	* (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
	*/

	User& u = g_User;
	BufferManager& bufferManager = g_BufferManager;
	FileSystem& fileSystem = g_FileSystem;
	Buffer* pFirstBuffer, *pSecondBuffer;
	int phyBlkno, index;
	int *iTable;

	if (lbn >= INode::HUGE_FILE_BLOCK) {
		u.u_error = User::U_EFBIG;
		return 0;
	}

	/* �����С���ļ����ӻ���������i_addr[0-5]�л�������̿�ż��� */
	if (lbn < 6) {
		phyBlkno = this->i_addr[lbn];

		/* ������߼���Ż�û����Ӧ�������̿����֮��Ӧ�������һ������顣*/
		if (phyBlkno == 0 && (pFirstBuffer = fileSystem.Alloc()) != NULL) {
			bufferManager.Bdwrite(pFirstBuffer);
			phyBlkno = pFirstBuffer->blkno;
			this->i_addr[lbn] = phyBlkno;
			this->i_flag |= INode::IUPD;
		}
		return phyBlkno;
	}

	/* lbn >= 6 ���͡������ļ� */
	if (lbn < INode::LARGE_FILE_BLOCK) {
		index = (lbn - INode::SMALL_FILE_BLOCK) / INode::ADDRESS_PER_INDEX_BLOCK + 6;
	}
	else {
		/* �����ļ�: ���Ƚ���263 - (128 * 128 * 2 + 128 * 2 + 6)���̿�֮�� */
		index = (lbn - INode::LARGE_FILE_BLOCK) / (INode::ADDRESS_PER_INDEX_BLOCK * INode::ADDRESS_PER_INDEX_BLOCK) + 8;
	}

	phyBlkno = this->i_addr[index];
	if (phyBlkno) {
		pFirstBuffer = bufferManager.Bread(phyBlkno);
	}
	else {
		/* ������Ϊ�㣬���ʾ��������Ӧ�ļ��������� */
		this->i_flag |= INode::IUPD;
		if ((pFirstBuffer = fileSystem.Alloc()) == 0) {
			return 0;
		}
		this->i_addr[index] = pFirstBuffer->blkno;
	}

	iTable = (int *)pFirstBuffer->addr;
	if (index >= 8) {
		/*
		* ���ھ����ļ��������pFirstBuf���Ƕ��μ��������
		* ��������߼���ţ����ɶ��μ���������ҵ�һ�μ��������
		*/
		index = ((lbn - INode::LARGE_FILE_BLOCK) / INode::ADDRESS_PER_INDEX_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;
		phyBlkno = iTable[index];

		if (phyBlkno) {
			bufferManager.Brelse(pFirstBuffer);
			pSecondBuffer = bufferManager.Bread(phyBlkno);
		}
		else {
			if ((pSecondBuffer = fileSystem.Alloc()) == NULL) {
				bufferManager.Brelse(pFirstBuffer);
				return 0;
			}
			iTable[index] = pSecondBuffer->blkno;
			bufferManager.Bdwrite(pFirstBuffer);
		}

		pFirstBuffer = pSecondBuffer;
		iTable = (int *)pSecondBuffer->addr;
	}

	if (lbn < INode::LARGE_FILE_BLOCK) {
		index = (lbn - INode::SMALL_FILE_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;
	}
	else {
		index = (lbn - INode::LARGE_FILE_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;
	}

	if ((phyBlkno = iTable[index]) == 0 && (pSecondBuffer = fileSystem.Alloc()) != NULL) {
		phyBlkno = pSecondBuffer->blkno;
		iTable[index] = phyBlkno;
		bufferManager.Bdwrite(pSecondBuffer);
		bufferManager.Bdwrite(pFirstBuffer);
	}
	else {
		bufferManager.Brelse(pFirstBuffer);
	}
	return phyBlkno;
}

/* ���Inode�����е����� */
void INode::Clean() {
	/*
	* Inode::Clean()�ض�����IAlloc()������·���DiskInode��ԭ�����ݣ�
	* �����ļ���Ϣ��Clean()�����в�Ӧ�����i_dev, i_number, i_flag, i_count,
	* ���������ڴ�Inode����DiskInode�����ľ��ļ���Ϣ����Inode�๹�캯����Ҫ
	* �����ʼ��Ϊ��Чֵ��
	*/

	// this->i_flag = 0;
	this->i_mode = 0;
	//this->i_count = 0;
	this->i_nlink = 0;
	//this->i_dev = -1;
	//this->i_number = -1;
	this->i_uid = -1;
	this->i_gid = -1;
	this->i_size = 0;
	this->i_lastr = -1;
	Utility::memset(i_addr, 0, sizeof(i_addr));
}

/* �������Inode�����ķ���ʱ�䡢�޸�ʱ�� */
void INode::IUpdate(int time) {
	Buffer* pBuffer;
	DiskINode dINode;
	FileSystem& fileSystem = g_FileSystem;
	BufferManager& bufferManager = g_BufferManager;

	/* 
	 *��IUPD��IACC��־֮һ�����ã�����Ҫ������ӦDiskInode
	 * Ŀ¼����������������;����Ŀ¼�ļ���IACC��IUPD��־ 
	 */
	if (this->i_flag&(INode::IUPD | INode::IACC)) {
		pBuffer = bufferManager.Bread(FileSystem::INODE_ZONE_START_SECTOR + this->i_number / FileSystem::INODE_NUMBER_PER_SECTOR);
		dINode.d_mode = this->i_mode;
		dINode.d_nlink = this->i_nlink;
		dINode.d_uid = this->i_uid;
		dINode.d_gid = this->i_gid;
		dINode.d_size = this->i_size;
		memcpy(dINode.d_addr, i_addr, sizeof(dINode.d_addr));
		if (this->i_flag & INode::IACC) {
			dINode.d_atime = time;
		}
		if (this->i_flag & INode::IUPD) {
			dINode.d_mtime = time;
		}

		unsigned char* p = pBuffer->addr + (this->i_number % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskINode);
		DiskINode* pNode = &dINode;
		Utility::memcpy(p, pNode, sizeof(DiskINode));
		bufferManager.Bwrite(pBuffer);
	}
}

void INode::ITrunc() {
	BufferManager &bufferManager = g_BufferManager;
	FileSystem& fileSystem = g_FileSystem;
	Buffer* pFirstBuffer, *pSecondBuffer;

	for (int i = 9; i >= 0; --i) {
		if (this->i_addr[i]) {
			if (i >= 6) {
				pFirstBuffer = bufferManager.Bread(this->i_addr[i]);
				int *pFirst = (int*)pFirstBuffer->addr;
				for (int j = BLOCK_SIZE / sizeof(int) - 1; j >= 0; --j) {
					if (pFirst[j]) {
						if (i >= 8) {
							pSecondBuffer = bufferManager.Bread(pFirst[j]);
							int* pSecond = (int*)pSecondBuffer->addr;
							for (int k = BLOCK_SIZE / sizeof(int) - 1; k >= 0; --k) {
								if (pSecond[k]) {
									fileSystem.Free(pSecond[k]);
								}
							}
							bufferManager.Brelse(pSecondBuffer);
						}
						fileSystem.Free(pFirst[j]);
					}
				}
				bufferManager.Brelse(pFirstBuffer);
			}
			fileSystem.Free(this->i_addr[i]);
			this->i_addr[i] = 0;
		}
	}
	this->i_size = 0;
	this->i_flag |= INode::IUPD;
	this->i_mode &= ~(INode::ILARG);
	this->i_nlink = 1;
}