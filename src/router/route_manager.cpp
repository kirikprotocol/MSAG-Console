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
    __trace2__("%s={SRC::%.20s(%d/?%d),n:%d,t:%d}{DST::%.20s(%d/?%d),n:%d,t:%d}",
             ppp,
             record->info.source.value,
             record->info.source.lenght,
             record->src_def,
             record->info.source.plan,
             record->info.source.type,
             record->info.dest.value,
             record->info.dest.lenght,
             record->dest_def,
             record->info.dest.plan,
             record->info.dest.type
               );
  }
  else
  {
    __trace2__("%s=NULL",ppp);
  }
}

inline 
int sort_compare_pat_pat(RouteRecord* pat1, RouteRecord* pat2)
{
//__trace2__("compare (strong)(pattern ? pattern)");
//print(pat1,"\tP1");
//print(pat2,"\tP2");
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = (int32_t)pat1->info.dest.type - (int32_t)pat2->info.dest.type; ifn0goto;
  result = (int32_t)pat1->info.dest.plan - (int32_t)pat2->info.dest.plan; ifn0goto;
  result = (int32_t)pat1->info.dest.lenght - (int32_t)pat2->info.dest.lenght; ifn0goto;
  result = 
    strncmp((char*)pat1->info.dest.value,
            (char*)pat2->info.dest.value,
            min(pat1->dest_def,pat2->dest_def));
  ifn0goto;
  result = (int32_t)pat1->dest_def - (int32_t)pat2->dest_def; ifn0goto;
//---------------------------------------------------------------------//
  result = (int32_t)pat1->info.source.type - (int32_t)pat2->info.source.type; ifn0goto;
  result = (int32_t)pat1->info.source.plan - (int32_t)pat2->info.source.plan; ifn0goto;
  result = (int32_t)pat1->info.source.lenght - (int32_t)pat2->info.source.lenght; ifn0goto;
  result = 
    strncmp((char*)pat1->info.source.value,
            (char*)pat2->info.source.value,
            min(pat1->src_def,pat2->src_def));
  ifn0goto;
  result = (int32_t)pat1->src_def - (int32_t)pat2->src_def; ifn0goto;
result_:
//  __trace2__("(sort)result(P1%cP2)%d",result>0?'>':result<0?'<':'=',
 //     result); 
  return (int32_t)result;
#undef if0ngoto
}

