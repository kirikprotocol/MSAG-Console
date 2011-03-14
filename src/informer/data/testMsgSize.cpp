#include "stdio.h"
#include "Message.h"

using namespace eyeline::informer;

int main()
{
#define showsz(x) printf("sizeof(%s) = %u\n",#x,unsigned(sizeof(x)))
    showsz(int);
    showsz(void*);
    showsz(Message);
    showsz(MessageLocker);
    return 0;
}
