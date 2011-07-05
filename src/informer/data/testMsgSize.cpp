#include "stdio.h"
#include "inttypes.h"
#include "Message.h"

using namespace eyeline::informer;

int main()
{
#define showsz(x) printf("sizeof(%s) = %u\n",#x,unsigned(sizeof(x)))
    showsz(int);
    showsz(void*);
    showsz(Message);
    showsz(MessageText);
    showsz(MessageFlags);
    showsz(MessageLocker);
    MessageLocker ml;
    ml.msg.msgId = 0;
    ml.msg.state = 0;
/*
#define showof(x) printf("offset of %s is %u\n",#x,unsigned(reinterpret_cast<const char*>(&(ml.msg.##x)) - reinterpret_cast<const char*>(&ml)))
    showof(msgId);
    showof(msgUserData);
    showof(text);
    showof(flags);
    showof(subscriber);
    showof(lastTime);
    showof(timeLeft);
    showof(retryCount);
    showof(state);
 */
    return 0;
}
