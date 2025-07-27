/**
 * Gcc helper functions
 */
#include "types.h"

uint64_t __udivdi3(uint64_t n, uint64_t d)
{
	if(d==0) {
		return 0xffffffffffffffffULL;
	}
	uint64_t q=0,r=0;
	for(int i=63;i>=0;i--) {
		r<<=1;
		r|=(n>>i)&1;
		if(r>=d) r-=d,q|=(1ULL<<i);
	}
	return q;
}

// uint64_t % uint64_t
uint64_t __umoddi3(uint64_t n, uint64_t d)
{
	if(d==0) {
		return 0xffffffffffffffffULL;
	}
	uint64_t r=0;
	for(int i=63;i>=0;i--) {
		r<<=1;
		r|=(n>>i)&1;
		if(r>=d) r-=d;
	}
	return r;
}
