#ifndef __SMSC_EMAILSME_UTIL_HPP__
#define __SMSC_EMAILSME_UTIL_HPP__

#include <stdio.h>
#include <vector>

namespace smsc{
namespace emailsme{
namespace util{

bool isChildRunning(int idx);
void PipesInit(int threadsNum);
int InitPipeThread();

pid_t ForkPipedCmd(int idx,const char *cmd, FILE*& f_in,FILE*& f_out);

}
}
}

#endif
