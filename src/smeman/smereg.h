
/*
  $Id$
*/

#if !defined __Cpp_Header__smeman_smereg_h__
#define __Cpp_Header__smeman_smereg_h__

#include "smetypes.h"
#include "smeproxy.h"

namespace smsc {
namespace smeman {

// abstract
class SmeRegistrar
{
public:
  virtual bool registerSmeProxy(const SmeSystemId& systemId,
                                const SmePassword& pwd,
                                SmeProxy* smeProxy) = 0;
  virtual void registerInternallSmeProxy(const SmeSystemId& systemId,
                                         SmeProxy* smeProxy)=0;
  virtual void unregisterSmeProxy(SmeProxy* smeProxy) = 0;
};


} // namespace smeman
} // namespace smsc

#endif
