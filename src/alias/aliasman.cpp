/*
  $Id$
*/
#if !defined (DISABLE_TRACING)
#define DISABLE_TRACING
#endif
#include "aliasman.h"
#include <stdexcept>
#include <string>
#include <stdlib.h>
#include "util/debug.h"
#include <memory>
#include <vector>
#include <algorithm>

//#define ENTER __trace2__("enter in %s",__PRETTY_FUNCTION__)
//#define LEAVE __trace2__("leave from %s",__PRETTY_FUNCTION__)
//#define LEAVE_(result) {LEAVE; __watch__(result); return result;}
#define ENTER
#define LEAVE
#define LEAVE_(result) return result;
#define DUMP_ARRAY 0

extern void __qsort__ (void *const pbase, size_t total_elems, size_t size,int(*cmp)(const void*,const void*));

namespace smsc{
namespace alias{

using namespace smsc::sms;
using namespace std;

#define __synchronized__
inline
void print(const APattern& pattern,char* text)
{
  __trace2__("#### %s = PATTERN {%s(len:%d/def:%d), npi: %d, ton: %d, num_n_plan: %d}",
              text,
              pattern.value,
              pattern.length,
              pattern.defLength,
              pattern.numberingPlan,
              pattern.typeOfNumber,
              pattern.num_n_plan);
  /*__trace2__("---- val: %x.%x.%x.%x.%x mask %x.%x.%x.%x.%x -----",
             pattern.value_32[0],
             pattern.value_32[1],
             pattern.value_32[2],
             pattern.value_32[3],
             pattern.value_32[4],
             pattern.mask_32[0],
             pattern.mask_32[1],
             pattern.mask_32[2],
             pattern.mask_32[3],
             pattern.mask_32[4]
             );*/
}
void print(const AValue& val,char* text)
{
  __trace2__("#### %s = VALUE {%s(len:%d), npi: %d, ton: %d, num_n_plan: %d}",
              text,
              val.value,
              val.length,
              val.numberingPlan,
              val.typeOfNumber,
              val.num_n_plan);
  __trace2__("---- val: %x.%x.%x.%x.%x -----",
             val.value_32[0],
             val.value_32[1],
             val.value_32[2],
             val.value_32[3],
             val.value_32[4]
             );
}
inline
int compare_val_pat(const AValue& val, const APattern& pattern,bool& strong)
{
  __trace2__("compare (value ? pattern)");
  print(val,"\tV:");
  print(pattern,"\tP:");
/*#define compare_v(n) \
  ((pattern.mask_32[n]&val.value_32[n]) - pattern.value_32[n])*/
#define ifn0goto {if ( result ) goto result_; }
  int32_t result;
  result = val.num_n_plan-pattern.num_n_plan; ifn0goto;
  result = val.length - pattern.length; ifn0goto;
/*  result = compare_v(0); ifn0goto;
  result = compare_v(1); ifn0goto;
  result = compare_v(2); ifn0goto;
  result = compare_v(3); ifn0goto;
  result = compare_v(4); ifn0goto; */
  result =
    strncmp((char*)val.value,(char*)pattern.value,pattern.defLength);
    ifn0goto;
  if ( pattern.defLength == val.length )
    strong = true;
  else
    strong = false;
result_:
  __trace2__("result(V%cP)%s%d",result>0?'>':result<0?'<':'=',(result==0)?strong?"(strong)":"(weak)":"",result);
  return (int32_t)result;
#undef ifn0goto
#undef compare_v
}
static inline int compare_pat_pat( const APattern& pat1,
                                  const APattern& pat2,
                                  bool& strong )
{
__trace2__("compare (%s)(pattern ? pattern)",strong?"strong":"weak");
print(pat1,"\tP1");
print(pat2,"\tP2");
#define ifn0goto {if (result) goto result_;}
int32_t result;
  result = pat1.num_n_plan - pat2.num_n_plan; ifn0goto;
  result = pat1.length - pat2.length; ifn0goto;
if ( strong )
{
/*#define compare_v(n) (pat1.value_32[n] - pat2.value_32[n] )
  result = compare_v(0); ifn0goto;
  result = compare_v(1); ifn0goto;
  result = compare_v(2); ifn0goto;
  result = compare_v(3); ifn0goto;
  result = compare_v(4); ifn0goto;
#undef compare_v*/
/*  for ( int i=0; i < min(pat1.defLength,pat2.defLength); ++i)
  {
    if ( pat1.value[i] != pat2.value[i] )
    {
      result = pat1.value[i] > pat2.value[i] ? 1 : -1;
      goto result_;
    }
  } */
  result =
    strncmp((char*)pat1.value,(char*)pat2.value,min(pat1.defLength,pat2.defLength));
    ifn0goto;
  result = pat1.defLength - pat2.defLength; ifn0goto;
}
else
{
//#define compare_v(n) (pat1.value_32[n]&pat2.mask_32[n] - pat2.value_32[n]&pat1.mask_32[n] )
/*  result = compare_v(0); ifn0goto;
  result = compare_v(1); ifn0goto;
  result = compare_v(2); ifn0goto;
  result = compare_v(3); ifn0goto;
  result = compare_v(4); ifn0goto; */
  result =
    strncmp((char*)pat1.value,(char*)pat2.value,min(pat1.defLength,pat2.defLength));
    ifn0goto;
  if ( pat1.defLength == pat2.defLength ) strong = true;
//#undef compare_v
}
result_:
  __trace2__("result(P1%cP2)%s%d",result>0?'>':result<0?'<':'=',
      (result==0)?strong?"(strong)":"(weak)":"",
      result);
  return (int32_t)result;
#undef if0ngoto
}

static TreeNode* findNodeByAliasRecurse(TreeNode* node,AValue& val,int& cmp)
{
  ENTER;
  __trace2__("node: %p",node);
  bool strong = false;
  __require__(node != 0);
  if (!node->alias ) {cmp = 0; goto find_child;}
  cmp =  compare_val_pat(val,*(node->alias),strong);
  if ( strong ) // cmp == 0
    LEAVE_(node);
  if ( cmp == 0 ) // weak
  {
  find_child:
    /*
      fix me here, change to binary search
    */
    //__watch__(node->child.size());
    /*for ( int i=0; i<node->child.size(); ++i )
    {
      TreeNode* res = findNodeByAliasRecurse(node->child[i],val,cmp);
      if ( res ) return res;
    }*/
    #if DUMP_ARRAY
    __trace2__("array has %d elements",node->child.size());
    for ( unsigned i=0; i<node->child.size(); ++i )
    {
      print (*(node->child[i]->alias),"@@@@");
    }
    #endif
    int left = 1;
    int right = (signed)node->child.size();
    for(;right>=left;)
    {
      int ptr = (right+left) >> 1;
      TreeNode* res = findNodeByAliasRecurse(node->child[ptr-1],val,cmp);
      if ( res ) LEAVE_(res);
      __require__( cmp != 0 );
      __require__ ( right <= (signed)node->child.size() );
      __require__ ( 0 < left );
      if ( cmp < 0 )right = ptr-1;
      else left = ptr+1;
    }
    // is not found
    LEAVE_(node);
  }
  LEAVE_(0);
}

TreeNode* findNodeByAddrRecurse(TreeNode* node,AValue& val, int& cmp)
{
  ENTER;
  __trace2__("node: %p",node);
  bool strong = false;
  __require__(node != 0);
  if ( !node->addr ) {cmp = 0; goto find_child;}
  cmp =  compare_val_pat(val,*(node->addr),strong);
  if ( strong ) // cmp == 0
    LEAVE_(node);
  if ( cmp == 0 ) // weak
  {
  find_child:
    /*
      fix me here, change to binary search
    */
    //__watch__(node->child.size());
    /*for ( int i=0; i<node->child.size(); ++i )
    {
      TreeNode* res = findNodeByAddrRecurse(node->child[i],val,cmp);
      if ( res ) return res;
    }*/
    #if DUMP_ARRAY
    __trace2__("array has %d elements",node->child.size());
    for ( unsigned i=0; i<node->child.size(); ++i )
    {
      print ( *(node->child[i]->addr),"@@@@");
    }
    #endif
    int left = 1;
    int right = (signed)node->child.size();
    for(;right>=left;)
    {
      int ptr = (right+left) >> 1;
      TreeNode* res = findNodeByAddrRecurse(node->child[ptr-1],val,cmp);
      if ( res ) LEAVE_(res);
      __require__( cmp != 0 );
      __require__ ( right <= (signed)node->child.size() );
      __require__ ( 0 < left );
      if ( cmp < 0 )right = ptr-1;
      else left = ptr+1;
    }
    // is not found
    LEAVE_(node);
  }
  LEAVE_(0);
}

int addIntoAliasTreeRecurse(TreeNode* node,AliasRecord* rec)
{
  ENTER;
  __require__(node != 0);
  __trace__("addIntoAliasTree");
  if ( node->alias ) print(*node->alias,"node->alias");
  else __trace__("no node->alias");
  print(rec->alias,"rec->alias");

  bool strong = false;
  int cmp = 0;
  if ( !node->alias ) goto find_child;
  cmp = compare_pat_pat(rec->alias,*(node->alias),strong);
  if ( cmp == 0 )
  {
    if ( strong )
    {
      __warning__("duplicate alias, is has not added into aliases set");
      LEAVE_(0);
    }
    __trace2__("weak equal:")
    if (node->alias->defLength > rec->alias.defLength)
    {
      __unreachable__("incorrect tree");
    }
  find_child:
    /*
      fix me here, change to binary search
    */
    /*for ( int i=0; i<node->child.size(); ++i )
    {
      cmp = addIntoAliasTreeRecurse(node->child[i],rec);
      if ( cmp == 0 ) LEAVE_(0);
    }*/
    /*#if DUMP_ARRAY
    for ( int i=0; i<node->child.size(); ++i )
    {
      print ( *(node->child[i]->alias),"@@@@");
    }
    #endif*/
    int left = 1;
    int right = (signed)node->child.size();
    for(;right>=left;)
    {
      int ptr = (right+left) >> 1;
      cmp = addIntoAliasTreeRecurse(node->child[ptr-1],rec);
      if ( cmp == 0 ) LEAVE_(0);
      __require__( cmp != 0 );
      __require__ ( right <= (signed)node->child.size() );
      __require__ ( 0 < left );
      if ( cmp < 0 )right = ptr-1;
      else left = ptr+1;
    }
    __trace__("**** add element ****");
    TreeNode* newNode = new TreeNode;
    newNode->addr = &rec->addr;
    newNode->alias = &rec->alias;
    node->child.push_back(newNode);
    cmp = 0;
  }
  LEAVE_(cmp);
}

int addIntoAddrTreeRecurse(TreeNode* node,AliasRecord* rec)
{
  ENTER;
  __trace2__("addIntoAddrTree");
  if ( node->addr ) print(*node->addr,"node->addr");
  else __trace__("no addr");
  print(rec->addr,"rec->addr");
  __require__(node != 0);
  bool strong = false;
  int cmp = 0;
  if ( !node->addr ) goto find_child;
  cmp = compare_pat_pat(rec->addr,*(node->addr),strong);
  if ( cmp == 0 )
  {
    if ( strong )
    {
      __warning__("duplicate alias, is has not added into aliases set");
      LEAVE_(0);
    }
    __trace2__("weak equal:")
    if (node->addr->defLength > rec->addr.defLength)
    {
      __unreachable__("incorrect tree");
    }
  find_child:
    /*
      fix me here, change to binary search
    */
    /*for ( int i=0; i<node->child.size(); ++i )
    {
      cmp = addIntoAddrTreeRecurse(node->child[i],rec);
      if ( cmp == 0 ) LEAVE_(0);
    }*/
    /*#if DUMP_ARRAY
    for ( int i=0; i<node->child.size(); ++i )
    {
      print ( *(node->child[i]->addr),"@@@@");
    }
    #endif*/
    int left = 1;
    int right = (signed)node->child.size();
    for(;right>=left;)
    {
      int ptr = (right+left) >> 1;
      cmp = addIntoAddrTreeRecurse(node->child[ptr-1],rec);
      if ( cmp == 0 ) LEAVE_(0);
      __require__( cmp != 0 );
      __require__ ( right <= (signed)node->child.size() );
      __require__ ( 0 < left );
      if ( cmp < 0 )right = ptr-1;
      else left = ptr+1;
    }
    __trace2__("****add element*****");
    TreeNode* newNode = new TreeNode;
    newNode->addr = &rec->addr;
    newNode->alias = &rec->alias;
    node->child.push_back(newNode);
    cmp = 0;
  }
  __trace2__("^addIntoAddrTree");
  LEAVE_(cmp);
}

static inline void makeAliasFromValueByAddres(
  const TreeNode& p,const AValue& val,Address &addr)
{
  ENTER;
  char buf[21];
  memset(buf,0,21);
  int ln = 0;

  throw_if_fail(p.alias->defLength<21 && p.alias->defLength >= 0);
  if ( p.alias->defLength != 0 )
  memcpy(buf,p.alias->value,p.alias->defLength);
  ln = p.alias->defLength;

  __require__(val.length-p.addr->defLength >= 0 );
  throw_if_fail(ln+(val.length-p.addr->defLength)<21);
  if ( val.length-p.addr->defLength != 0 )
  {
    memcpy(buf+ln,
         val.value+p.addr->defLength,
         val.length-p.addr->defLength);
    ln+=(val.length-p.addr->defLength);
  }

  __require__(ln < 21 );
  throw_if_fail( ln >= 0 );
  if ( ln == 0 ) throw runtime_error("result alias has zero length");
  addr.setNumberingPlan(p.alias->numberingPlan);
  addr.setTypeOfNumber(p.alias->typeOfNumber);
  addr.setValue(ln,buf);
  LEAVE;
}

static inline void makeAddressFromValueByAlias(
  const TreeNode& p,const AValue& val,Address &addr)
{
  ENTER;
  char buf[21];
  memset(buf,0,21);
  int ln;
  throw_if_fail(p.addr->defLength< 21 && p.addr->defLength >= 0);
  if ( p.addr->defLength != 0 )
    memcpy(buf,p.addr->value,p.addr->defLength);
  ln = p.addr->defLength;

  __require__( val.length-p.alias->defLength >=0 );
  //if(!( ln+(val.length-p.alias->defLength) <= 21 ))
  //  throw runtime_error("incorrect address->alias translation definition, result length > 21");
  throw_if_fail(ln+(val.length-p.alias->defLength) < 21);
  if ( (val.length-p.alias->defLength) != 0 )
  {
    memcpy(buf+ln,
         val.value+p.alias->defLength,
         val.length-p.alias->defLength);
    ln += val.length-p.alias->defLength;
  }
  __require__(ln < 21 );
  throw_if_fail( ln >= 0 );
  if ( ln == 0 ) throw runtime_error("result address has zero length");

  addr.setNumberingPlan(p.addr->numberingPlan);
  addr.setTypeOfNumber(p.addr->typeOfNumber);
  addr.setValue(ln,buf);
  LEAVE;
}

static inline void makeAPattern(APattern& pat, const Address& addr)
{
  ENTER;
  char buf[21];
  char pat_mask[21];
  pat.numberingPlan = addr.getNumberingPlan();
  pat.typeOfNumber = addr.getTypeOfNumber();
  int length = addr.getValue(buf);
  __require__ ( length < 21 );
  memset(pat_mask,0,21);
  pat.defLength = 0;
  pat.length = 0;
  bool undef = false;
  //pat.hasStar = false;
  for ( int i=0; i<length; ++i )
  {
    switch(buf[i])
    {
    case '?': // only at end
      undef = true;
      ++pat.length;
      break;
    case '*': // incorrect symbol
      throw runtime_error("* is incorrect symbol");
      //pat.hasStar = true;
      //goto for_break;
    default:
      if ( undef ) throw runtime_error("? may be only at end of pattern");
      ++pat.defLength;
      ++pat.length;
      pat_mask[i] = 0xff;
    }
  }
  //for_break:;
  memset(pat.value,0,21);
  for ( int i=0; i<length; ++i)
  {
    pat.value[i] = buf[i] & pat_mask[i];
  }
  LEAVE;
}

static inline void makeAValue(AValue& val, const Address& addr)
{
  ENTER;
  val.numberingPlan = addr.getNumberingPlan();
  val.typeOfNumber = addr.getTypeOfNumber();
  int length = addr.getValue((char*)val.value);
  __require__ ( length < 21 );
  val.length = length;
  LEAVE;
}

bool AliasManager::AddressToAlias(
  const Address& addr, Address& alias/*, bool* hide*/)
{
__synchronized__
  __trace__("\n\n*@*@*@*@*@*@*@*@*@*@*@* AddressToAlias *@*@*@*@*@*@*@*@*");
  ENTER;
  AValue val;
  makeAValue(val,addr);
  int cmp;
  TreeNode* node = findNodeByAddrRecurse(&addrRootNode,val,cmp);
  if ( node && !node->addr ) node = 0;
  if ( node )
  {
    __trace2__("result node %p ",node);
    print(val,"\tvalue:");
    print(*node->addr,"\taddr:");
    print(*node->alias,"\talias:");
    /*__trace2__("find for Val:%s,length:%d\n\tP:%.20s,length:%d",
               val.value,
               val.length,
               node->addr->value,
               node->addr->length);*/

    makeAliasFromValueByAddres(*node,val,alias);
  }
  LEAVE;
	/*if ( node && hide )
	{
		*hide = node->alias->hide;
	}*/
  return node != 0;
}

bool AliasManager::AliasToAddress(
  const Address& alias, Address& addr)
{
__synchronized__
  ENTER;
  __trace__("\n\n*@*@*@*@*@*@*@*@*@*@*@* AliasToAddress *@*@*@*@*@*@*@*@*");
  AValue val;
  makeAValue(val,alias);
  int cmp;
  TreeNode* node = findNodeByAliasRecurse(&aliasRootNode,val,cmp);
  if ( node && !node->alias ) node = 0;
  if ( node )
  {
    __trace2__("result node %p ",node);
    print(val,"\tvalue:");
    print(*node->alias,"\talias:");
    print(*node->addr,"\taddr:");
    /*__trace2__("find for Val:%s,length:%d\n\tP:%.20s,Star:%d,length:%d",
               val.value,
               val.length,
               node->alias->value,
               node->alias->hasStar,
               node->alias->length);*/
    makeAddressFromValueByAlias(*node,val,addr);
  }
  LEAVE;
  return node != 0;
}

void AliasManager::addAlias(const AliasInfo& info)
{
__synchronized__
  ENTER;
  auto_ptr<AliasRecord> rec(new AliasRecord);
  makeAPattern(rec->alias,info.alias);
  makeAPattern(rec->addr,info.addr);
	rec->alias.hide = info.hide;
  //rec->info = info;
  __trace2__("+++++++ ADD ALIAS +++++++");
  print(rec->addr,"addr");
  print(rec->alias,"alias");
  /*__trace2__("add into alias tree");
  addIntoAliasTreeRecurse(&aliasRootNode,rec.get());
  __trace2__("add into addr tree");
  addIntoAddrTreeRecurse(&addrRootNode,rec.get());*/
  //__trace2__("------- ADD ALIAS -------");
  rec->next = new_aliases;
  new_aliases = rec.release();
  new_aliases_count++;
  LEAVE;
}

void AliasManager::clean()
{
__synchronized__
  while(first_alias)
  {
    AliasRecord* r = first_alias;
    first_alias = first_alias->next;
    delete r;
  }
  while(new_aliases)
  {
    AliasRecord* r = new_aliases;
    new_aliases = new_aliases->next;
    delete r;
  }
  new_aliases_count = 0;
  aliasRootNode.clean();
  addrRootNode.clean();
  //__unreachable__("is not implemented");
}

AliasIterator* AliasManager::iterator()
{
__synchronized__
  __unreachable__("is not implemented");
  //return new LocalAliasIterator(first_alias);
  return 0;
}

int ali_sort_comparator(const void* pat1,const void* pat2)
{
  bool strong = true;
  int cmp = compare_pat_pat((**(AliasRecord**)pat1).alias,
                        (**(AliasRecord**)pat2).alias,strong);
  return cmp;
}
int adr_sort_comparator(const void* pat1,const void* pat2)
{
  bool strong = true;
  int cmp = compare_pat_pat((**(AliasRecord**)pat1).addr,
                        (**(AliasRecord**)pat2).addr,strong);
  return cmp;
}

void AliasManager::commit()
{
__synchronized__
  aliasRootNode.clean();
  addrRootNode.clean();
  AliasRecord** tmp_vector = new AliasRecord*[new_aliases_count];
  {
    AliasRecord** p = tmp_vector;
    AliasRecord* r = new_aliases;
    while ( r )
    {
      *p = r;
      r = r->next;
      ++p;
    }
    __require__( p == tmp_vector + new_aliases_count );
  }
  __qsort__(tmp_vector,new_aliases_count,sizeof(AliasRecord*),
            adr_sort_comparator);
  __trace2__("&---------- BEGIN by Addr --------------&");
  for ( int i =0; i<new_aliases_count; ++i )
  {
    __trace__("\n");
    print(tmp_vector[i]->addr,"from:");
    //print(tmp_vector[i]->alias,"\tto:");
  }
  __trace2__("&---------- END --------------&");
  for ( int i =0; i<new_aliases_count; ++i )
  {
		if ( tmp_vector[i]->alias.hide )
			addIntoAddrTreeRecurse(&addrRootNode,tmp_vector[i]);
  }
  __qsort__(tmp_vector,new_aliases_count,sizeof(AliasRecord*),
            ali_sort_comparator);
  __trace2__("&---------- BEGIN table by Alias --------------&");
  for ( int i =0; i<new_aliases_count; ++i )
  {
    __trace__("\n");
    print(tmp_vector[i]->alias,"from: ");
    //print(tmp_vector[i]->addr,"\tto:");
  }
  __trace2__("&---------- END --------------&");
  for ( int i =0; i<new_aliases_count; ++i )
  {
		addIntoAliasTreeRecurse(&aliasRootNode,tmp_vector[i]);
  }
  delete tmp_vector;
  while(first_alias)
  {
    AliasRecord* r = first_alias;
    first_alias = first_alias->next;
    delete r;
  }
  first_alias = new_aliases;
  new_aliases = 0;
  new_aliases_count = 0;
}

}; // namespace alias
}; // namespace smsc
