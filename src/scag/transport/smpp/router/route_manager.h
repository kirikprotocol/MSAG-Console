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
  std::string dumpIntoString() const;
};

struct RouteSrcTreeNode
{
  RouteRecord* record;
  vector<RouteSrcTreeNode*> child;
  RouteSrcTreeNode() : record(0) {}
  RouteSrcTreeNode(const RouteSrcTreeNode& rhs)
  {
    for ( unsigned i=0; i< rhs.child.size(); ++i )
    {
      if ( rhs.child[i] )
      {
        child.push_back(new RouteSrcTreeNode(*rhs.child[i]));
      }
    }
    record=rhs.record;
  }
  ~RouteSrcTreeNode() {clean();}
  void clean()
  {
    record = 0;
    for ( unsigned i=0; i< child.size(); ++i )
      { if ( child[i] ) delete child[i]; }
    child.clear();
  }
  void dumpInto( std::vector< std::string >& trace,
                 const RouteRecord* oldrec, 
                 const char* pfx ) const;
};

struct RouteTreeNode
{
  RouteRecord* record;
  vector<RouteTreeNode*> child;
  vector<RouteSrcTreeNode*> sources;
  RouteTreeNode() : record(0) {}
  RouteTreeNode(const RouteTreeNode& rhs)
  {
    for ( unsigned i=0; i< rhs.child.size(); ++i )
    {
      if ( rhs.child[i] )
      {
        child.push_back(new RouteTreeNode(*rhs.child[i]));
      }
    }
    for ( unsigned i=0; i< rhs.sources.size(); ++i )
    {
      if(rhs.sources[i])
      {
        sources.push_back(new RouteSrcTreeNode(*rhs.sources[i]));
      }
    }
    record=rhs.record;
  }
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

  void dumpInto( std::vector< std::string >& trace,
                 const RouteRecord* oldrec,
                 const char* pfx ) const;
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
  RouteManager() : first_record(0),new_first_record(0)
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
  void commit( std::vector< std::string >* traceit = 0 );
  void cancel();

  void dumpInto( std::vector<std::string>& trace );

  bool lookup( const smsc::sms::Address& source,
               const smsc::sms::Address& dest,
               RouteInfo& info,
               std::vector<std::string>* traceit );
  bool lookup( SmeIndex srcidx,
               const smsc::sms::Address& source,
               const smsc::sms::Address& dest,
               RouteInfo& info,
               std::vector< std::string >* traceit );
};

}//namespace scag
}//namespace transport
}//namespace smpp
}//namespace router

#endif
