
/*
  $Id$
*/

#if !defined __Cpp_Header__smeman_dispatch_h__
#define __Cpp_Header__smeman_dispatch_h__

#include "smetypes.h"
#include "smeproxy.h"
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace smeman {

// abstract 
class Dispatch
{
public:
  virtual SmeProxy* selectSmeProxy(unsigned long timeout=0,int* idx=0) = 0;
};

}; // namespace smeman
}; // namespace smsc

#endif


