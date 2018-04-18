#pragma once

#include "bits/stdc++.h"
using namespace std;

class DeviceDriver {
public:
    static const char* DISK_FILE_NAME;

private:
    FILE* fp;

public:
    DeviceDriver();
    ~DeviceDriver();
    bool Exists();
    void Construct();
    void write(const void* buffer, unsigned int size, unsigned int offset = -1, unsigned int origin = SEEK_SET);
    void read(void* buffer, unsigned int size, unsigned int offset = -1, unsigned int origin = SEEK_SET);
};