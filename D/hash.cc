#include "D.h"

// *************************************************************
// * theIDHash
// *   Hash an arbitrary string to 4 bytes of an unsigned int.
// *************************************************************
unsigned int theIDHash(const char *s0) //, unsigned int key=0)
{
char values[4] = {'\0', '\0', '\0', '\0'};

	if (!s0) return 0;
	
	int result=0, i, len = strlen(s0), pos=0;

	for(i=0; i<len; i++, (++pos) %= 4)
		if (values[pos])	// non zero
			values[pos] ^= s0[i];
		else
			values[pos] = s0[i];	// retain original value

	if (len > 4)	// 4 or less
		len = 4;

	for(i=0; i<len; i++)
	{
//		values[i] ^= (key & f);	// xor with right 8 bits of key
//		key = (key >>8);	// ready for next 8 bits
		result = (result <<8)
			+ (char) values[i];	// OR next byte value
	}
	return result;
}
