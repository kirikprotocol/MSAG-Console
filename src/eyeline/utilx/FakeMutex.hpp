#ifndef __EYELINE_UTILX_FAKEMUTEX_HPP__
# define __EYELINE_UTILX_FAKEMUTEX_HPP__

# include <pthread.h>

namespace eyeline {
namespace utilx {

class FakeMutex
{
public:
  FakeMutex() {}

  inline  void Lock() {}
  inline  void Unlock() {}
  inline  bool TryLock() {
    return true;
  }

  inline int WaitCondition(pthread_cond_t & cond_var) { return 0; }

};

}}

#endif
