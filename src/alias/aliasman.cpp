#include "aliasman.h"
#include <stdexcept>
#include <string>
#include <stdlib.h>
#include "util/debug.h"

namespace smsc{
namespace alias{

using namespace smsc::sms;
using namespace std;

#define __synchronized__

static inline int compare_patval( const APattern& pattern,
                                   const AValue& val )
{
  __trace2__("compare Pat:%s ? Val:%s",
              pattern.value,
              val.value);
#define compare_v(n) (pattern.value_32[n] - (pattern.mask_32[n] & val.value_32[n]))
#define ifn0goto {if (result) goto result_;}
  int32_t result;
  result = pattern.num_n_plan - val.num_n_plan; ifn0goto;
  result = compare_v(0); ifn0goto;
  result = compare_v(1); ifn0goto;
  result = compare_v(2); ifn0goto;
  result = compare_v(3); ifn0goto;
  result = compare_v(4); ifn0goto;
result_:
	__trace2__("= %c == %d",result,result>0?'>':result<0?'<':'0');
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
result_:
	__trace2__("= %c == %d",result,result>0?'>':result<0?'<':'0');
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

static inline void makeAPattern(APattern& pat, const Address& addr)
{
}

static inline void makeAValue(AValue& val, const Address& addr)
{
}

bool AliasManager::AddressToAlias(
	const Address& addr, Address& alias)
{
}

bool AliasManager::AliasToAddress(
	const Address& alias, Address&	addr)
{
__synchronized__
  if ( !table_ptr ) return false;
  if (!sorted)
  {
    qsort(adr_table,table_ptr,sizeof(AliasRecord*),pattern_compare_adr);
    qsort(ali_table,table_ptr,sizeof(AliasRecord*),pattern_compare_ali);
    sorted = true;
    /*for ( int i=0; i < table_ptr; ++i )
      printAliAdr(ali_table[i]);*/
  }
  AValue val;
  makeAValue(val,alias);
  AliasRecord** recordX = (AliasRecord**)bsearch(
                        &val,
												ali_table,
												table_ptr,
												sizeof(RouteRecord*),
												compare_ali);
  if (!recordX) return false;
  AliasRecord* record = *recordX;
  record->ok_next = 0;
  AliasRecord* ok_ali = record;
  for (AliasRecord** r = recordX-1; r != ali_table-1; --r )
  {
    if ( compare_patval((*r)->alias,val) == 0 )
    {
      (*r)->ok_next = ok_ali;
      ok_ali = *r;
    }else break;
  }
  for (AliasRecord** r = recordX+1; r != ali_table+table_ptr; ++r )
  {
    if ( compare_patval((*r)->alias,val) == 0 )
    {
      (*r)->ok_next = ok_ali;
      ok_ali = *r;
    }else break;
  }
  record = 0;
  int defLength = 0;
  if ( ok_ali->ok_next ) // has more then one routes
  {
    while ( ok_ali )
    {
      if ( ok_ali->alias.defLength > defLength )
      {
				defLength	 = ok_ali->alias.defLength;
				record = ok_ali;
      }
      else if ( ok_ali->alias.defLength == defLength )
      {
				__warning__("found equal alias value");
      }
			ok_ali = ok_ali->ok_next;
    }
    __require__(record);
		ok_ali = record;
	}
	makeAddressFromValueByAlias(ok_ali->alias,val,&addr);
	return true;
}

void AliasManager::addAlias(const AliasInfo& info)
{
	auto_ptr<AliasRecord> rec(new AliasRecoed);
	makeAPattern(rec->alias,info.alias);
	makeAPattern(rec->addr,info.addr);
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
	++table_ptr;
	sorted = false;
}

void AliasManager::clean()
{
	table_ptr = 0;
	//__unreachable__("is not implemented");
}

AliasIterator* AliasManager::iterator()
{
	__unreachable__("is not implemented");
	return 0;
}

}; // namespace alias
}; // namespace smsc



