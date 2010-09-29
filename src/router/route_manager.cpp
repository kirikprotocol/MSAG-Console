/*
  $Id$


  4 ���� 2003
  �������� ����� ������/�������� ���������

  � �������� ������ �������� src->dst �������� ������ �������� RouteRecord ��� ����������
  ��������������� ����������� ������ ���������� �� ������ �����������.
  ������ ��������� ������� ����������, ��� ������� ������ � ���������� � ����� ����������� �������
  ��� �������������� ������������� ����� �����������.

*/

#if !defined DISABLE_TRACING
#define DISABLE_TRACING
#endif

#include "route_manager.h"
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <string>
#include <sstream>
#include <map>

#define __synchronized__

extern void __qsort__ (void *const pbase, size_t total_elems, size_t size,int(*cmp)(const void*,const void*));

namespace smsc {
namespace router{
using std::runtime_error;
using std::auto_ptr;
using std::sort;
using namespace std;

bool RouteManager::smeRoutersEnabled=false;

int calcDefLengthAndCheck(Address* addr)
{
  int cnt = 0;
  throw_if_fail(addr->length<=20);
  for ( int i=0; i < addr->length; ++i )
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
             record->rp.source.value,
             record->rp.source.length,
             record->src_def,
             record->rp.source.plan,
             record->rp.source.type,
             record->rp.dest.value,
             record->rp.dest.length,
             record->dest_def,
             record->rp.dest.plan,
             record->rp.dest.type
               );
  }
  else
  {
    __trace2__("%s=NULL",ppp);
  }
}

static inline
string AddrToString(const Address& addr)
{
  char buff[128] = {0};
  snprintf(buff,127,".%d.%d.%s\x00",addr.type,addr.plan,addr.value);
  return string(buff);
}

