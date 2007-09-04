/*
        $Id$
*/

#if !defined __SCAG_Cpp_Header__route_manager_h__
#define __SCAG_Cpp_Header__route_manager_h__

#include "route_types.h"
#include "sms/sms.h"
#include "core/buffers/Hash.hpp"

#include <vector>

namespace scag {
namespace transport {
namespace smpp {
namespace router {


using namespace std;


struct RouteRecord
{
  RouteInfo info; // has address
  SmeIndex proxyIdx;
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
    child.clear();
    for ( unsigned i=0; i< sources.size(); ++i )
      { if ( sources[i] ) delete sources[i]; }
    sources.clear();
  }
};


class RouteManager
{
  struct SmeRoute
  {
    SmeRoute():first_record(0),new_first_record(0){}
    RouteRecord* first_record;
    RouteRecord* new_first_record;
    RouteTreeNode  root;
  };
  smsc::core::buffers::Hash<SmeRoute> smeRoutes;
  RouteRecord* first_record;
  RouteRecord* new_first_record;
  RouteTreeNode  root;
  vector<string> trace_;
  bool trace_enabled_;

  void clear_list(RouteRecord*& lst)
  {
    while ( lst )
    {
      RouteRecord* r = lst;
      lst = lst->next;
      delete r;
    }
  }

public :
  RouteManager() : first_record(0),new_first_record(0),trace_enabled_(false)
  {}

  virtual ~RouteManager()
  {
    clear_list(first_record);
    clear_list(new_first_record);
    char* k;
    SmeRoute* v;
    smeRoutes.First();
    while(smeRoutes.Next(k,v))
    {
      clear_list(v->first_record);
      clear_list(v->new_first_record);
    }
  }

  void addRoute(const RouteInfo& routeInfo);
  void commit(bool = false);
  void cancel();

  bool lookup(const smsc::sms::Address& source, const smsc::sms::Address& dest, RouteInfo& info);
  bool lookup(SmeIndex srcidx, const smsc::sms::Address& source, const smsc::sms::Address& dest, RouteInfo& info);
  void getTrace(vector<string>&);
  void enableTrace(bool);
};

}//namespace scag
}//namespace transport
}//namespace smpp
}//namespace router

#endif
