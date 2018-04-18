#include "DeviceDriver.h"

const char * DeviceDriver::DISK_FILE_NAME = "1453381-fs.img";

DeviceDriver::DeviceDriver() {
    fp = fopen(DISK_FILE_NAME, "rb+");
}

DeviceDriver::~DeviceDriver() {
    if (fp) {
        fclose(fp);
    }
}

bool DeviceDriver::Exists() {
    return fp != NULL;
}

void DeviceDriver::Construct() {
    fp = fopen(DISK_FILE_NAME, "wb+");
    if (fp == NULL) {
        printf("打开或新建文件%s失败！", DISK_FILE_NAME);
        exit(-1);
    }
}

void DeviceDriver::write(const void* buffer, unsigned int size, unsigned int offset, unsigned int origin) {
    if (offset >= 0) {
        fseek(fp, offset, origin);
    }
    fwrite(buffer, size, 1, fp);
}

void DeviceDriver::read(void* buffer, unsigned int size, unsigned int offset, unsigned int origin) {
    if (offset >= 0) {
        fseek(fp, offset, origin);
    }
    fread(buffer, size, 1, fp);
}