inline
int sort_compare_pat_pat(RouteRecord* pat1, RouteRecord* pat2)
{
//__trace2__("compare (strong)(pattern ? pattern)");
//print(pat1,"\tP1");
//print(pat2,"\tP2");
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = (int32_t)pat1->rp.dest.type - (int32_t)pat2->rp.dest.type; ifn0goto;
  result = (int32_t)pat1->rp.dest.plan - (int32_t)pat2->rp.dest.plan; ifn0goto;
  result = (int32_t)pat1->rp.dest.length - (int32_t)pat2->rp.dest.length; ifn0goto;
  result =
    strncmp((char*)pat1->rp.dest.value,
            (char*)pat2->rp.dest.value,
            min(pat1->dest_def,pat2->dest_def));
  ifn0goto;
  result = (int32_t)pat1->dest_def - (int32_t)pat2->dest_def; ifn0goto;
//---------------------------------------------------------------------//
  result = (int32_t)pat1->rp.source.type - (int32_t)pat2->rp.source.type; ifn0goto;
  result = (int32_t)pat1->rp.source.plan - (int32_t)pat2->rp.source.plan; ifn0goto;
  result = (int32_t)pat1->rp.source.length - (int32_t)pat2->rp.source.length; ifn0goto;
  result =
    strncmp((char*)pat1->rp.source.value,
            (char*)pat2->rp.source.value,
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
  Address* pat1 = &rr1->rp.source;
  Address* pat2 = &rr2->rp.source;
  int pat1_def = rr1->src_def;
  int pat2_def = rr2->src_def;
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = (int32_t)pat1->type - (int32_t)pat2->type; ifn0goto;
  result = (int32_t)pat1->plan - (int32_t)pat2->plan; ifn0goto;
  result = (int32_t)pat1->length - (int32_t)pat2->length; ifn0goto;
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
  Address* pat1 = &rr1->rp.dest;
  Address* pat2 = &rr2->rp.dest;
  int pat1_def = rr1->dest_def;
  int pat2_def = rr2->dest_def;
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = (int32_t)pat1->type - (int32_t)pat2->type; ifn0goto;
  result = (int32_t)pat1->plan - (int32_t)pat2->plan; ifn0goto;
  result = (int32_t)pat1->length - (int32_t)pat2->length; ifn0goto;
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
void RouteManager::addRoute(RouteInfo* routeInfo,const RoutePoint& rp)
{
__synchronized__
  __require__(sme_table);
  auto_ptr<RouteRecord> r(new RouteRecord);
  int srcProxyIdx=-1;
  if ( routeInfo->srcSmeSystemId.length() != 0 )
  {
    srcProxyIdx = sme_table->lookup(routeInfo->srcSmeSystemId);
  }
  if(smeRoutersEnabled && srcProxyIdx!=-1)
  {
    if(smeRouters[srcProxyIdx]==0)
    {
      smeRouters[srcProxyIdx]=new SmeRouter();
      if(srcProxyIdx>maxSmeRouter)
      {
        maxSmeRouter=srcProxyIdx;
      }
    }
  }
  r->info = routeInfo;
  r->rp=rp;
  r->src_def = calcDefLengthAndCheck(&r->rp.source);
  r->dest_def = calcDefLengthAndCheck(&r->rp.dest);
  r->next = smeRoutersEnabled && srcProxyIdx!=-1?smeRouters[srcProxyIdx]->new_first_record:new_first_record;
  r->proxyIdx = r->srcProxyIdx = -1;
  r->alternate_pair = 0;
  if ( r->info->smeSystemId.length() != 0 )
  {
    r->proxyIdx = sme_table->lookup(r->info->smeSystemId);
  }
  if ( r->info->srcSmeSystemId.length() != 0 )
  {
    r->srcProxyIdx = srcProxyIdx;
  }
  if(smeRoutersEnabled && srcProxyIdx!=-1)
  {
    smeRouters[srcProxyIdx]->new_first_record=r.release();
  }else
  {
    new_first_record = r.release();
  }
}

static
RouteRecord* findInSrcTreeRecurse(RouteSrcTreeNode* node,RouteRecord* r,int& xcmp,vector<string>* trace_ )
{
  __trace__("findInSrcTreeRecurse");
  print(node->record,"\tnode->record");
  //print(r,"\tr");
  bool strong = false;
  int cmp;
  xcmp = compare_addr_addr_src(r,node->record,strong);
  if (trace_)
  {
    ostringstream ost;
    ost << (!xcmp?(strong?"strong":"weak  "):"none  ")
      << " matching by source address with tuple {"
      << AddrToString(node->record->rp.source) << "} -> "
      << AddrToString(node->record->rp.dest);
    trace_->push_back(ost.str());
  }
  if ( xcmp == 0 )
  {
    if ( !strong )
    {
      __trace__("weak eqaul");
      RouteRecord* rec;
      size_t left = 1;
      size_t right = node->child.size();
      __trace2__("+childs :%d",right);

      //for ( int i=0; i < right; ++i ) print(node->child[i]->record,"+@SRC@");

      //if ( right > 0 )
      for(;right>=left;)
      {
        size_t ptr = (right+left) >> 1;
        rec = findInSrcTreeRecurse(node->child[ptr-1],r,cmp,trace_);
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
RouteRecord* findInTreeRecurse(RouteTreeNode* node,RouteRecord* r,int& xcmp,vector<string>* trace_ )
{
  __trace__("findInTreeRecurse");
  print(node->record,"\tnode->record");
  //print(r,"\tr");
  bool strong = false;
  int cmp = 0;
  if ( !node->record ) goto find_child;
  xcmp = compare_addr_addr_dest(r,node->record,strong);
  if (trace_)
  {
    ostringstream ost;
    ost << (!xcmp?(strong?"strong":"weak  "):"none  ")
      << " matching by dest address with tuple "
      << AddrToString(node->record->rp.source) << " -> {"
      << AddrToString(node->record->rp.dest) << "}";
    trace_->push_back(ost.str());
  }
  if ( xcmp == 0 )
  {
    if ( !strong )
    {
      __trace__("weak");
    find_child:
      // find by dest
      RouteRecord* rec;
      size_t left = 1;
      size_t right = node->child.size();

      __trace2__("dest child: %d",right);
      //for ( int i=0; i < right; ++i ) print(node->child[i]->record,"@DST@");

      //if ( right > 0 )
      for(;right>=left;)
      {
        size_t ptr = (right+left) >> 1;
        rec = findInTreeRecurse(node->child[ptr-1],r,cmp,trace_);
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
      size_t left = 1;
      size_t right = node->sources.size();
     // for ( int i=0; i < right; ++i ) print(node->sources[i]->record,"@SRC@");
      __require__ ( right > 0 );
      __trace2__("sorces: %d",right);
      for(;right>=left;)
      {
        size_t ptr = (right+left) >> 1;
        __require__ ( ptr > 0 );
        rec = findInSrcTreeRecurse(node->sources[ptr-1],r,cmp,trace_);
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
                        const Address* dest,
                        vector<string>* trace_)
{
  __trace__("*** findInTree ***");
  int cmp = 0;
  RouteRecord r;
  r.rp.source = *source;
  r.rp.dest = *dest;
  r.src_def = calcDefLengthAndCheck(&r.rp.source);
  r.dest_def = calcDefLengthAndCheck(&r.rp.dest);
  print(&r,"find value");
  RouteRecord* rec = findInTreeRecurse(node,&r,cmp,trace_);
  print(rec,"*** find result ***");
  return rec;
}

inline void print_step(int step)
{
#ifndef DISABLE_TRACING
  //for(int st=0; st<step;++st) fprintf(stderr,"  ");
#endif
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
int addRouteIntoSrcTreeRecurse(RouteSrcTreeNode* node,RouteRecord* rec,vector<string>* trace_,int& dup)
{
  __trace__("addRouteIntoSrcTreeRecurse");
  print(node->record,"\tnode->record");
  print(rec,"\trec");
  bool strong = false;
  int xcmp = compare_addr_addr_src(rec,node->record,strong);
  if ( xcmp == 0 )
  {
    if ( strong )
    {
      bool conflicted = false;
      for ( RouteRecord* r0 = node->record; r0 != 0 ; r0 = r0->alternate_pair ) {
        if ( r0->srcProxyIdx == rec->srcProxyIdx ) {
          //__warning2__("duplicate route %s, is not added",rec->info->routeId.c_str());
          dup++;
          if ( trace_ ) {
            {
              ostringstream ost;
              ost << "duplicated route '"
                << r0->info->routeId << "':\n"
                << "    " << AddrToString(r0->rp.source) << "(" << r0->info->srcSmeSystemId.c_str() << ")"
                << " [" << r0->info->srcSubj << "] "
                << " ->\n"
                << "    " << AddrToString(r0->rp.dest) << "(" << r0->info->smeSystemId.c_str() << ")"
                << " [" << r0->info->dstSubj << "]";
              trace_->push_back(ost.str());
            }
            {
              ostringstream ost;
              ost << "  exists as '"
                << rec->info->routeId << "':\n"
                << "    " <<AddrToString(rec->rp.source) << "(" << rec->info->srcSmeSystemId.c_str() << ")"
                << " [" << rec->info->srcSubj << "] "
                << " ->\n"
                << "    " <<AddrToString(rec->rp.dest) << "(" << rec->info->smeSystemId.c_str() << ")"
                << "[" << rec->info->dstSubj << "]";
              trace_->push_back(ost.str());
            }
          }
          conflicted = true;
        }
      }
      if ( !conflicted ) {
        __trace__("*** add alternate src proxy ***");
        rec->alternate_pair = node->record;
        node->record = rec;
        return 0;
      }
    }
    else //  weak
    {
      __trace__("week equal");
      size_t left = 1;
      size_t right = node->child.size();
      if ( right > 0 ) for(;right>=left;)
      {
        size_t ptr = (right+left) >> 1;
        int cmp = addRouteIntoSrcTreeRecurse(node->child[ptr-1],rec,trace_,dup);
        if ( cmp == 0 ) return 0;
        //if ( right > left )
        //{
          if ( cmp < 0 )right = ptr-1;
          else left = ptr+1;
        //}
        //else break;
      }
      __trace__("*** add src element ***");
      RouteSrcTreeNode* newSrcNode = new RouteSrcTreeNode;
      newSrcNode->record = rec;
      node->child.push_back(newSrcNode);
      return 0;
    }
  }
  return xcmp;
}

static
int addRouteIntoTreeRecurse(RouteTreeNode* node,RouteRecord* rec,vector<string>* trace_,int& dup)
{
  __require__(node != 0);
  __trace__("addRouteIntoTreeRecurse");
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
      size_t left = 1;
      size_t right = node->sources.size();
      //if ( right > 0 )
      __require__(right>0);
      for(;right >= left;)
      {
        size_t ptr = (right+left) >> 1;
        __require__(ptr > 0);
        int cmp = addRouteIntoSrcTreeRecurse(node->sources[ptr-1],rec,trace_,dup);
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
      __trace__("*** add src element ***");
        RouteSrcTreeNode* newSrcNode = new RouteSrcTreeNode;
        newSrcNode->record = rec;
        node->sources.push_back(newSrcNode);
        return 0;
      //}
    }
    __trace__("weak equal:");
    if (node->record->dest_def > rec->dest_def)
    {
      __unreachable__("incorrect tree");
    }
  find_child:
    size_t left = 1;
    size_t right = node->child.size();
    if ( right > 0 ) for(;right>=left;)
    {
      size_t ptr = (right+left) >> 1;
      int cmp = addRouteIntoTreeRecurse(node->child[ptr-1],rec,trace_,dup);
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
int addRouteIntoTree(RouteTreeNode* node,RouteRecord* rec,vector<string>* trace_,int& dup)
{
  return addRouteIntoTreeRecurse(node,rec,trace_,dup);
}

void RouteManager::commit(bool traceit)
{
  __trace__("commit!");

  for(std::set<const RouteInfo*>::iterator it=infoSet.begin(),end=infoSet.end();it!=end;++it)
  {
    delete *it;
  }
  infoSet.clear();

  int idx=0;
  bool main=true;
  std::vector<RouteRecord*> table;

  while(main || idx<=maxSmeRouter)
  {
    if(!main && !smeRouters[idx])
    {
      idx++;
      continue;
    }
    RouteRecord*& nfr=main?new_first_record:smeRouters[idx]->new_first_record;
    int count = 0;
    RouteRecord* r = nfr;
    for ( ; r != 0; r = r->next ) { ++count; }
    if ( !count )
    {
      if(main)
      {
        main=false;
      }else
      {
        idx++;
      }
      continue;
    }
    table.clear();
    table.resize(count);
    r = nfr;
    for ( int i=0; i < count; ++i )
    {
      __require__(r != 0);
      table[i] = r;
      infoSet.insert(r->info);
      r = r->next;
    }
    __qsort__(&table[0],count,sizeof(RouteRecord*),compare_patterns);

    {__synchronized__
      RouteTreeNode& rootRef=main?root:smeRouters[idx]->root;
      rootRef.clean();

      for ( int i=0; i < count; ++i )
      {
        print(table[i],"@@@");
      }

      typedef std::map<std::string,int> DupsMap;
      DupsMap dups;

      for ( int i=0; i < count; ++i )
      {
        if ( table[i] != 0 )
        {
          int dup=0;
          addRouteIntoTree(&rootRef,table[i],traceit?&trace_:0,dup);
          if(dup)
          {
            DupsMap::iterator it=dups.find(table[i]->info->routeId);
            if(it!=dups.end())
            {
              it->second+=dup;
            }else
            {
              dups.insert(DupsMap::value_type(table[i]->info->routeId,dup));
            }
          }
        }
      }
      for(DupsMap::iterator it=dups.begin();it!=dups.end();it++)
      {
        __warning2__("%d duplicates found in route %s",it->second,it->first.c_str());
      }
      RouteRecord*& fr=main?first_record:smeRouters[idx]->first_record;
      ClearList(fr);
      fr=nfr;
      nfr=0;
    }
    if(main)
    {
      main=false;
    }else
    {
      idx++;
    }
  }
#ifndef DISABLE_TRACING
  dump(&root,0);
#endif
  //delete table;
}

void RouteManager::cancel()
{
  ClearList(new_first_record);
  for(int i=0;i<=maxSmeRouter;i++)
  {
    if(smeRouters[i])
    {
      ClearList(smeRouters[i]->new_first_record);
    }
  }
}

// RoutingTable implementation
bool RouteManager::lookup(int srcidx, const Address& source, const Address& dest, RouteResult& rr)
{
__synchronized__
  //if ( info ) *info = 0;
  rr.destSmeIdx = -1;
  rr.destProxy = 0;
  rr.found = false;
  __require__(sme_table);
  // ....

  RouteTreeNode* rootPtr=srcidx!=-1 && smeRouters[srcidx]?&smeRouters[srcidx]->root:&root;

  smsc::smeman::SmeInfo src_smeinfo;
  if(trace_enabled_)
  {
    if ( srcidx!=-1 )
    {
      src_smeinfo = sme_table->getSmeInfo(srcidx);
    }
  }

  if ( trace_enabled_ )
  {
    trace_.push_back(string("lookup for: ")+AddrToString(source)+"("+
      (srcidx!=-1?src_smeinfo.systemId:string("default"))
      +") -> "+AddrToString(dest));
  }

  RouteRecord* rec =  findInTree(rootPtr,&source,&dest,trace_enabled_?&trace_:0);
  if ( !rec )
  {
    if(trace_enabled_)
    {
      trace_.push_back("route not found");
    }
    return false;
  }
  // ��������� �� 4 ���� 2003, ���� �������������� �������
  RouteRecord* rec0 = 0;

  if ( trace_enabled_ )
  {
    string s("lookup for alternative route with src proxy: '");
    if ( srcidx!=-1 ) {
      s += src_smeinfo.systemId;
    }else{
      s += "default";
    }
    s += "'";
    trace_.push_back(s);
  }

  for ( ; rec != 0 ; rec = rec->alternate_pair )
  {
    if ( trace_enabled_ )
    {
      ostringstream ost;
      ost << "check alternative route with src proxy: '" <<
        (rec->srcProxyIdx!=-1?rec->info->srcSmeSystemId.c_str():"default") << "'";
      trace_.push_back(ost.str());
    }
    if ( srcidx != -1 && rec->srcProxyIdx == srcidx )
    {
      if ( trace_enabled_ )
      {
        ostringstream ost;
        ost << "found alternative route with src proxy: '" << rec->info->srcSmeSystemId.c_str() << "'";
        trace_.push_back(ost.str());
      }
      break;
    }
    if ( rec->srcProxyIdx == -1 )
    {
      if ( trace_enabled_ )
      {
        trace_.push_back("found default src proxy for route");
      }
      rec0 = rec;
    }
  }

  if ( !rec ) rec = rec0;
  if ( !rec )
  {
    if ( trace_enabled_ )
    {
      trace_.push_back("src proxy matching not found");
    }
    return false; // �� ������
  }

  rr.destProxy = sme_table->getSmeProxy(rec->proxyIdx);

  if ( trace_enabled_ )
  {
    ostringstream ost;
    ost << "route found, "
      << AddrToString(rec->rp.source) << "(" << rec->info->srcSmeSystemId.c_str() << ") -> "
      << AddrToString(rec->rp.dest) << "(" << rec->info->smeSystemId.c_str() << ")";
    trace_.push_back(ost.str());
  }
  rr.info = *rec->info;
  rr.found=true;
  rr.destSmeIdx = rec->proxyIdx;
  rr.rp=rec->rp;
  if (rec->info->trafMode==tmNone)
  {
    if(trace_enabled_)
    {
      trace_.push_back("Route disabled");
    }
    return false;
  }
  return true;
}

// RoutingTable implementation
bool RouteManager::lookup(const Address& source, const Address& dest, RouteResult& rr)
{
  return lookup(-1,source,dest,rr);
}

void RouteManager::getTrace(vector<string>& tracelist)
{
  tracelist.swap(trace_);
  vector<string>().swap(trace_);
}

void RouteManager::enableTrace(bool val)
{
  trace_enabled_ = val;
  if ( !trace_enabled_ ) vector<string>().swap(trace_);
}


/*RouteInfo RouteManager::getRouteInfo(int idx)
{
  return records.at(idx);
}

SmeProxy* RouteManager::getSmeProxy(int idx)
{
}*/

} // namespace router
} // namespace smsc
