#include "Buffer.h"

Buffer::Buffer() {
	flags = 0;
	forw = NULL;
	back = NULL;
	wcount = 0;
	addr = NULL;
	blkno = -1;
	error = -1;
	resid = 0;
}

Buffer::~Buffer() {

}