/*
        $Id$
*/

#if !defined __Cpp_Header__route_manager_h__
#define __Cpp_Header__route_manager_h__

#include "route_types.h"
#include "smeman/smetable.h"
#include "route_iter.h"
#include "route_admin.h"
#include "route_table.h"
#include "sms/sms.h"

#include <vector>

namespace smsc {
namespace router {

using smsc::smeman::SmeProxy;
using smsc::smeman::SmeIndex;
using smsc::smeman::SmeTable;

struct RoutePattern
{
  union
  {
    struct
    {
      uint8_t dest_typeOfNumber;
      uint8_t src_typeOfNumber;
      uint8_t dest_numberingPlan;
      uint8_t src_numberingPlan;
    };
    int32_t num_n_plan;
  };
  union
  {
    uint8_t src_addressMask[21];
    int32_t src_addressMask_32[5];
  };
  union
  {
    uint8_t dest_addressMask[21];
    int32_t dest_addressMask_32[5];
  };
  union
  {
    uint8_t src_addressPattern[21];
    int32_t src_addressPattern_32[5];
  };
  union
  {
    uint8_t dest_addressPattern[21];
    int32_t dest_addressPattern_32[5];
  };
};

struct RouteAddress
{
  union
  {
    struct
    {
      uint8_t dest_typeOfNumber;
      uint8_t src_typeOfNumber;
      uint8_t dest_numberingPlan;
      uint8_t src_numberingPlan;
    };
    int32_t num_n_plan;
  };
  union
  {
    uint8_t src_address[21];
    int32_t src_address_32[5];
  };
  union
  {
    uint8_t dest_address[21];
    int32_t dest_address_32[5];
  };
};

struct RouteRecord 
{
  RouteInfo info;
  //SmeProxy* proxy;
  SmeIndex proxyIdx;
  RoutePattern pattern;
  uint8_t src_pattern_undef;
  uint8_t dest_pattern_undef;
  //RouteIdentifier rid;
  RouteRecord* ok_next;
  RouteRecord() : ok_next(0) {}
};

class RouteManager : public RouteAdmin, public RouteTable
{
  // struct array (not need delete[] for calling destructors)
  SmeTable* smeTable;
  RouteRecord** table;
  int table_size;
	int table_ptr;
	bool sorted;
  //
public :
  RouteManager() : table_size(1024), table_ptr(0), sorted(false)
  {
    table=new RouteRecord*[table_size];
  }
  virtual ~RouteManager(){}
  void assign(SmeTable* smetable); // for detach call with NULL;
  RouteIterator* iterator();
  // RouteAdministrator implementaion 
  virtual void addRoute(const RouteInfo& routeInfo);
  /*virtual void removeRoute(RouteId id);
  virtual void modifyRoute(RouteId id,const RouteInfo& routeInfo);*/
  // RoutingTable implementation
  //virtual int lookup( const smsc::sms::Address& source, const smsc::sms::Address& dest);
  //virtual RouteInfo getRouteInfo(int idx);
  //virtual SmeProxy* getSmeProxy(int idx);
  
  //
  // return true when route found
  //    otherwise return false
  // в указатель на прокси записывается прокси для маршрута (может быть 0)
  //
  virtual bool lookup(const Address& source, const Address& dest, SmeProxy*& proxy, RouteInfo* info=0);

};

}; // namespace route
}; // namespace smsc

#endif
