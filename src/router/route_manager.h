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
#include <set>

namespace smsc {
namespace router {

using smsc::smeman::SmeProxy;
using smsc::smeman::SmeIndex;
using smsc::smeman::SmeTable;

using namespace std;

/*
struct RouteValue
{
  uint8_t length;
  uint8_t npi; // numbering plan
  uint8_t tni; // type of number
  uint8_t value[0];
};
*/

struct RouteRecord
{
  const RouteInfo* info; // has address
  RoutePoint rp;
  SmeIndex proxyIdx;
  SmeIndex srcProxyIdx;
  RouteRecord* alternate_pair;
  RouteRecord* next;
  uint8_t src_def;
  uint8_t dest_def;
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
    child.clear();
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
    child.clear();
    sources.clear();
  }
};


class RouteManager : public RouteAdmin, public RouteTable
{
protected:
  SmeTable* sme_table;

  struct SmeRouter{
    RouteRecord* first_record;
    RouteRecord* new_first_record;
    RouteTreeNode  root;

    SmeRouter():first_record(0),new_first_record(0)
    {

    }
    ~SmeRouter()
    {
      ClearList(first_record);
      ClearList(new_first_record);
    }

  };

  std::vector<SmeRouter*> smeRouters;
  int maxSmeRouter;

  RouteRecord* first_record;
  RouteRecord* new_first_record;
  RouteTreeNode  root;
  vector<string> trace_;
  bool trace_enabled_;

  std::set<RouteInfo*> infoSet;

  static bool smeRoutersEnabled;

  static void ClearList(RouteRecord*& list)
  {
    while(list)
    {
      RouteRecord* r=list;
      list=list->next;
      delete r;
    }
  }

public :
  RouteManager() : sme_table(0),first_record(0),new_first_record(0),trace_enabled_(false)
  {
    smeRouters.resize(smsc::smeman::MAX_SME_PROXIES,0);
    maxSmeRouter=0;
  }

  virtual ~RouteManager()
  {
    ClearList(first_record);
    ClearList(new_first_record);
    for(std::vector<SmeRouter*>::iterator it=smeRouters.begin();it!=smeRouters.end();it++)
    {
      if(*it)
      {
        delete *it;
        *it=0;
      }
    }
    smeRouters.clear();
  }

  static void EnableSmeRouters(bool value)
  {
    smeRoutersEnabled=value;
  }

  void assign(SmeTable* smetable); // for detach call with NULL;
  //RouteIterator* iterator();
  // RouteAdministrator implementaion
  virtual void addRoute(RouteInfo* routeInfo,const RoutePoint& rp);
  virtual void commit(bool = false);
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
  // � ��������� �� ������ ������������ ������ ��� �������� (����� ���� 0)
  //
  virtual bool lookup(const Address& source, const Address& dest, SmeProxy*& proxy, int* idx=0,RouteInfo* info=0);
  virtual bool lookup(int srcidx, const Address& source, const Address& dest, SmeProxy*& proxy, int* idx=0,RouteInfo* info=0);
  virtual void getTrace(vector<string>&);
  virtual void enableTrace(bool);
};

} // namespace route
} // namespace smsc

#endif
