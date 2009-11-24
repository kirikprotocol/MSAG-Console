/*
  $Id$
*/

#if !defined __Cpp_Header__smeman_smeadmin_h__
#define __Cpp_Header__smeman_smeadmin_h__

#include "smetypes.h"
#include "smeiter.h"

namespace smsc {
namespace smeman {


// abstract
class SmeAdministrator
{
public:
  virtual void addSme(const SmeInfo& info) = 0;
  virtual void deleteSme(const SmeSystemId& systemid) = 0;
  virtual SmeIterator* iterator() = 0;
  virtual void updateSmeInfo(const SmeSystemId& systemid,const SmeInfo& newinfo) =0;
};


} // namespace smeman
} // namespace smsc

#endif
