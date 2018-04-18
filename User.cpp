#include "User.h"

extern FileManager g_FileManager;

User::User() {
	u_cdir = NULL;
    u_pdir = NULL;
    fileManager = &g_FileManager;
	u_dirp = "/";
    fileManager->Open();
}