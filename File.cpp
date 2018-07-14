#include "Utility.h"
#include "File.h"
#include "User.h"

extern User g_User;

File::File() {
	flag = 0;
	count = 0;
	inode = NULL;
	offset = 0;
}

File::~File() {
}

OpenFiles::OpenFiles() {
	Utility::memset(processOpenFileTable, NULL, sizeof(processOpenFileTable));
}

OpenFiles::~OpenFiles() {
}

/* 进程请求打开文件时，在打开文件描述符表中分配一个空闲表项 */
int OpenFiles::AllocFreeSlot() {
    User& u = g_User;
    for (int i = 0; i < OpenFiles::MAX_FILES; i++) {
        /* 进程打开文件描述符表中找到空闲项，则返回之 */
        if (this->processOpenFileTable[i] == NULL) {
            u.ar0[User::EAX] = i;
            return i;
        }
    }

    u.ar0[User::EAX] = -1;   /* Open1，需要一个标志。当打开文件结构创建失败时，可以回收系统资源*/
    u.u_error = User::U_EMFILE;
    return -1;
}

/* 根据用户系统调用提供的文件描述符参数fd，找到对应的打开文件控制块File结构 */
File* OpenFiles::GetF(int fd) {
	User& u = g_User;
	File* pFile;

	if (fd < 0 || fd >= OpenFiles::MAX_FILES) {
		u.u_error = User::U_EBADF;
		return NULL;
	}

	pFile = this->processOpenFileTable[fd];
	if (pFile == NULL) {
		u.u_error = User::U_EBADF;
	}
	return pFile;
}

/* 为已分配到的空闲描述符fd和已分配的打开文件表中空闲File对象建立勾连关系 */
void OpenFiles::SetF(int fd, File* pFile) {
	if (fd < 0 || fd >= OpenFiles::MAX_FILES) {
		return;
	}
	this->processOpenFileTable[fd] = pFile;
}
