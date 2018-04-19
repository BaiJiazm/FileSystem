#include "Utility.h"

void Utility::memcpy(void *dest, const void *src, size_t n) {
	::memcpy(dest, src, n);
}

void Utility::memset(void *s, int ch, size_t n) {
	::memset(s, ch, n);
}

int Utility::memcmp(const void *buf1, const void *buf2, unsigned int count) {
	return ::memcmp(buf1, buf2, count);
}

int Utility::min(int a, int b) {
	if (a < b)
		return a;
	return b;
}

time_t Utility::time(time_t* t) {
	return ::time(t);
}
