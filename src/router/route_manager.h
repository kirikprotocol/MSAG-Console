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

using namespace std;

struct RouteValue
{
  uint8_t length;
  uint8_t npi; // numbering plan
  uint8_t tni; // type of number
  uint8_t value[0];
};

struct RouteRecord 
{
  RouteInfo info; // has address
  SmeIndex proxyIdx;
  SmeIndex srcProxyIdx;
  RouteRecord* alternate_pair;
  uint8_t src_def;
  uint8_t dest_def;
  RouteRecord* next;
  RouteRecord() : next(0) {}
};

struct RouteSrcTreeNode
{
  RouteRecord* record;
  vector<RouteSrcTreeNode*> child;
  RouteSrcTreeNode() : record(0) {}
  ~RouteSrcTreeNode() {clean();}
  void clean()
  {
    record = 0;
    for ( unsigned i=0; i< child.size(); ++i ) 
      { if ( child[i] ) delete child[i]; }
  }
};

struct RouteTreeNode
{
  RouteRecord* record;
  vector<RouteTreeNode*> child;
  vector<RouteSrcTreeNode*> sources;
  RouteTreeNode() : record(0) {}
  ~RouteTreeNode() {clean();}
  void clean()
  {
    record = 0;
    for ( unsigned i=0; i< child.size(); ++i ) 
      { if ( child[i] ) delete child[i]; }
    for ( unsigned i=0; i< sources.size(); ++i ) 
      { if ( sources[i] ) delete sources[i]; }
  }
};


class RouteManager : public RouteAdmin, public RouteTable
{
  SmeTable* sme_table;
  RouteRecord* first_record;
  RouteRecord* new_first_record;
  RouteTreeNode  root;
public :
  RouteManager() : sme_table(0),first_record(0),new_first_record(0)
  {}
  
  virtual ~RouteManager() 
  {
    while ( first_record )
    {
      RouteRecord* r = first_record;
      first_record = first_record->next;
      delete r;
    }
    while ( new_first_record )
    {
      RouteRecord* r = new_first_record;
      new_first_record = new_first_record->next;
      delete r;
    }
  }

  void assign(SmeTable* smetable); // for detach call with NULL;
  //RouteIterator* iterator();
  // RouteAdministrator implementaion 
  virtual void addRoute(const RouteInfo& routeInfo);
  virtual void commit();
  virtual void cancel();
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
  virtual bool lookup(const Address& source, const Address& dest, SmeProxy*& proxy, int* idx=0,RouteInfo* info=0);

};

}; // namespace route
}; // namespace smsc

#endif
