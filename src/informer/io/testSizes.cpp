#include <stdio.h>
#include <inttypes.h>

int main()
{
#define SHOWSZ(x) printf("the size of %-20s is: %d\n",#x,int(sizeof(x)))
    SHOWSZ(char);
    SHOWSZ(unsigned char);
    SHOWSZ(short);
    SHOWSZ(unsigned short);
    SHOWSZ(int);
    SHOWSZ(unsigned int);
    SHOWSZ(long);
    SHOWSZ(unsigned long);
    SHOWSZ(long long);
    SHOWSZ(unsigned long long);
    SHOWSZ(float);
    SHOWSZ(double);
    SHOWSZ(void*);
    SHOWSZ(int8_t);
    SHOWSZ(int16_t);
    SHOWSZ(int32_t);
    SHOWSZ(int64_t);
    return 0;
}
