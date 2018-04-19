#ifndef UTILITY_H
#define UTILITY_H

#include <bits/stdc++.h>
using namespace std;

class Utility{
public:
	static void memcpy(void *dest, const void *src, size_t n);
	static void memset(void *s, int ch, size_t n);
	static int memcmp(const void *buf1, const void *buf2, unsigned int count);
	static int min(int a, int b);
	static time_t time(time_t* t);
};

#endif
