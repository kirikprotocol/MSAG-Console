/*
  $Id$
*/

#if !defined __Cpp_Header__smeman_smeiter_h__
#define __Cpp_Header__smeman_smeiter_h__

#include "smetypes.h"
#include "smeproxy.h"
#include "smeinfo.h"

namespace smsc {
namespace smeman {


// abstract
class SmeIterator
{
public:
  virtual bool next() = 0;
  virtual SmeProxy* getSmeProxy() const = 0;
  virtual SmeInfo  getSmeInfo() const = 0;
  virtual SmeIndex getSmeIndex() const = 0;
};


}; // namespace smeman
}; // namespace smsc

#endif


