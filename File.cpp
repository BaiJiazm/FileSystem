#include "Utility.h"
#include "File.h"
#include "User.h"

extern User g_User;

File::File() {
	f_flag = 0;
	f_count = 0;
	f_inode = NULL;
	f_offset = 0;
}

File::~File() {
}

OpenFiles::OpenFiles() {
	Utility::memset(processOpenFileTable, NULL, sizeof(processOpenFileTable));
}

OpenFiles::~OpenFiles() {
}

/* ����������ļ�ʱ���ڴ��ļ����������з���һ�����б��� */
int OpenFiles::AllocFreeSlot() {
	User& u = g_User;
	for (int i = 0; i < OpenFiles::MAX_FILES; i++) {
		/* ���̴��ļ������������ҵ�������򷵻�֮ */
		if (this->processOpenFileTable[i] == NULL) {
			//u.u_ar0[User::EAX] = i;
			return i;
		}
	}

	//u.u_ar0[User::EAX] = -1;   /* Open1����Ҫһ����־�������ļ��ṹ����ʧ��ʱ�����Ի���ϵͳ��Դ*/
	u.u_error = User::U_EMFILE;
	return -1;
}

/* �����û�ϵͳ�����ṩ���ļ�����������fd���ҵ���Ӧ�Ĵ��ļ����ƿ�File�ṹ */
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

/* Ϊ�ѷ��䵽�Ŀ���������fd���ѷ���Ĵ��ļ����п���File������������ϵ */
void OpenFiles::SetF(int fd, File* pFile) {
	if (fd < 0 || fd >= OpenFiles::MAX_FILES) {
		return;
	}
	this->processOpenFileTable[fd] = pFile;
}
