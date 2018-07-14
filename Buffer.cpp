#include "Buffer.h"

Buffer::Buffer() {
	flags = 0;
	forw = NULL;
	back = NULL;
	wcount = 0;
	addr = NULL;
	blkno = -1;
	u_error = -1;
	resid = 0;

    no=0;
}

Buffer::~Buffer() {

}

#include "BufferManager.h"
#include <iostream>
#include <iomanip>
#include <ctype.h>
using namespace std;

void Buffer::debugContent() {
    debugMark();
    for (int i = 0; i < BufferManager::BUFFER_SIZE; i += 32) {
        cout << "  " << setfill('0') << setw(4) << hex << i << ": ";
        for (int j = i; j < i + 32; ++j) {
            cout << ((j + 1 - i % 8 == 0) ? " - " : " ");
            cout << setfill('0') << setw(2) << hex << (unsigned int)(unsigned char)addr[j];
        }
        cout << "  " << setw(4) << *(int*)(addr + i) << " ";
        for (int j = i + 4; j < i + 32; ++j) {
            cout << (isprint(addr[j]) ? (char)addr[j] : '.');
        }
        cout << endl;
    }
    cout << dec;
}

void Buffer::debugMark() {
    cout << "no = " << no << " blkno = " << blkno << " flag = " << (flags&B_DONE ? " DONE " : " ") << (flags&B_DELWRI ? " DELWRI " : " ") << endl;
}