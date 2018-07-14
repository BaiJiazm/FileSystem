#include "Utility.h"
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
    Update();
    deviceDriver = NULL;
    superBlock = NULL;
}

/* 格式化SuperBlock */
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

/* 格式化整个文件系统 */
void FileSystem::FormatDevice() {
    FormatSuperBlock();
    deviceDriver->Construct();

    //空文件，先写入superblock占据空间，未设置文件大小
    deviceDriver->write(superBlock, sizeof(SuperBlock), 0);

    DiskINode emptyDINode, rootDINode;
    //根目录DiskNode
    rootDINode.d_mode |= INode::IALLOC | INode::IFDIR;
    rootDINode.d_nlink = 1;
    deviceDriver->write(&rootDINode, sizeof(rootDINode));
    
    //从第1个DiskINode初始化，第0个固定用于根目录"/"，不可改变
    for (int i = 1; i < FileSystem::INode_NUMBERS; ++i) {
        if (superBlock->s_ninode < SuperBlock::MAX_NINODE) {
            superBlock->s_inode[superBlock->s_ninode++] = i;
        }
        deviceDriver->write(&emptyDINode, sizeof(emptyDINode));
    }

    //空闲盘块初始化
    char freeBlock[BLOCK_SIZE], freeBlock1[BLOCK_SIZE];
    memset(freeBlock, 0, BLOCK_SIZE);
    memset(freeBlock1, 0, BLOCK_SIZE);

    for (int i = 0; i < FileSystem::DATA_ZONE_SIZE; ++i) {
        if (superBlock->s_nfree >= SuperBlock::MAX_NFREE) {
            memcpy(freeBlock1, &superBlock->s_nfree, sizeof(int) + sizeof(superBlock->s_free));
            deviceDriver->write(&freeBlock1, BLOCK_SIZE);
            superBlock->s_nfree = 0;
        }
        else {
            deviceDriver->write(freeBlock, BLOCK_SIZE);
        }
        superBlock->s_free[superBlock->s_nfree++] = i + DATA_ZONE_START_SECTOR;
    }

    time((time_t*)&superBlock->s_time);
    //再次写入superblock
    deviceDriver->write(superBlock, sizeof(SuperBlock), 0);
}

/* 系统初始化时读入SuperBlock */
void FileSystem::LoadSuperBlock() {
    deviceDriver->read(superBlock, sizeof(SuperBlock), SUPERBLOCK_START_SECTOR*BLOCK_SIZE);
}

/* 将SuperBlock对象的内存副本更新到存储设备的SuperBlock中去 */
void FileSystem::Update() {
	Buffer* pBuffer;
	superBlock->s_fmod = 0;
    superBlock->s_time = (int)Utility::time(NULL);
	for (int j = 0; j < 2; j++) {
		int* p = (int *)superBlock + j * 128;
		pBuffer = this->bufferManager->GetBlk(FileSystem::SUPERBLOCK_START_SECTOR + j);
		Utility::memcpy(pBuffer->addr, p, BLOCK_SIZE);
		this->bufferManager->Bwrite(pBuffer);
	}
	g_INodeTable.UpdateINodeTable();
	this->bufferManager->Bflush();
}

/* 在存储设备上分配空闲磁盘块 */
Buffer* FileSystem::Alloc() {
	int blkno;
	Buffer* pBuffer;
	User& u = g_User;

    /* 从索引表“栈顶”获取空闲磁盘块编号 */
	blkno = superBlock->s_free[--superBlock->s_nfree];

    /* 若获取磁盘块编号为零，则表示已分配尽所有的空闲磁盘块。*/
	if (blkno <= 0) {
		superBlock->s_nfree = 0;
		u.u_error = User::U_ENOSPC;
		return NULL;
	}

	/*
	* 栈已空，新分配到空闲磁盘块中记录了下一组空闲磁盘块的编号
	* 将下一组空闲磁盘块的编号读入SuperBlock的空闲磁盘块索引表s_free[100]中。
	*/
    if (superBlock->s_nfree <= 0) {
		pBuffer = this->bufferManager->Bread(blkno);
		int* p = (int *)pBuffer->addr;
		superBlock->s_nfree = *p++;
		memcpy(superBlock->s_free, p, sizeof(superBlock->s_free));
		this->bufferManager->Brelse(pBuffer);
	}
	pBuffer = this->bufferManager->GetBlk(blkno);
	if (pBuffer) {
		this->bufferManager->Bclear(pBuffer);
	}
	superBlock->s_fmod = 1;
	return pBuffer;
}

/* 在存储设备dev上分配一个空闲外存INode，一般用于创建新的文件。*/
INode* FileSystem::IAlloc() {
    Buffer* pBuffer;
    INode* pINode;
    User& u = g_User;
    int ino;

    /* SuperBlock直接管理的空闲Inode索引表已空，必须到磁盘上搜索空闲Inode。*/
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
                * 如果外存inode的i_mode==0，此时并不能确定该inode是空闲的，
                * 因为有可能是内存inode没有写到磁盘上,所以要继续搜索内存inode中是否有相应的项
                */
                if (g_INodeTable.IsLoaded(ino) == -1) {
                    superBlock->s_inode[superBlock->s_ninode++] = ino;
                    if (superBlock->s_ninode >= SuperBlock::MAX_NINODE) {
                        break;
                    }
                }
            }

            this->bufferManager->Brelse(pBuffer);
            if (superBlock->s_ninode >= SuperBlock::MAX_NINODE) {
                break;
            }
        }
        if (superBlock->s_ninode <= 0) {
            u.u_error = User::U_ENOSPC;
            return NULL;
        }
    }

    ino = superBlock->s_inode[--superBlock->s_ninode];
    pINode = g_INodeTable.IGet(ino);
    if (NULL == pINode) {
        cout << "无空闲内存INode" << endl;
        return NULL;
    }

    pINode->Clean();
    superBlock->s_fmod = 1;
    return pINode;
}

/* 释放编号为number的外存INode，一般用于删除文件。*/
void FileSystem::IFree(int number) {
	if (superBlock->s_ninode >= SuperBlock::MAX_NINODE) {
		return ;
	}
	superBlock->s_inode[superBlock->s_ninode++] = number;
	superBlock->s_fmod = 1;
}

/* 释放存储设备dev上编号为blkno的磁盘块 */
void FileSystem::Free(int blkno) {
	Buffer* pBuffer;
	User& u = g_User;

	if (superBlock->s_nfree >=SuperBlock::MAX_NFREE ) {
		pBuffer = this->bufferManager->GetBlk(blkno);
		int *p = (int*)pBuffer->addr;
		*p++ = superBlock->s_nfree;
		Utility::memcpy(p, superBlock->s_free, sizeof(int)*SuperBlock::MAX_NFREE);
		superBlock->s_nfree = 0;
		this->bufferManager->Bwrite(pBuffer);
	}

	superBlock->s_free[superBlock->s_nfree++] = blkno;
	superBlock->s_fmod = 1;
}