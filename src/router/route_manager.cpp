/*
  $Id$
*/

#if !defined DISABLE_TRACING
//#define DISABLE_TRACING
#endif

#include "route_manager.h"
#include <stdexcept>
#include <memory>
#include <algorithm>

#define __synchronized__

extern void __qsort__ (void *const pbase, size_t total_elems, size_t size,int(*cmp)(const void*,const void*));

namespace smsc {
namespace router{
using std::runtime_error;
using std::auto_ptr;
using std::sort;

int calcDefLengthAndCheck(Address* addr)
{
  int cnt = 0;
  throw_if_fail(addr->lenght<=20);
  for ( int i=0; i < addr->lenght; ++i )
  {
    if ( addr->value[i] == '?' || addr->value[i] == '*' ) break;
    ++cnt;
  }
  return cnt;
}

static inline void print(RouteRecord* record,const char* ppp= "")
{
	if ( record )
	{
		__trace2__("%s={SRC::%20s(%d/?%d)}{DST::%20s(%d/?%d)}",
						 ppp,
						 record->info.source.value,
						 record->info.source.lenght,
						 record->src_def,
						 record->info.dest.value,
						 record->info.dest.lenght,
	 				 record->dest_def);
	}
	else
	{
		__trace2__("%s=NULL",ppp);
	}

}

inline 
int sort_compare_pat_pat(RouteRecord* pat1, RouteRecord* pat2)
{
__trace2__("compare (strong)(pattern ? pattern)");
print(pat1,"\tP1");
print(pat2,"\tP2");
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = pat1->info.dest.type - pat2->info.dest.type; ifn0goto;
  result = pat1->info.dest.plan - pat2->info.dest.plan; ifn0goto;
  result = pat1->info.dest.lenght - pat2->info.dest.lenght; ifn0goto;
  result = 
    strncmp((char*)pat1->info.dest.value,
            (char*)pat2->info.dest.value,
            min(pat1->dest_def,pat2->dest_def));
  ifn0goto;
  result = pat1->dest_def - pat2->dest_def; ifn0goto;
//---------------------------------------------------------------------//
  result = pat1->info.source.type - pat2->info.source.type; ifn0goto;
  result = pat1->info.source.plan - pat2->info.source.plan; ifn0goto;
  result = pat1->info.source.lenght - pat2->info.source.lenght; ifn0goto;
  result = 
    strncmp((char*)pat1->info.dest.value,
            (char*)pat2->info.dest.value,
            min(pat1->src_def,pat2->src_def));
  ifn0goto;
  result = pat1->src_def - pat2->src_def; ifn0goto;
result_:
  __trace2__("(sort)result(P1%cP2)%d",result>0?'>':result<0?'<':'=',
      result); 
  return (int32_t)result;
#undef if0ngoto
}

inline 
int compare_addr_addr_src(RouteRecord* rr1,
                      RouteRecord* rr2,
                      bool& strong)
{
__trace2__("compare (pattern ? pattern)");
print(rr1,"\tP1");
print(rr2,"\tP2");
  Address* pat1 = &rr1->info.source;
  Address* pat2 = &rr2->info.source;
  int pat1_def = rr1->src_def;
  int pat2_def = rr2->src_def;
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = pat1->type - pat2->type; ifn0goto;
  result = pat1->plan - pat2->plan; ifn0goto;
  result = pat1->lenght - pat2->lenght; ifn0goto;
  result = 
    strncmp((char*)pat1->value,
            (char*)pat2->value,
            min(pat1_def,pat2_def));
  ifn0goto;
  if ( pat1_def == pat2_def ) strong = true;
  else strong = false;
result_:
  __trace2__("result(P1%cP2)%s%d",result>0?'>':result<0?'<':'=',
      (result==0)?strong?"(strong)":"(weak)":"",
      result); 
  return (int32_t)result;
#undef if0ngoto
}

inline 
int compare_addr_addr_dest(RouteRecord* rr1,
                      RouteRecord* rr2,
                      bool& strong)
{
__trace2__("compare (pattern ? pattern)");
print(rr1,"\tP1");
print(rr2,"\tP2");
  Address* pat1 = &rr1->info.dest;
  Address* pat2 = &rr2->info.dest;
  int pat1_def = rr1->dest_def;
  int pat2_def = rr2->dest_def;
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = pat1->type - pat2->type; ifn0goto;
  result = pat1->plan - pat2->plan; ifn0goto;
  result = pat1->lenght - pat2->lenght; ifn0goto;
  result = 
    strncmp((char*)pat1->value,
            (char*)pat2->value,
            min(pat1_def,pat2_def));
  ifn0goto;
  if ( pat1_def == pat2_def ) strong = true;
  else strong = false;
result_:
  __trace2__("result(P1%cP2)%s%d",result>0?'>':result<0?'<':'=',
      (result==0)?strong?"(strong)":"(weak)":"",
      result); 
  return (int32_t)result;
#undef if0ngoto
}

int compare_patterns(const void* a1,const void* a2)
{
  //bool strong = true;
  return sort_compare_pat_pat(*(RouteRecord**)a1,*(RouteRecord**)a2);
}

void RouteManager::assign(SmeTable* smetable) // for detach call with NULL
{
__synchronized__
  sme_table = smetable;
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
  auto_ptr<RouteRecord> r(new RouteRecord);
  r->info = routeInfo;
  r->src_def = calcDefLengthAndCheck(&r->info.source);
  r->dest_def = calcDefLengthAndCheck(&r->info.dest);
  r->next = new_first_record;
  new_first_record = r.release();
}

static 
RouteRecord* findInSrcTreeRecurse(RouteSrcTreeNode* node,RouteRecord* r,int& cmp )
{
	__trace2__("findInSrcTreeRecurse");
	print(node->record,"\tnode->record");
	print(r,"\tr");
  bool strong = false;
  cmp = compare_addr_addr_src(r,node->record,strong);
  if ( cmp == 0 )
  {
    if ( !strong )
    {
      RouteRecord* rec;
      int left = 1;
      int right = node->child.size();
      if ( right > 0 ) for(;;)
      {
        int ptr = (right+left) >> 1;
        rec = findInSrcTreeRecurse(node->child[ptr-1],r,cmp);
        if ( rec ) return rec;
        __require__( cmp != 0 );
        if ( right > left ) 
        {
          if ( cmp < 0 )right = ptr-1;
          else left = ptr+1;
        }
        else break;
      }
    }
  }
  return 0;
}

static 
RouteRecord* findInTreeRecurse(RouteTreeNode* node,RouteRecord* r,int& cmp )
{
	__trace2__("findInTreeRecurse");
	print(node->record,"\tnode->record");
	print(r,"\tr");
  bool strong = false;
	if ( !node->record ) goto find_child;
  cmp = compare_addr_addr_dest(r,node->record,strong);
  if ( cmp == 0 )
  {
    if ( !strong )
    {
		find_child:      
			// find by dest     
      RouteRecord* rec;
      int left = 1;
      int right = node->child.size();
      if ( right > 0 ) for(;;)
      {
        int ptr = (right+left) >> 1;
        rec = findInTreeRecurse(node->child[ptr-1],r,cmp);
        if ( rec ) return rec;
        if ( right > left ) 
        {
          if ( cmp < 0 )right = ptr-1;
          else left = ptr+1;
        }
        else break;
      }
    }
    // find by source
    {
      RouteRecord* rec;
      int left = 1;
      int right = node->sources.size();
      if ( right > 0 ) for(;;)
      {
        int ptr = (right+left) >> 1;
        rec = findInSrcTreeRecurse(node->sources[ptr-1],r,cmp);
        if ( rec ) return rec;
        if ( right > left ) 
        {
          if ( cmp < 0 )right = ptr-1;
          else left = ptr+1;
        }
        else break;
      }
    }
  }
  return 0;
}

inline 
RouteRecord* findInTree(RouteTreeNode* node,
                        const Address* source,
                        const Address* dest)
{
  __trace2__("*** findInTree ***");
	int cmp = 0;
  RouteRecord r;
  r.info.source = *source;
  r.info.dest = *dest;
  r.src_def = calcDefLengthAndCheck(&r.info.source);
  r.dest_def = calcDefLengthAndCheck(&r.info.dest);
  RouteRecord* rec = findInTreeRecurse(node,&r,cmp);
	print(rec,"*** find result ***");
	return rec;
}


static 
int addRouteIntoSrcTreeRecurse(RouteSrcTreeNode* node,RouteRecord* rec)
{
	__trace2__("addRouteIntoSrcTreeRecurse");
	print(node->record,"\tnode->record");
	print(rec,"\trec");
  bool strong = false;
  int cmp = compare_addr_addr_src(rec,node->record,strong);
  if ( cmp == 0 )
  {
    if ( !strong )
    {
      __warning__("duplicate route, is not added");
    }
    else
    {
      int left = 1;
      int right = node->child.size();
      if ( right > 0 ) for(;;)
      {
        int ptr = (right+left) >> 1;
        cmp = addRouteIntoSrcTreeRecurse(node->child[ptr-1],rec);
        if ( cmp == 0 ) return 0;
        if ( right > left ) 
        {
          if ( cmp < 0 )right = ptr-1;
          else left = ptr+1;
        }
        else break;
      }
      RouteSrcTreeNode* newSrcNode = new RouteSrcTreeNode;
      newSrcNode->record = rec;
      node->child.push_back(newSrcNode);
      return 0;
    }
  }
  return cmp;
}

static 
int addRouteIntoTreeRecurse(RouteTreeNode* node,RouteRecord* rec)
{
  __require__(node != 0);
	__trace2__("addRouteIntoTreeRecurse");
	print(node->record,"\tnode->record");
	print(rec,"\trec");
  //if ( node->alias ) print(*node->alias,"node->alias");
  //else __trace__("no node->alias");
  //print(rec->alias,"rec->alias");
  
  bool strong = false;
  int cmp = 0;
  if ( !node->record ) goto find_child;
  cmp = compare_addr_addr_dest(rec,node->record,strong);
  if ( cmp == 0 )
  {
    if ( strong )
    {
      int left = 1;
      int right = node->child.size();
      if ( right > 0 ) for(;;)
      {
        int ptr = (right+left) >> 1;
        cmp = addRouteIntoSrcTreeRecurse(node->sources[ptr-1],rec);
        if ( cmp == 0 ) return 0;
        if ( right > left ) 
        {
          if ( cmp < 0 )right = ptr-1;
          else left = ptr+1;
        }
        else break;
      }
    }
    __trace2__("weak equal:")
    if (node->record->dest_def > rec->dest_def) 
    {
      __unreachable__("incorrect tree");
    }
  find_child:
    int left = 1;
    int right = node->child.size();
    if ( right > 0 ) for(;;)
    {
      int ptr = (right+left) >> 1;
      cmp = addRouteIntoTreeRecurse(node->child[ptr-1],rec);
      if ( cmp == 0 ) return 0;
      if ( right > left ) 
      {
        if ( cmp < 0 )right = ptr-1;
        else left = ptr+1;
      }
      else break;
    }
    __trace__("**** add element ****");
    RouteTreeNode* newNode = new RouteTreeNode;
    newNode->record = rec;
    node->child.push_back(newNode);
    RouteSrcTreeNode* newSrcNode = new RouteSrcTreeNode;
    newSrcNode->record = rec;
    newNode->sources.push_back(newSrcNode);
    cmp = 0;
  }
  return cmp;
}

static 
int addRouteIntoTree(RouteTreeNode* node,RouteRecord* rec)
{
  return addRouteIntoTreeRecurse(node,rec);
}

void RouteManager::commit()
{
  __trace2__("commit!");
	int count = 0;
  RouteRecord* r = new_first_record;
  for ( ; r != 0; r = r->next ) { ++count; }
  if ( !count ) return;
  auto_ptr<RouteRecord*> table(new RouteRecord*[count]);
  r = new_first_record;
  for ( int i=0; i < count; ++i )
  {
    __require__(r != 0);
    table.get()[i] = r;
    r = r->next;
  }
  __qsort__(table.get(),count,sizeof(RouteRecord*),compare_patterns);
  {__synchronized__
    root.clean();
    for ( int i=0; i < count; ++i )
    {
      addRouteIntoTree(&root,table.get()[i]);
    }
    while ( first_record )
    {
      RouteRecord* r = first_record;
      first_record = first_record->next;
      delete r;
    }
    first_record = new_first_record;
    new_first_record = 0;
  }
  //delete table;
}

void RouteManager::cancel()
{
  while ( new_first_record )
  {
    RouteRecord* r = new_first_record;
    new_first_record = new_first_record->next;
    delete r;
  }
}

// RoutingTable implementation
bool RouteManager::lookup(const Address& source, const Address& dest, SmeProxy*& proxy, int* idx, RouteInfo* info)
{
__synchronized__
  proxy = 0;
  __require__(sme_table);
  // ....
  RouteRecord* rec =  findInTree(&root,&source,&dest);
  if ( !rec ) return false;
  proxy = sme_table->getSmeProxy(rec->proxyIdx);
  if ( info ) *info = rec->info;
  if ( idx ) *idx = rec->proxyIdx;
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
