#include "FileSystem.h"
#include "User.h"

extern DeviceDriver g_DeviceDriver;
extern BufferManager g_BufferManager;
extern SuperBlock g_SuperBlock;
extern INodeTable g_INodeTable;
extern User g_User;

FileSystem::FileSystem() {
    deviceDriver = &g_DeviceDriver;
    superBlock = &g_SuperBlock;
	bufferManager = &g_BufferManager;

    if (!deviceDriver->Exists()) {
        FormatDevice();
    }
    else {
        LoadSuperBlock();
    }
}

FileSystem::~FileSystem() {
    deviceDriver = NULL;
    superBlock = NULL;
}

void FileSystem::FormatSuperBlock() {
    superBlock->s_isize = FileSystem::INODE_ZONE_SIZE;
    superBlock->s_fsize = FileSystem::DISK_SIZE;
    superBlock->s_nfree = 0;
    superBlock->s_free[0] = -1;
    superBlock->s_ninode = 0;
    superBlock->s_flock = 0;
    superBlock->s_ilock = 0;
    superBlock->s_fmod = 0;
    superBlock->s_ronly = 0;
    time((time_t*)&superBlock->s_time);
}

void FileSystem::FormatDevice() {
    FormatSuperBlock();
    deviceDriver->Construct();

    //���ļ�����д��superblockռ�ݿռ䣬δ�����ļ���С
    deviceDriver->write(superBlock, sizeof(SuperBlock), 0);

    DiskINode emptyDINode, rootDINode;
    //��Ŀ¼DiskNode
    rootDINode.d_mode |= INode::IALLOC | INode::IFDIR;
    deviceDriver->write(&rootDINode, sizeof(rootDINode), INODE_ZONE_START_SECTOR*BLOCK_SIZE);

    //�ӵ�1��DiskINode��ʼ������0���̶����ڸ�Ŀ¼"/"�����ɸı�
    for (int i = 1; i < FileSystem::INode_NUMBERS; ++i) {
        if (superBlock->s_ninode < SuperBlock::MAX_NINode) {
            superBlock->s_inode[superBlock->s_ninode++] = i;
        }
        deviceDriver->write(&emptyDINode, sizeof(emptyDINode));
    }

    //�����̿��ʼ��
    char freeBlock[BLOCK_SIZE], freeBlock1[BLOCK_SIZE];
    memset(freeBlock, 0, BLOCK_SIZE);
    memset(freeBlock1, 0, BLOCK_SIZE);

    for (int i = 0; i < FileSystem::DATA_ZONE_SIZE; ++i) {
        if (++superBlock->s_nfree >= SuperBlock::MAX_NFREE) {
            memcpy(freeBlock1, &superBlock->s_nfree, sizeof(superBlock->s_nfree) + sizeof(superBlock->s_free));
            deviceDriver->write(&freeBlock1, BLOCK_SIZE);
            superBlock->s_nfree = 0;
        }
        else {
            deviceDriver->write(freeBlock, BLOCK_SIZE);
        }
        superBlock->s_free[superBlock->s_nfree] = i;
    }

    time((time_t*)&superBlock->s_time);
}

void FileSystem::LoadSuperBlock() {
    deviceDriver->read(superBlock, sizeof(SuperBlock), SUPERBLOCK_START_SECTOR*BLOCK_SIZE);
}

Buffer* FileSystem::Alloc() {
	int blkno;
	Buffer* pBuffer;
	User& u = g_User;

	/* ��������ջ������ȡ���д��̿��� */
	blkno = superBlock->s_free[--superBlock->s_nfree];

	/* ����ȡ���̿���Ϊ�㣬���ʾ�ѷ��価���еĿ��д��̿顣*/
	if (blkno <= 0) {
		superBlock->s_nfree = 0;
		cout << "No Space On Device !\n";
		u.u_error = User::U_ENOSPC;
		return NULL;
	}

	/*
	* ջ�ѿգ��·��䵽���д��̿��м�¼����һ����д��̿�ı��
	* ����һ����д��̿�ı�Ŷ���SuperBlock�Ŀ��д��̿�������s_free[100]�С�
	*/
	if (superBlock->s_nfree <= 0) {
		pBuffer = this->bufferManager->Bread(blkno);
		int* p = (int *)pBuffer->addr;
		superBlock->s_nfree = *p++;
		memcpy(superBlock->s_free, p, sizeof(int) * SuperBlock::MAX_NFREE);
		this->bufferManager->Brelse(pBuffer);
	}

	pBuffer = this->bufferManager->GetBlk(blkno);
	if (pBuffer) {
		this->bufferManager->ClrBuf(pBuffer);
	}
	superBlock->s_fmod = 1;
	return pBuffer;
}

INode* FileSystem::IAlloc() {
	Buffer* pBuffer;
	INode* pINode;
	User& u = g_User;
	int ino;

	/*
	 * SuperBlockֱ�ӹ���Ŀ���Inode�������ѿգ�
	 * ���뵽��������������Inode��
	 */
	if (superBlock->s_ninode <= 0) {
		ino = -1;
		for (int i = 0; i < superBlock->s_isize; ++i) {
			pBuffer = this->bufferManager->Bread(FileSystem::INODE_ZONE_START_SECTOR + i);
			int* p = (int*)pBuffer->addr;
			for (int j = 0; j < FileSystem::INODE_NUMBER_PER_SECTOR; ++j) {
				++ino;
				int mode = *(p + j * sizeof(DiskINode) / sizeof(int));
				if (mode) {
					continue;
				}

				/*
				* ������inode��i_mode==0����ʱ������ȷ��
				* ��inode�ǿ��еģ���Ϊ�п������ڴ�inodeû��д��
				* ������,����Ҫ���������ڴ�inode���Ƿ�����Ӧ����
				*/
				if (g_INodeTable.IsLoaded(ino) == -1) {
					superBlock->s_inode[superBlock->s_ninode++] = ino;
					if (superBlock->s_ninode >= SuperBlock::MAX_NINode) {
						break;
					}
				}
			}

			this->bufferManager->Brelse(pBuffer);
			if (superBlock->s_ninode >= SuperBlock::MAX_NINode) {
				break;
			}
		}
		if (superBlock->s_ninode <= 0) {
			u.u_error = User::U_ENOSPC;
			return NULL;
		}
	}

	while (true) {
		ino = superBlock->s_inode[--superBlock->s_ninode];
		pINode = g_INodeTable.IGet(ino);
		if (NULL == pINode) {
			return NULL;
		}
		pINode->Clean();
		superBlock->s_fmod = 1;
		return pINode;
	}
	return NULL;
}