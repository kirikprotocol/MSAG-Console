/*
        $Id$
*/

#if !defined __SCAG_Cpp_Header__route_manager_h__
#define __SCAG_Cpp_Header__route_manager_h__

#include "route_types.h"
#include "sms/sms.h"

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


class RouteManager
{
  RouteRecord* first_record;
  RouteRecord* new_first_record;
  RouteTreeNode  root;
  vector<string> trace_;
  bool trace_enabled_;
public :
  RouteManager() : first_record(0),new_first_record(0),trace_enabled_(false)
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
