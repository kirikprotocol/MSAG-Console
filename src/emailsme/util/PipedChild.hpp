#ifndef __SMSC_EMAILSME_UTIL_HPP__
#define __SMSC_EMAILSME_UTIL_HPP__

namespace smsc{
namespace emailsme{
namespace util{

extern bool childRunning;
pid_t ForkPipedCmd(const char *cmd, FILE*& f_in,FILE*& f_out);

}
}
}

#endif
