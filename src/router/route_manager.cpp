/*
  $Id$
*/

#include "route_manager.h"
#include <stdexcept>
#include <memory>

#define __synchronized__

namespace smsc {
namespace router{
using std::runtime_error;
using std::auto_ptr;

#define is_a(pattern,address) ( compare_pataddr(pattern,address) == 0 )
static inline int compare_pataddr( const RoutePattern& pattern,
                                   const RouteAddress& addr )
{
      __trace2__("compare R(%s->%s) / A(%s->%s)",
                                       pattern.src_addressPattern,
                                       pattern.dest_addressPattern,
                                       addr.src_address,
                                       addr.dest_address
                                               );
      __trace2__("compare R(%lx->%lx) / A(%lx->%lx)",
                                               pattern.src_addressPattern_32[0],
                                               pattern.dest_addressPattern_32[0],
                                               addr.src_address_32[0],
                                               addr.dest_address_32[0]
                                               );
//      __trace2__("--route mask: %llx->%llx",
//                                               *(uint64_t*)(pattern.src_addressMask),
//                                               *(uint64_t*)(pattern.dest_addressMask));
//      __trace2__("--route mask32: %lx->%lx",
//                                               pattern.src_addressMask_32[0],
//                                               pattern.dest_addressMask_32[0]);
#define compare_src(n) \
  (pattern.src_addressPattern_32[n] - \
    (pattern.src_addressMask_32[n] & addr.src_address_32[n]))
#define compare_dest(n) \
  (pattern.dest_addressPattern_32[n] - \
    (pattern.dest_addressMask_32[n] & addr.dest_address_32[n]))
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = pattern.num_n_plan - addr.num_n_plan; ifn0goto;
//  __trace2__("compare src(0): %d ",compare_src(0));
//  __trace2__("compare dest(0): %d ",compare_dest(0));
        result = compare_src(0); ifn0goto;
  result = compare_src(1); ifn0goto;
  result = compare_src(2); ifn0goto;
  result = compare_src(3); ifn0goto;
  result = compare_src(4); ifn0goto;
  result = compare_dest(0); ifn0goto;
  result = compare_dest(1); ifn0goto;
  result = compare_dest(2); ifn0goto;
  result = compare_dest(3); ifn0goto;
  result = compare_dest(4); ifn0goto;
result_:
  __trace2__("=== %d",result);
        return (int32_t)result;
#undef if0ngoto
#undef compare_src
#undef compare_dest
}

static inline int compare_patpat( const RoutePattern& pat1,
                                  const RoutePattern& pat2 )
{
  /*__trace2__("compare R1(%s->%s) ? R2(%s->%s)",
                                         pat1.src_addressPattern,
                                         pat1.dest_addressPattern,
                                         pat2.src_addressPattern,
                                         pat2.dest_addressPattern
                                                 );*/
#define compare_src(n) \
  ( pat1.src_addressPattern_32[n] - pat2.src_addressPattern_32[n] )
#define compare_dest(n) \
  ( pat1.dest_addressPattern_32[n] - pat2.dest_addressPattern_32[n] )
#define ifn0goto {if (result) goto result_;}
  int result;
  result = pat1.num_n_plan - pat2.num_n_plan; ifn0goto;
  result = compare_src(0); ifn0goto;
  result = compare_src(1); ifn0goto;
  result = compare_src(2); ifn0goto;
  result = compare_src(3); ifn0goto;
  result = compare_src(4); ifn0goto;
  result = compare_dest(0); ifn0goto;
  result = compare_dest(1); ifn0goto;
  result = compare_dest(2); ifn0goto;
  result = compare_dest(3); ifn0goto;
  result = compare_dest(4); ifn0goto;
result_:
  //__trace2__("=== %d",result);
  return result;
#undef ifn0goto
#undef compare_src
#undef compare_dest
}

int route_compare(const void* route,const void* pattern)
{
  //__trace2__("%p ? %p",(*(int**)pattern),route);
  return compare_pataddr((**(RouteRecord**)pattern).pattern,*(RouteAddress*)route);
}

int route_pattern_compare(const void* pat1,const void* pat2)
{
        return compare_patpat((**(RouteRecord**)pat2).pattern,(**(RouteRecord**)pat1).pattern);
}

void RouteManager::assign(SmeTable* smetable) // for detach call with NULL
{
__synchronized__
        smeTable = smetable;
}

RouteIterator* RouteManager::iterator()
{
__synchronized__
        throw runtime_error("RouteManager::iterator() unsuppoerted");
        return 0;
}

// RouteAdministrator implementaion
void RouteManager::addRoute(const RouteInfo& routeInfo)
{
__synchronized__
  auto_ptr<RouteRecord> record(new RouteRecord);
  __require__(table_size>=table_ptr);
  __require__(smeTable);

  record->proxyIdx = smeTable->lookup(routeInfo.smeSystemId);
      // throws SmeErr if not found

  record->pattern.src_typeOfNumber = routeInfo.source.getTypeOfNumber();
  record->pattern.dest_typeOfNumber = routeInfo.dest.getTypeOfNumber();
  record->pattern.src_numberingPlan = routeInfo.source.getNumberingPlan();
  record->pattern.dest_numberingPlan = routeInfo.dest.getNumberingPlan();

  uint8_t length;
  int undefVal;
  char addrVal[21];
  char addrPattern[21];

        memset(addrVal,0,sizeof(addrVal));
  undefVal = 20;
  length = routeInfo.source.getValue(addrVal);
        __require__( length < 21 );
  /*if (length>=21) // 20 => 4*5 ( 5 32bit values )
  {
    throw runtime_error("value of source address great then 20 chars");
  }*/
  memset(addrPattern,0,sizeof(addrPattern));
  for ( int i=0; i<length; ++i )
  {
    switch(addrVal[i])
    {
    case '?': // any part
      break;
    case '*': // only end of value
      goto end_src_pattern_loop;
    default:
      --undefVal;
      addrPattern[i] = 0xff;
    }
  }
  end_src_pattern_loop:
  for ( int i=0; i<20; ++i)
  {
    addrVal[i] &= addrPattern[i];
  }
  memcpy(record->pattern.src_addressMask,addrPattern,21);
  memcpy(record->pattern.src_addressPattern,addrVal,21);
  record->src_pattern_undef = undefVal;

  undefVal = 20;
  length = routeInfo.dest.getValue(addrVal);
        __require__( length < 21 );
  /*if (length>=21) // 20 => 4*5 ( 5 32bit values )
  {
    throw runtime_error("value of dest address great then 20 chars");
  }*/
  memset(addrPattern,0,sizeof(addrPattern));
  for ( int i=0; i<20; ++i )
  {
    switch(addrVal[i])
    {
    case '?': // any part
      break;
    case '*': // only end of value
      goto end_dest_pattern_loop;
    default:
      --undefVal;
      addrPattern[i] = 0xff;
    }
  }
  end_dest_pattern_loop:
  for ( int i=0; i<20; ++i)
  {
    addrVal[i] &= addrPattern[i];
  }
  memcpy(record->pattern.dest_addressMask,addrPattern,21);
  memcpy(record->pattern.dest_addressPattern,addrVal,21);
  record->dest_pattern_undef = undefVal;

  if ( table_size == table_ptr )
  {
    RouteRecord** tmp = new RouteRecord*[table_size+1024];
    memcpy(tmp,table,sizeof(RouteRecord*)*table_size);
    delete table;
    table = tmp;
  }

  table[table_ptr++] = record.release();
        //__trace2__("+%p",table[table_ptr-1]);
        //if ( table_ptr > 1 )
        //__trace2__("prev route: %s->%s",
        //                                       table[table_ptr-2]->pattern.src_addressPattern,
        //                                       table[table_ptr-2]->pattern.dest_addressPattern);
        //__trace2__("add route mask: %llx->%llx",
        //                                       *(uint64_t*)(table[table_ptr-1]->pattern.src_addressMask),
        //                                       *(uint64_t*)(table[table_ptr-1]->pattern.dest_addressMask));
  __trace2__("add route: %s->%s",
             (table[table_ptr-1]->pattern.src_addressPattern),
             (table[table_ptr-1]->pattern.dest_addressPattern));
  sorted = false;
}

static inline void makeAddress( RouteAddress* addr, const Address* source, const Address* dest)
{
        uint8_t length;
        memset(addr,0,sizeof(RouteAddress));
  addr->src_typeOfNumber = source->getTypeOfNumber();
  addr->dest_typeOfNumber = dest->getTypeOfNumber();
        addr->src_numberingPlan = source->getNumberingPlan();
  addr->dest_numberingPlan = dest->getNumberingPlan();
        length = source->getValue((char*)addr->src_address);
        __require__( length < 21 );
        length = dest->getValue((char*)addr->dest_address);
        __require__( length < 21 );
}

// RoutingTable implementation
bool RouteManager::lookup(const Address& source, const Address& dest, SmeProxy*& proxy, int* idx, RouteInfo* info)
{
__synchronized__
  __require__(smeTable);
        if ( !table_ptr ) return false;
        if (!sorted)
  {
    qsort(table,table_ptr,sizeof(RouteRecord*),route_pattern_compare);
    sorted = true;
  }
  RouteAddress address;
  proxy = 0;
  makeAddress(&address,&source,&dest);
  __trace2__("lookup route %s->%s",address.src_address,address.dest_address);
  //RouteRecord* record = bsearch_record(&address,table,table_ptr);
  RouteRecord** recordX = (RouteRecord**)bsearch(
                        &address,table,table_ptr,sizeof(RouteRecord*),route_compare);
  __trace2__("xroute == %p",recordX);
        if (!recordX) return false;
        RouteRecord* record = *recordX;
  record->ok_next = 0;
  RouteRecord* ok_route = record;
  __trace2__("found route %s->%s",
             ok_route->pattern.src_addressPattern,
             ok_route->pattern.dest_addressPattern);
  for (RouteRecord** r = recordX-1; r != table-1; --r )
  {
    if ( is_a((*r)->pattern,address) )
    {
      (*r)->ok_next = ok_route;
      ok_route = *r;
    }else break;
  }
  for (RouteRecord** r = recordX+1; r != table+table_ptr; ++r )
  {
    if ( is_a((*r)->pattern,address) )
    {
      (*r)->ok_next = ok_route;
      ok_route = *r;
    }else break;
  }
  record = 0;
  int src_undef = 1000;
  int dest_undef = 1000;
  if ( ok_route->ok_next ) // has more then one routes
  {
    while ( ok_route )
    {
      if ( ok_route->src_pattern_undef < src_undef )
      {
        src_undef = ok_route->src_pattern_undef;
        record = ok_route;
        ok_route = ok_route->ok_next;
      }
      else if ( ok_route->src_pattern_undef == src_undef )
                        {
        RouteRecord* tmp = ok_route->ok_next;
        ok_route->ok_next = record;
        record = ok_route;
        ok_route = tmp;
      }
    }
    __require__(record);
    ok_route = record;
    record = 0;
    while ( ok_route )
    {
      if ( ok_route->dest_pattern_undef < dest_undef )
      {
        dest_undef = ok_route->dest_pattern_undef;
        record = ok_route;
        ok_route = ok_route->ok_next;
      }
      else if ( ok_route->src_pattern_undef == src_undef )
      {
        RouteRecord* tmp = ok_route->ok_next;
        ok_route->ok_next = record;
        record = ok_route;
        ok_route = tmp;
      }
    }
    __require__(record);
                if ( record->ok_next )
                {
                        __warning__("more then one route found, use anyone");
                }
  }
        else
        {
                record = ok_route;
        }
  proxy = smeTable->getSmeProxy(record->proxyIdx);
  if ( info ) *info = record->info;
        if ( idx ) *idx = record->proxyIdx;
  return true;
}

/*RouteInfo RouteManager::getRouteInfo(int idx)
{
  return records.at(idx);
}

SmeProxy* RouteManager::getSmeProxy(int idx)
{
}*/

}; // namespace router
}; // namespace smsc