inline 
int compare_addr_addr_src(RouteRecord* rr1,
                      RouteRecord* rr2,
                      bool& strong)
{
//__trace2__("compare (pattern ? pattern)");
//print(rr1,"\tP1");
//print(rr2,"\tP2");
  Address* pat1 = &rr1->info.source;
  Address* pat2 = &rr2->info.source;
  int pat1_def = rr1->src_def;
  int pat2_def = rr2->src_def;
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = (int32_t)pat1->type - (int32_t)pat2->type; ifn0goto;
  result = (int32_t)pat1->plan - (int32_t)pat2->plan; ifn0goto;
  result = (int32_t)pat1->lenght - (int32_t)pat2->lenght; ifn0goto;
  result = 
    strncmp((char*)pat1->value,
            (char*)pat2->value,
            min(pat1_def,pat2_def));
  ifn0goto;
  if ( pat1_def == pat2_def ) strong = true;
  else strong = false;
result_:
  __trace2__("SRC compare result(P1%cP2)%s%d",result>0?'>':result<0?'<':'=',
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
//__trace2__("compare (pattern ? pattern)");
//print(rr1,"\tP1");
//print(rr2,"\tP2");
  Address* pat1 = &rr1->info.dest;
  Address* pat2 = &rr2->info.dest;
  int pat1_def = rr1->dest_def;
  int pat2_def = rr2->dest_def;
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = (int32_t)pat1->type - (int32_t)pat2->type; ifn0goto;
  result = (int32_t)pat1->plan - (int32_t)pat2->plan; ifn0goto;
  result = (int32_t)pat1->lenght - (int32_t)pat2->lenght; ifn0goto;
  result = 
    strncmp((char*)pat1->value,
            (char*)pat2->value,
            min(pat1_def,pat2_def));
  ifn0goto;
  if ( pat1_def == pat2_def ) strong = true;
  else strong = false;
result_:
  __trace2__("DEST compare result(P1%cP2)%s%d",result>0?'>':result<0?'<':'=',
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

/*RouteIterator* RouteManager::iterator()
{
__synchronized__
  throw runtime_error("RouteManager::iterator() unsuppoerted");
  return 0;
}*/

// RouteAdministrator implementaion
void RouteManager::addRoute(const RouteInfo& routeInfo)
{
__synchronized__
  __require__(sme_table);
  auto_ptr<RouteRecord> r(new RouteRecord);
  r->info = routeInfo;
  r->src_def = calcDefLengthAndCheck(&r->info.source);
  r->dest_def = calcDefLengthAndCheck(&r->info.dest);
  r->next = new_first_record;
  r->proxyIdx = sme_table->lookup(r->info.smeSystemId);
  new_first_record = r.release();
}

static 
RouteRecord* findInSrcTreeRecurse(RouteSrcTreeNode* node,RouteRecord* r,int& xcmp )
{
  __trace2__("findInSrcTreeRecurse");
  print(node->record,"\tnode->record");
  //print(r,"\tr");
  bool strong = false;
  int cmp;
  xcmp = compare_addr_addr_src(r,node->record,strong);
  if ( xcmp == 0 )
  {
    if ( !strong )
    {
      __trace2__("weak eqaul");
      RouteRecord* rec;
      int left = 1;
      int right = node->child.size();
      __trace2__("+childs :%d",right);
      
      //for ( int i=0; i < right; ++i ) print(node->child[i]->record,"+@SRC@");
      
      //if ( right > 0 ) 
      for(;right>=left;)
      {
        int ptr = (right+left) >> 1;
        rec = findInSrcTreeRecurse(node->child[ptr-1],r,cmp);
        if ( rec ) return rec;
        __require__( cmp != 0 );
        //if ( right > left ) 
        //{
          if ( cmp < 0 )right = ptr-1;
          else left = ptr+1;
        //}
        //else break;
      }
      return node->record;
    }
    else // strong
        return node->record;
  }
  return 0;
}

static 
RouteRecord* findInTreeRecurse(RouteTreeNode* node,RouteRecord* r,int& xcmp )
{
  __trace2__("findInTreeRecurse");
  print(node->record,"\tnode->record");
  //print(r,"\tr");
  bool strong = false;
  int cmp = 0;
  if ( !node->record ) goto find_child;
  xcmp = compare_addr_addr_dest(r,node->record,strong);
  if ( xcmp == 0 )
  {
    if ( !strong )
    {
      __trace2__("weak");
    find_child:      
      // find by dest     
      RouteRecord* rec;
      int left = 1;
      int right = node->child.size();
      
      __trace2__("dest child: %d",right);
      //for ( int i=0; i < right; ++i ) print(node->child[i]->record,"@DST@");
      
      //if ( right > 0 ) 
      for(;right>=left;)
      {
        int ptr = (right+left) >> 1;
        rec = findInTreeRecurse(node->child[ptr-1],r,cmp);
        if ( rec ) return rec; // found
        //if ( right > left ) 
        //{
          if ( cmp < 0 )right = ptr-1;
          else left = ptr+1;
        //}
        //else return 0; // not found
      }
    }
    else // strong
    {
      goto find_by_source;
    }
    //}
    // else find by source
    //else return 0; // not found
find_by_source:    
    // find by source
    if ( !node->record ) return 0;

    {
      RouteRecord* rec;
      int left = 1;
      int right = node->sources.size();
     // for ( int i=0; i < right; ++i ) print(node->sources[i]->record,"@SRC@");
      __require__ ( right > 0 );
      __trace2__("sorces: %d",right);
      for(;right>=left;)
      {
        int ptr = (right+left) >> 1;
        __require__ ( ptr > 0 );
        rec = findInSrcTreeRecurse(node->sources[ptr-1],r,cmp);
        if ( rec ) return rec;
        //if ( right > left ) 
        //{
          if ( cmp < 0 )right = ptr-1;
          else left = ptr+1;
        //}
        //else return 0; // not found
      }
      //else 
      //{
        //__warning__("very strange, route has no sources");
        //return 0; // ops, no sources !!!!! 
      //}
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
  print(&r,"find value");
  RouteRecord* rec = findInTreeRecurse(node,&r,cmp);
  print(rec,"*** find result ***");
  return rec;
}

inline void print_step(int step)
{
  for(int st=0; st<step;++st) fprintf(stderr,"  ");
}

static
void dump(RouteSrcTreeNode* node,int step)
{
  print_step(step);
  print(node->record,"SRC_NODE rec: ");
  for ( unsigned i=0; i < node->child.size(); ++i )
  {
    print_step(step);
    dump(node->child[i],step+1);
  }
}

static
void dump(RouteTreeNode* node,int step)
{
  print_step(step);
  print(node->record,"DEST_NODE rec: ");
  for ( unsigned i=0; i < node->sources.size(); ++i )
  {
    print_step(step);
    dump(node->sources[i],step+1);
  }
  for ( unsigned i=0; i < node->child.size(); ++i )
  {
    print_step(step);
    dump(node->child[i],step+1);
  }
}

static 
int addRouteIntoSrcTreeRecurse(RouteSrcTreeNode* node,RouteRecord* rec)
{
  __trace2__("addRouteIntoSrcTreeRecurse");
  print(node->record,"\tnode->record");
  print(rec,"\trec");
  bool strong = false;
  int xcmp = compare_addr_addr_src(rec,node->record,strong);
  if ( xcmp == 0 )
  {
    if ( strong )
    {
      __warning__("duplicate route, is not added");
    }
    else //  weak
    {
      __trace2__("week equal");
      int left = 1;
      int right = node->child.size();
      if ( right > 0 ) for(;right>=left;)
      {
        int ptr = (right+left) >> 1;
        int cmp = addRouteIntoSrcTreeRecurse(node->child[ptr-1],rec);
        if ( cmp == 0 ) return 0;
        //if ( right > left ) 
        //{
          if ( cmp < 0 )right = ptr-1;
          else left = ptr+1;
        //}
        //else break;
      }
      __trace2__("*** add src element ***");
      RouteSrcTreeNode* newSrcNode = new RouteSrcTreeNode;
      newSrcNode->record = rec;
      node->child.push_back(newSrcNode);
      return 0;
    }
  }
  return xcmp;
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
  int xcmp = 0;
  if ( !node->record ) goto find_child;
  xcmp = compare_addr_addr_dest(rec,node->record,strong);
  if ( xcmp == 0 )
  {
    if ( strong )
    {
      int left = 1;
      int right = node->sources.size();
      //if ( right > 0 ) 
      __require__(right>0);
      for(;right >= left;)
      {
        int ptr = (right+left) >> 1;
        __require__(ptr > 0);
        int cmp = addRouteIntoSrcTreeRecurse(node->sources[ptr-1],rec);
        if ( cmp == 0 ) return 0;
        //if ( right > left ) 
        //{
          if ( cmp < 0 )right = ptr-1;
          else left = ptr+1;
        //}
        //else break;
      }
      /*else
      {*/
      __trace2__("*** add src element ***");
        RouteSrcTreeNode* newSrcNode = new RouteSrcTreeNode;
        newSrcNode->record = rec;
        node->sources.push_back(newSrcNode);
        return 0;
      //}
    }
    __trace2__("weak equal:")
    if (node->record->dest_def > rec->dest_def) 
    {
      __unreachable__("incorrect tree");
    }
  find_child:
    int left = 1;
    int right = node->child.size();
    if ( right > 0 ) for(;right>=left;)
    {
      int ptr = (right+left) >> 1;
      int cmp = addRouteIntoTreeRecurse(node->child[ptr-1],rec);
      if ( cmp == 0 ) return 0;
      //if ( right > left ) 
      //{
        if ( cmp < 0 )right = ptr-1;
        else left = ptr+1;
      //}
      //else break;
    }
    __trace__("**** add element ****");
    //if ( !strong )
    //{
      RouteTreeNode* newNode = new RouteTreeNode;
      newNode->record = rec;
      node->child.push_back(newNode);
      RouteSrcTreeNode* newSrcNode = new RouteSrcTreeNode;
      newSrcNode->record = rec;
      newNode->sources.push_back(newSrcNode);
    //}
    //else
    //{
    //  RouteSrcTreeNode* newSrcNode = new RouteSrcTreeNode;
    //    newSrcNode->record = rec;
    //  node->sources.push_back(newSrcNode);
    //  cmp = 0;
    //}
  }
  return xcmp;
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
      print(table.get()[i],"@@@");
    }

    for ( int i=0; i < count; ++i )
    {
      if ( table.get()[i] != 0 ) addRouteIntoTree(&root,table.get()[i]);
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
  dump(&root,0);
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
	//if ( info ) *info = 0;
	if ( idx ) *idx = 0;
  proxy = 0;
  __require__(sme_table);
  // ....
  RouteRecord* rec =  findInTree(&root,&source,&dest);
  if ( !rec ) return false;
  proxy = sme_table->getSmeProxy(rec->proxyIdx);
  if ( info ) *info = rec->info;
  if ( idx && rec->info.enabling ) *idx = rec->proxyIdx;
  if (!rc->info.enabling) return false;
	return rec->info.enabling;
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
