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

extern void __qsort__ (void *const pbase, size_t total_elems, size_t size,int(*cmp)(const void*,const void*));
 
namespace smsc{
namespace alias{

using namespace smsc::sms;
using namespace std;

#define __synchronized__
inline 
void print(const APattern& pattern,char* text)
{
	__trace2__("#### %s = PATTERN {%s(len:%d/def:%d), npi: %d, ton: %d}",
              text, 
              pattern.value,
              pattern.length,
              pattern.defLength,
							pattern.numberingPlan,
							pattern.typeOfNumber);
}
void print(const AValue& val,char* text)
{
	__trace2__("#### %s = VALUE {%s(len:%d), npi: %d, ton: %d}",
              text, 
              val.value,
              val.length,
							val.numberingPlan,
							val.typeOfNumber);
}
inline 
int compare_val_pat(const AValue& val, const APattern& pattern,bool& strong)
{
	__trace2__("compare (value ? pattern)");
	print(val,"\tV:");
	print(pattern,"\tP:");
#define compare_v(n) \
  ((pattern.mask_32[n]&val.value_32[n]) - pattern.value_32[n])
#define ifn0goto {if ( result ) goto result_; }
  int32_t result;
  result = val.num_n_plan-pattern.num_n_plan; ifn0goto;
  result = compare_v(0); ifn0goto;
  result = compare_v(1); ifn0goto;
  result = compare_v(2); ifn0goto;
  result = compare_v(3); ifn0goto;
  result = compare_v(4); ifn0goto;
	result = val.length - pattern.length; ifn0goto;
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

#define compare_v(n) (pat1.value_32[n]&pat2.mask_32[n] - pat2.value_32[n]&pat1.mask_32[n] )
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = pat1.num_n_plan - pat2.num_n_plan; ifn0goto;
  if ( strong )
		{result = pat1.defLength - pat2.defLength; ifn0goto;}
	else if ( pat1.defLength == pat2.defLength ) strong = true;
  result = compare_v(0); ifn0goto;
  result = compare_v(1); ifn0goto;
  result = compare_v(2); ifn0goto;
  result = compare_v(3); ifn0goto;
  result = compare_v(4); ifn0goto;
	result = pat1.length - pat2.length; ifn0goto;
result_:
  __trace2__("result(P1%cP2)%s%d",result>0?'>':result<0?'<':'=',
			(result==0)?strong?"(strong)":"(weak)":"",
		  result); 
  return (int32_t)result;
#undef if0ngoto
#undef compare_v
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
    for ( int i=0; i<node->child.size(); ++i )
    {
      TreeNode* res = findNodeByAliasRecurse(node->child[i],val,cmp);
      if ( res ) return res;
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
    for ( int i=0; i<node->child.size(); ++i )
    {
      TreeNode* res = findNodeByAddrRecurse(node->child[i],val,cmp);
      if ( res ) return res;
    }
    // is not found
    return node;
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
    for ( int i=0; i<node->child.size(); ++i )
    {
      cmp = addIntoAliasTreeRecurse(node->child[i],rec);
      if ( cmp == 0 ) LEAVE_(0);
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
    for ( int i=0; i<node->child.size(); ++i )
    {
      cmp = addIntoAddrTreeRecurse(node->child[i],rec);
      if ( cmp == 0 ) LEAVE_(0);
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
  pat.numberingPlan = addr.getNumberingPlan();
  pat.typeOfNumber = addr.getTypeOfNumber();
  int length = addr.getValue(buf);
  __require__ ( length < 21 );
  memset(pat.mask,0,21);
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
      pat.mask[i] = 0xff;
    }
  }
  for_break:;
  memset(pat.value,0,21);
  for ( int i=0; i<length; ++i)
  {
    pat.value[i] = buf[i] & pat.mask[i];
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
  const Address& addr, Address& alias)
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

























#if 0
static inline int compare_patval( const APattern& pattern,
                                   const AValue& val,bool strong = true)
{
  __trace2__("compare Pat:%s ? Val:%s",
              pattern.value,
              val.value);
  __trace2__("compare Pat(npi):%d ? Val(npi):%d, P:%x,v:%x",
              pattern.numberingPlan,
              val.numberingPlan,
              pattern.num_n_plan,
              val.num_n_plan);
  __trace2__("compare Pat(tni):%d ? Val(tni):%d, P:%x,v:%x",
              pattern.typeOfNumber,
              val.typeOfNumber,
              pattern.num_n_plan,
              val.num_n_plan);
#define compare_v(n) (pattern.value_32[n] - (pattern.mask_32[n] & val.value_32[n]))
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = pattern.num_n_plan - val.num_n_plan; ifn0goto;
  result = compare_v(0); ifn0goto;
  result = compare_v(1); ifn0goto;
  result = compare_v(2); ifn0goto;
  result = compare_v(3); ifn0goto;
  result = compare_v(4); ifn0goto;
  if ( strong )
  {
    __trace2__("check_length: %c : P%d ? V%d",
             pattern.hasStar?'*':' ',
             pattern.length,
             val.length);
    
    result = pattern.hasStar|| (pattern.length != pattern.defLength)?
      -1:
      (((int)pattern.length)-((int)val.length));
        //-1:0;
  }
  else
  {
    __trace2__("check_length: %c : P%d ? V%d",
             pattern.hasStar?'*':' ',
             pattern.length,
             val.length);
    
    result = pattern.hasStar?
      0:
      (((int)pattern.length)-((int)val.length))?
        -1:0;
  }
result_:
  __trace2__("= %c == %d",result>0?'>':result<0?'<':'0',result);
  return (int32_t)result;
#undef if0ngoto
#undef compare_v
}

static inline int compare_patpat( const APattern& pat1,
                                  const APattern& pat2 )
{
  __trace2__("compare Pat1:%s ? Pat2:%s",
              pat1.value,
              pat2.value);
#define compare_v(n) (pat1.value_32[n] - pat2.value_32[n] )
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = pat1.num_n_plan - pat2.num_n_plan; ifn0goto;
  result = compare_v(0); ifn0goto;
  result = compare_v(1); ifn0goto;
  result = compare_v(2); ifn0goto;
  result = compare_v(3); ifn0goto;
  result = compare_v(4); ifn0goto;
  __trace2__("check_length: %c : P%d ? A%d",
             pat1.hasStar||pat2.hasStar?'*':' ',
             pat1.length,
             pat2.length);
  result = pat1.hasStar?pat2.hasStar?0:-1:pat2.hasStar?+1:0;
  ifn0goto;
  //result = pat1.hasStar||pat2.hasStar?0:
  //    ((int)pat1.length)-((int)pat2.length)?-1:0;
  result = ((int)pat1.length)-((int)pat2.length);
  ifn0goto;
result_:
  __trace2__("= %c == %d",result>0?'>':result<0?'<':'0',result);
  return (int32_t)result;
#undef if0ngoto
#undef compare_v
}

int compare_ali(const void* val,const void* pat){
  return compare_patval((**(AliasRecord**)pat).alias,*(AValue*)val);}

int compare_adr(const void* val,const void* pat){
  return compare_patval((**(AliasRecord**)pat).addr,*(AValue*)val);}

int pattern_compare_ali(const void* pat1,const void* pat2){
  return compare_patpat((**(AliasRecord**)pat2).alias,
                        (**(AliasRecord**)pat1).alias);}
int pattern_compare_adr(const void* pat1,const void* pat2){
  return compare_patpat((**(AliasRecord**)pat2).addr,
                        (**(AliasRecord**)pat1).addr);}

static inline void makeAliasFromValueByAddres(
  const AliasRecord& p,const AValue& val,Address &addr)
{
  ENTER;
  char buf[21];
  //__require__(p.addr.defLength <= val.length );
  //__require__(p.alias.defLength+val.length-p.addr.defLength < 21);
  
  memset(buf,0,21);
  int ln = 0;
  
  //if (!p.alias.defLength<=21) 
  //  throw runtime_error("incorrect address->alias translation definition, result length > 21");
  throw_if_fail(p.alias.defLength<21 && p.alias.defLength >= 0);
  if ( p.alias.defLength != 0 )
  memcpy(buf,p.alias.value,p.alias.defLength);
  ln = p.alias.defLength;
  
  __require__(val.length-p.addr.defLength >= 0 );
  //if (!(ln+(val.length-p.addr.defLength)<=21))
  //  throw runtime_error("incorrect address->alias translation definition, result length > 21");
  throw_if_fail(ln+(val.length-p.addr.defLength)<21);
  if ( val.length-p.addr.defLength != 0 )
  {
    memcpy(buf+ln,
         val.value+p.addr.defLength,
         val.length-p.addr.defLength);
    ln+=(val.length-p.addr.defLength);
  }
  
  __require__(ln < 21 );
  throw_if_fail( ln >= 0 );
  if ( ln == 0 ) throw runtime_error("result alias has zero length");
  addr.setNumberingPlan(p.alias.numberingPlan);
  addr.setTypeOfNumber(p.alias.typeOfNumber);
  addr.setValue(ln,buf);
  LEAVE;
}

static inline void makeAddressFromValueByAlias(
  const AliasRecord& p,const AValue& val,Address &addr)
{
  ENTER;
  char buf[21];
  //__require__(p.alias.defLength <= val.length );
  //__require__(p.addr.defLength+val.length-p.alias.defLength < 21);
  memset(buf,0,21);
  int ln;
  //if(!(ln+p.addr.defLength<=21))
  //  throw runtime_error("incorrect address->alias translation definition, result length > 21");
  throw_if_fail(p.addr.defLength< 21 && p.addr.defLength >= 0);
  if ( p.addr.defLength != 0 )
    memcpy(buf,p.addr.value,p.addr.defLength);
  ln = p.addr.defLength;

  __require__( val.length-p.alias.defLength >=0 );
  //if(!( ln+(val.length-p.alias.defLength) <= 21 ))
  //  throw runtime_error("incorrect address->alias translation definition, result length > 21");
  throw_if_fail(ln+(val.length-p.alias.defLength) < 21);
  if ( (val.length-p.alias.defLength) != 0 )
  {
    memcpy(buf+ln,
         val.value+p.alias.defLength,
         val.length-p.alias.defLength);
    ln += val.length-p.alias.defLength;
  }
  __require__(ln < 21 );
  throw_if_fail( ln >= 0 );
  if ( ln == 0 ) throw runtime_error("result address has zero length");
  
  addr.setNumberingPlan(p.addr.numberingPlan);
  addr.setTypeOfNumber(p.addr.typeOfNumber);
  addr.setValue(ln,buf);
  LEAVE;
}

static inline void makeAPattern(APattern& pat, const Address& addr)
{
  ENTER;
  char buf[21];
  pat.numberingPlan = addr.getNumberingPlan();
  pat.typeOfNumber = addr.getTypeOfNumber();
  int length = addr.getValue(buf);
  __require__ ( length < 21 );
  memset(pat.mask,0,21);
  pat.defLength = 0;
  pat.length = 0;
  bool undef = false;
  pat.hasStar = false;
  for ( int i=0; i<length; ++i )
  {
    switch(buf[i])
    {
    case '?': // only at end
      undef = true;
      ++pat.length;
      break;
    case '*': // only end of value
      pat.hasStar = true;
      goto for_break;
    default:
      if ( undef ) throw runtime_error("*,? may be only at end of pattern");
      ++pat.defLength;
      ++pat.length;
      pat.mask[i] = 0xff;
    }
  }
  for_break:;
  memset(pat.value,0,21);
  for ( int i=0; i<length; ++i)
  {
    pat.value[i] = buf[i] & pat.mask[i];
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

static void* 
my_b_search( void* val, AliasRecord** table, int table_size,
             int (*compare)(const void* val,const void* pat), 
             bool& equal )
{
  int left = 1;
  int right = table_size;
  int ptr = ((left+right)>>1);
  __trace2__("table size %d",table_size);
  __watch__(table);
  __watch__(table+table_size);
  for(;;)
  {
    __require__( ptr >= 1 );
    __require__( ptr <= table_size );
    __watch__(table+ptr-1);
    int cmp = compare(val,table+ptr-1);
    if ( cmp == 0 ) 
    { 
      equal = true; 
      return (table+ptr-1);
    } 
    if ( right == left ) 
    {
      equal = false;
      //return (left>1)?table+left-2:table;
      return table+left-1;
    }
    if ( cmp > 0 ) left = ptr+1;
    else right = ptr;
    ptr = (left+right) >> 1;
    __require__(right<=table_size);
    __require__(right>=left);
  }
}

void AliasManager::normalizeUncertainty()
{
  // normalize aliases
  AliasRecord* greatUncertainty = 0;
  int defLength = 0;

  for ( int i = table_ptr-1; i >= 0; --i )
  {
    if ( adr_table[i]->addr.hasStar || 
         (adr_table[i]->addr.length != adr_table[i]->addr.defLength) )
    {
      while ( greatUncertainty && 
              defLength >= adr_table[i]->addr.defLength )
      {
        defLength = greatUncertainty->addr.defLength;
        greatUncertainty = greatUncertainty->addr.greatUncertainty;
      }
      adr_table[i]->addr.greatUncertainty = greatUncertainty;
      if ( adr_table[i]->addr.defLength > defLength )
      {
        greatUncertainty = adr_table[i];
        defLength = adr_table[i]->addr.defLength;
      }
    }
    else
    {
      adr_table[i]->addr.greatUncertainty = greatUncertainty;
    }
    __trace2__(":::ADR::: Rec:%p, P:%s,Star:%d,length:%d",
         adr_table[i],
         adr_table[i]->addr.value,
         adr_table[i]->addr.hasStar,
         adr_table[i]->addr.length);

  }

  greatUncertainty = 0;
  defLength = 0;

  for ( int i=table_ptr-1; i >=0 ; --i )
  {
    if ( ali_table[i]->alias.hasStar || 
         (ali_table[i]->alias.length != ali_table[i]->alias.defLength) )
    {
      while ( greatUncertainty && 
              defLength >= ali_table[i]->alias.defLength )
      {
        defLength = greatUncertainty->alias.defLength;
        greatUncertainty = greatUncertainty->alias.greatUncertainty;
      }
      ali_table[i]->alias.greatUncertainty = greatUncertainty;
      if ( ali_table[i]->alias.defLength > defLength )
      {
        greatUncertainty = ali_table[i];
        defLength = ali_table[i]->alias.defLength;
      }
    }
    else
    {
      ali_table[i]->alias.greatUncertainty = greatUncertainty;
    }
    __trace2__(":::ALIAS::: Rec:%p, P:%s,Star:%d,length:%d",
             ali_table[i],
             ali_table[i]->alias.value,
             ali_table[i]->alias.hasStar,
             ali_table[i]->alias.length);
  }
}

bool AliasManager::AddressToAlias(
  const Address& addr, Address& alias)
{
__synchronized__
  ENTER;
  if ( !table_ptr ) return false;
  if (!sorted)
  {
    __qsort__(adr_table,table_ptr,sizeof(AliasRecord*),pattern_compare_adr);
    __qsort__(ali_table,table_ptr,sizeof(AliasRecord*),pattern_compare_ali);
    normalizeUncertainty();
    sorted = true;
  }
  AValue val;
  makeAValue(val,addr);
  bool equal;
  AliasRecord** recordX = (AliasRecord**)my_b_search(
                        &val,
                        adr_table,
                        table_ptr,
                        compare_adr,
                        equal);
  __trace2__("find record %p (%s)",recordX?*recordX:0,equal?"equal":"less");
  if (!recordX){
    recordX = adr_table;
    if ( compare_patval((*recordX)->addr,val,false) != 0 )
    {
      LEAVE; return false;
    }
    else equal = true;
  }
  AliasRecord* record = *recordX;
  if ( !equal )
  {
    do
    {
      //if ( record->addr.defLength != record->addr.length || record->addr.hasStar )
      //{
        if ( compare_patval(record->addr,val,false) != 0 )
          {record = record->addr.greatUncertainty; continue;}
      //}
      //else record = 0; 
      break;
    }
    while ( record != 0 );
  }
  if (!record){ LEAVE; return false; }
  __trace2__("result record %p ",record);
  __trace2__("find for Val:%s,length:%d\n\tP:%s,Star:%d,length:%d",
               val.value,
               val.length,
               record->addr.value,
               record->addr.hasStar,
               record->addr.length);
  makeAliasFromValueByAddres(*record,val,alias);
  LEAVE;
  return true;
}

bool AliasManager::AliasToAddress(
  const Address& alias, Address&  addr)
{
__synchronized__
  ENTER;
  if (!table_ptr ) return false;
  if (!sorted)
  {
    __qsort__(adr_table,table_ptr,sizeof(AliasRecord*),pattern_compare_adr);
    __qsort__(ali_table,table_ptr,sizeof(AliasRecord*),pattern_compare_ali);
    normalizeUncertainty();
    sorted = true;
  }
  AValue val;
  makeAValue(val,alias);
  bool equal;
  AliasRecord** recordX = (AliasRecord**)my_b_search(
                        &val,
                        ali_table,
                        table_ptr,
                        compare_ali,
                        equal);
  __trace2__("find record %p (%s)",recordX?*recordX:0,equal?"equal":"less");
  if (!recordX) {
    recordX = ali_table;
    if ( compare_patval((*recordX)->alias,val,false) != 0 )
    {
      LEAVE; return false;
    }
    else equal = true;
  }
  AliasRecord* record = *recordX;
  if ( !equal )
  {
    do 
    {
      //if ( record->alias.defLength != record->alias.length || record->alias.hasStar )
      //{
        if ( compare_patval(record->alias,val,false) != 0 )
          {record = record->alias.greatUncertainty;continue;}
      //}
      //else record = 0;
      break;
    }
    while ( record != 0 );
  }
  if (!record) {LEAVE; return false;}
  __trace2__("result record %p ",record);
  __trace2__("find for Val:%s,length:%d\n\tP:%s,Star:%d,length:%d",
               val.value,
               val.length,
               record->alias.value,
               record->alias.hasStar,
               record->alias.length);
  makeAddressFromValueByAlias(*record,val,addr);
  LEAVE;
  return true;
}

void AliasManager::addAlias(const AliasInfo& info)
{
  ENTER;
  auto_ptr<AliasRecord> rec(new AliasRecord);
  makeAPattern(rec->alias,info.alias);
  makeAPattern(rec->addr,info.addr);
  //rec->info = info;
  if ( table_ptr == table_size )
  {
    auto_ptr<AliasRecord*> adr_tmp(new AliasRecord*[table_size+1024]);
    auto_ptr<AliasRecord*> ali_tmp(new AliasRecord*[table_size+1024]);
    memcpy(adr_tmp.get(),adr_table,sizeof(AliasRecord*)*table_size);
    memcpy(ali_tmp.get(),ali_table,sizeof(AliasRecord*)*table_size);
    delete adr_table; adr_table = adr_tmp.release();
    delete ali_table; ali_table = ali_tmp.release();
    table_size+=1024;
  }
  ali_table[table_ptr] = rec.release();
  adr_table[table_ptr] = ali_table[table_ptr];
  ali_table[table_ptr]->ok_next = first_alias;
  first_alias = ali_table[table_ptr];
  ++table_ptr;
  sorted = false;
  LEAVE;
}

void AliasManager::clean()
{
  table_ptr = 0;
  first_alias = 0;
  //__unreachable__("is not implemented");
}

/*class LocalAliasIterator : public AliasIterator
{
  AliasRecord* rec;
  bool started ;
public:
  LocalAliasIterator(AliasRecord* first_alias) : started(false)
  {
    rec = first_alias;
  }
  virtual ~LocalAliasIterator(){}
  virtual bool next()
  {
    if (!started) started = true;
    else if ( rec ) rec = rec->ok_next;
    return rec!=0;
  }
  virtual AliasInfo getAliasInfo() 
  {
    return rec->info;
  }
  virtual bool hasNext()
  {
    return rec->ok_next;
  }
};*/

#endif

void AliasManager::addAlias(const AliasInfo& info)
{
__synchronized__  
  ENTER;
  auto_ptr<AliasRecord> rec(new AliasRecord);
  makeAPattern(rec->alias,info.alias);
  makeAPattern(rec->addr,info.addr);
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
		print(tmp_vector[i]->alias,"\tto:");
	}
	__trace2__("&---------- END --------------&");
	for ( int i =0; i<new_aliases_count; ++i )
	{
		addIntoAddrTreeRecurse(&addrRootNode,tmp_vector[i]);
	}
	__qsort__(tmp_vector,new_aliases_count,sizeof(AliasRecord*),
						ali_sort_comparator);
	__trace2__("&---------- BEGIN table by Alias --------------&");
	for ( int i =0; i<new_aliases_count; ++i )
	{
		__trace__("\n");
		print(tmp_vector[i]->alias,"from: ");
		print(tmp_vector[i]->addr,"\tto:");
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



