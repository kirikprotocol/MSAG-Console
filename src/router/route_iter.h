
/*
  $Id$
*/

#if !defined __Cpp_Header__route_iter_h__
#define __Cpp_Header__route_iter_h__

#include "smeman/smetypes.h"
#include "smeman/smeproxy.h"
#include "route_types.h"

namespace smsc {
namespace route {

using smsc::smeman::SmeProxy;

// abstract
class RouteIterator
{
public:
  virtual bool next() = 0;
  virtual SmeProxy* getSmeProxy() const = 0;
  virtual RouteInfo  getRouteInfo() const = 0;
  virtual int getIndex() const = 0;
};


}; // namespace smeman
}; // namespace smsc

#endif


