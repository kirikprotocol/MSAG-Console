/*
  $Id$
*/

#if !defined __Cpp_Header__smeman_smetable_h__
#define __Cpp_Header__smeman_smetable_h__

#include "smetypes.h"
#include "smeproxy.h"
#include "smeinfo.h"

namespace smsc {
namespace smeman {


// abstract
class SmeTable
{
public:
  virtual SmeIndex lookup(const SmeSystemId& systemId) const = 0;
  virtual SmeProxy* getSmeProxy(SmeIndex index) const = 0;
  virtual SmeInfo getSmeInfo(SmeIndex index) const = 0;
};


}; // namespace smeman
}; // namespace smsc

#endif


