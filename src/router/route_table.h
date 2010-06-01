
/*
        $Id$
*/

#if !defined __Cpp_Header__route_table_h__
#define __Cpp_Header__route_table_h__

#include "route_types.h"
#include "sms/sms.h"
#include <vector>
#include <string>

using namespace std;

namespace smsc {
namespace router {

using smsc::sms::Address;

// abstract
class RouteTable
{
        //
public :

  // throws Exception when route not found
  //virtual SmeProxy* lookup(const Address& source,const Address& dest, RouteInfo* info=0) = 0;

  // return  thue when proxy found and false otherwise
  virtual bool lookup(const Address& source, const Address& dest, RouteResult& rr) = 0;
  virtual bool lookup(int srcidx, const Address& source, const Address& dest, RouteResult& rr) = 0;

  //virtual RouteInfo getRouteInfo(int idx) = 0;
  //virtual SmeProxy* getSmeProxy(int idx) = 0;
  virtual void getTrace(vector<string>&) = 0;
  virtual void enableTrace(bool) = 0;
};

} // namespace route
} // namespace smsc

#endif
