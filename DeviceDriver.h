#pragma once

#include "bits/stdc++.h"
using namespace std;

class DeviceDriver {
public:
    /* ���̾����ļ��� */
    static const char* DISK_FILE_NAME;

private:
    /* �����ļ�ָ�� */
    FILE* fp;

public:
    DeviceDriver();
    ~DeviceDriver();

    /* ��龵���ļ��Ƿ���� */
    bool Exists();

    /* �򿪾����ļ� */
    void Construct();

    /* ʵ��д���̺��� */
    void write(const void* buffer, unsigned int size,
        int offset = -1, unsigned int origin = SEEK_SET);

    /* ʵ��д���̺��� */
    void read(void* buffer, unsigned int size, 
        int offset = -1, unsigned int origin = SEEK_SET);
};