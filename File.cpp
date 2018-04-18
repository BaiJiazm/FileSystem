#include "File.h"

File::File() {
	f_flag = 0;
	f_count = 0;
	f_inode = NULL;
	f_offset = 0;
}

File::~File() {

}

OpenFiles::OpenFiles() {
	memset(processOpenFileTable, NULL, sizeof(processOpenFileTable));
}

OpenFiles::~OpenFiles() {

}

IOParameter::IOParameter() {

}
IOParameter::~IOParameter() {

}