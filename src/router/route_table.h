
/*
        $Id$
*/

#if !defined __Cpp_Header__route_table_h__
#define __Cpp_Header__route_table_h__

#include "route_types.h"

namespace smsc {
namespace route {

// abstract
class RouteTable
{
        //
public :
  virtual int lookup(source, dest) = 0;
  virtual RouteInfo getRouteInfo(int idx) = 0;
  virtual SmeProxy* getSmeProxy(int idx) = 0;
};

}; // namespace route
}; // namespace smsc

#endif
