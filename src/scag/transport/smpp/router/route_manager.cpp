/*
  $Id$


  4 июля 2003
  изменена схема поиска/хранения маршрутов

  в пределах одного маршрута src->dst добавлен список структур RouteRecord для реализации
  альтернотивного определения прокси назначения по прокси отправителя.
  Данная структура являясь избыточной, тем неменее проста в реализации и имеет минимальный оверхед
  при незначительном использовании такой возможности.

*/

#if !defined DISABLE_TRACING
#define DISABLE_TRACING
#endif

#include "route_manager.h"
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <string>
#include <map>


extern void __qsort__ (void *const pbase, size_t total_elems, size_t size,int(*cmp)(const void*,const void*));

namespace scag {
namespace transport {
namespace smpp {
namespace router {


using std::runtime_error;
using std::auto_ptr;
using std::sort;
using namespace std;
using namespace smsc::sms;

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
             record->info.source.value,
             record->info.source.length,
             record->src_def,
             record->info.source.plan,
             record->info.source.type,
             record->info.dest.value,
             record->info.dest.length,
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

static inline
string AddrToString(const Address& addr)
{
  char buff[128] = {0};
  snprintf(buff,127,".%d.%d.%s",addr.type,addr.plan,addr.value);
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
  result = (int32_t)pat1->info.dest.type - (int32_t)pat2->info.dest.type; ifn0goto;
  result = (int32_t)pat1->info.dest.plan - (int32_t)pat2->info.dest.plan; ifn0goto;
  result = (int32_t)pat1->info.dest.length - (int32_t)pat2->info.dest.length; ifn0goto;
  result =
    strncmp((char*)pat1->info.dest.value,
            (char*)pat2->info.dest.value,
            min(pat1->dest_def,pat2->dest_def));
  ifn0goto;
  result = (int32_t)pat1->dest_def - (int32_t)pat2->dest_def; ifn0goto;
//---------------------------------------------------------------------//
  result = (int32_t)pat1->info.source.type - (int32_t)pat2->info.source.type; ifn0goto;
  result = (int32_t)pat1->info.source.plan - (int32_t)pat2->info.source.plan; ifn0goto;
  result = (int32_t)pat1->info.source.length - (int32_t)pat2->info.source.length; ifn0goto;
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
  Address* pat1 = &rr1->info.dest;
  Address* pat2 = &rr2->info.dest;
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


// RouteAdministrator implementaion
void RouteManager::addRoute(const RouteInfo& routeInfo)
{
  auto_ptr<RouteRecord> r(new RouteRecord);
  r->info = routeInfo;
  r->src_def = calcDefLengthAndCheck(&r->info.source);
  r->dest_def = calcDefLengthAndCheck(&r->info.dest);
  r->proxyIdx = 0;
  if ( r->info.smeSystemId.length() != 0 )
  {
    r->proxyIdx = r->info.smeSystemId.c_str();
  }
  if(r->info.srcSmeSystemId.length()==0)
  {
    r->next = new_first_record;
    new_first_record = r.release();
  }else
  {
    SmeRoute* ptr=smeRoutes.GetPtr(r->info.srcSmeSystemId.c_str());
    if(!ptr)
    {
      ptr=smeRoutes.SetItem(r->info.srcSmeSystemId.c_str(),SmeRoute());
    }
    r->next=ptr->new_first_record;
    ptr->new_first_record=r.release();
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
    std::string res;
    res.reserve(128);
    res=!xcmp?(strong?"strong":"weak  "):"none  ";
    res+=" matching by dest address with tuple {";
    res+=node->record->info.source.toString().c_str();
    res+="} -> {";
    res+=node->record->info.dest.toString().c_str();
    res+='}';
  }
  if ( xcmp == 0 )
  {
    if ( !strong )
    {
      __trace__("weak eqaul");
      RouteRecord* rec;
      int left = 1;
      int right = int(node->child.size());
      __trace2__("+childs :%d",right);

      //for ( int i=0; i < right; ++i ) print(node->child[i]->record,"+@SRC@");

      //if ( right > 0 )
      for(;right>=left;)
      {
        int ptr = (right+left) >> 1;
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
    std::string res;
    res.reserve(128);
    res=!xcmp?(strong?"strong":"weak  "):"none  ";
    res+=" matching by dest address with tuple {";
    res+=node->record->info.source.toString().c_str();
    res+="} -> {";
    res+=node->record->info.dest.toString().c_str();
    res+='}';
    trace_->push_back(res);
  }
  if ( xcmp == 0 )
  {
    if ( !strong )
    {
      __trace__("weak");
    find_child:
      // find by dest
      RouteRecord* rec;
      int left = 1;
      int right = int(node->child.size());

      __trace2__("dest child: %d",right);
      //for ( int i=0; i < right; ++i ) print(node->child[i]->record,"@DST@");

      //if ( right > 0 )
      for(;right>=left;)
      {
        int ptr = (right+left) >> 1;
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
      int left = 1;
      int right = int(node->sources.size());
     // for ( int i=0; i < right; ++i ) print(node->sources[i]->record,"@SRC@");
      __require__ ( right > 0 );
      __trace2__("sorces: %d",right);
      for(;right>=left;)
      {
        int ptr = (right+left) >> 1;
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
  r.info.source = *source;
  r.info.dest = *dest;
  r.src_def = calcDefLengthAndCheck(&r.info.source);
  r.dest_def = calcDefLengthAndCheck(&r.info.dest);
  print(&r,"find value");
  RouteRecord* rec = findInTreeRecurse(node,&r,cmp,trace_);
  print(rec,"*** find result ***");
  return rec;
}

static
void dump(RouteSrcTreeNode* node,int step)
{
  print(node->record,"SRC_NODE rec: ");
  for ( unsigned i=0; i < node->child.size(); ++i )
  {
    dump(node->child[i],step+1);
  }
}

static
void dump(RouteTreeNode* node,int step)
{
  print(node->record,"DEST_NODE rec: ");
  for ( unsigned i=0; i < node->sources.size(); ++i )
  {
    dump(node->sources[i],step+1);
  }
  for ( unsigned i=0; i < node->child.size(); ++i )
  {
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
      dup++;
      if ( trace_ )
      {
        std::string res;
        res.reserve(128);
        res="duplicated route '";
        res+=node->record->info.routeId.c_str();
        res+="':\n    ";
        res+=node->record->info.source.toString().c_str();
        res+='(';
        res+=node->record->info.srcSmeSystemId.c_str();
        res+=") [";
        res+=node->record->info.srcSubj.c_str();
        res+="] ->\n    ";
        res+=node->record->info.dest.toString().c_str();
        res+='(';
        res+=node->record->info.smeSystemId.c_str();
        res+=") [";
        res+=node->record->info.dstSubj.c_str();
        res+=']';
        trace_->push_back(res);
        res="  exists as '";
        res+=rec->info.routeId.c_str();
        res+="':\n    ";
        res+=rec->info.source.toString().c_str();
        res+='(';
        res+=rec->info.srcSmeSystemId.c_str();
        res+=") [";
        res+=rec->info.srcSubj.c_str();
        res+="]  ->\n    ";
        res+=rec->info.dest.toString().c_str();
        res+='(';
        res+=rec->info.smeSystemId.c_str();
        res+=") [";
        res+=rec->info.dstSubj.c_str();
        res+=']';
        trace_->push_back(res);
      }
    }
    else //  weak
    {
      __trace__("week equal");
      int left = 1;
      int right = int(node->child.size());
      if ( right > 0 ) for(;right>=left;)
      {
        int ptr = (right+left) >> 1;
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
      int left = 1;
      int right = int(node->sources.size());
      //if ( right > 0 )
      __require__(right>0);
      for(;right >= left;)
      {
        int ptr = (right+left) >> 1;
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
    int left = 1;
    int right = int(node->child.size());
    if ( right > 0 ) for(;right>=left;)
    {
      int ptr = (right+left) >> 1;
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

void RouteManager::commit(std::vector<std::string>* traceit)
{
  __trace__("commit!");
  RouteRecord** nfr_ptr=&new_first_record;
  RouteRecord** fr_ptr=&first_record;
  RouteTreeNode* root_ptr=&root;
  std::vector<RouteRecord*> table;
  bool main=true;

  char* key;
  SmeRoute* smeRoute;
  smeRoutes.First();
  do{
    if(!main)
    {
      nfr_ptr=&smeRoute->new_first_record;
      fr_ptr=&smeRoute->first_record;
      root_ptr=&smeRoute->root;
    }else
    {
      main=false;
    }

    int count = 0;
    RouteRecord* r = *nfr_ptr;
    for ( ; r != 0; r = r->next ) { ++count; }
    if ( !count ) continue;
    table.clear();
    table.reserve(count);
    r = *nfr_ptr;
    for ( int i=0; i < count; ++i )
    {
      __require__(r != 0);
      table.push_back(r);
      r = r->next;
    }
    __qsort__(&table[0],count,sizeof(RouteRecord*),compare_patterns);
    root_ptr->clean();

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
        addRouteIntoTree(root_ptr,table[i],traceit,dup);
        if(dup)
        {
          DupsMap::iterator it=dups.find(table[i]->info.routeId.str);
          if(it!=dups.end())
          {
            it->second+=dup;
          }else
          {
            dups.insert(DupsMap::value_type(table[i]->info.routeId.str,dup));
          }
        }
      }
    }
    for(DupsMap::iterator it=dups.begin();it!=dups.end();it++)
    {
      __warning2__("%d duplicates found in route %s",it->second,it->first.c_str());
    }
    clear_list(*fr_ptr);
    *fr_ptr= *nfr_ptr;
    *nfr_ptr= 0;
#ifndef DISABLE_TRACING
    dump(&root,0);
#endif
  }while(smeRoutes.Next(key,smeRoute));
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
bool RouteManager::lookup(SmeIndex srcidx,
                          const Address& source,
                          const Address& dest,
                          RouteInfo& info,
                          std::vector<std::string>* traceit )
{
  if ( traceit )
  {
    traceit->push_back(string("lookup for: ")+AddrToString(source)+"("+
      (srcidx?srcidx:string("default"))
      +") -> "+AddrToString(dest));
  }

  RouteRecord* rec=0;
  if(srcidx)
  {
    SmeRoute* ptr=smeRoutes.GetPtr(srcidx);
    if(ptr)
    {
      rec=findInTree(&ptr->root,&source,&dest,traceit);
    }
  }

  if(!rec)
  {
    rec =  findInTree(&root,&source,&dest,traceit);
  }
  if ( !rec )
  {
    if(traceit)
    {
      traceit->push_back("route not found");
    }
    return false;
  }

  if ( traceit )
  {
    std::string res;
    res.reserve(128);
    res="route found, ";
    res+=rec->info.source.toString().c_str();
    res+='(';
    res+=rec->info.srcSmeSystemId.c_str();
    res+=") -> ";
    res+=rec->info.dest.toString().c_str();
    res+='(';
    res+=rec->info.smeSystemId.c_str();
    res+=')';
    traceit->push_back(res);
  }
  info = rec->info;
  return rec->info.enabled;
}

// RoutingTable implementation
bool RouteManager::lookup(const Address& source, const Address& dest,
                          RouteInfo& info,
                          std::vector<std::string>* traceit )
{
  return lookup(0,source,dest,info,traceit);
}

/*
void RouteManager::getTrace(vector<string>& tracelist)
{
  tracelist.swap(trace_);
  vector<string>().swap(trace_);
}

void RouteManager::enableTrace(bool val)
{
  trace_enabled_ = val;
  if ( !trace_enabled_ )
  {
    vector<string>().swap(trace_);
  }
}
 */

}//namespace scag
}//namespace transport
}//namespace smpp
}//namespace router
