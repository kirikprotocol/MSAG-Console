#if !defined(__Cxx_header__aliasman_h__)
#define __Cxx_header__aliasman_h__

#include <inttypes.h>
#include "sms/sms.h"

namespace smsc{
namespace alias{

using namespace std;
using namespace sms;

struct AliasInfo
{
  Address addr;
  Address alias;
};

struct APattern
{
  union
  {
    struct
    {
      uint8_t typeOfNumber;
      uint8_t numberingPlan;
    };
    int16_t num_n_plan;
  };
  union
  {
    uint8_t mask[21];
    int32_t mask_32[5];
  };
  union
  {
    uint8_t value[21];
    int32_t value_32[5];
  };
  int length;
  int defLength;
};

struct AValue
{
  union
  {
    struct
    {
      uint8_t typeOfNumber;
      uint8_t numberingPlan;
    };
    int16_t num_n_plan;
  };
  union
  {
    uint8_t value[21];
    int32_t value_32[5];
  };
  int length;
};

struct AliasRecord
{
  APattern alias;
  APattern addr;
  AliasRecord* ok_next;
};

class AliasIterator
{
public:
  virtual bool next() = 0;
  virtual AliasInfo getAliasInfo() = 0;
  virtual bool hasNext() = 0;
};

class AliasTable
{
public:
  virtual bool AliasToAddress(const Address& alias, Address&  addr) = 0;
  virtual bool AddressToAlias(const Address& addr, Address& alias) = 0;
};

class AliasAdmin
{
public:
  virtual void addAlias(const AliasInfo& info) = 0;
  virtual void clean() = 0;
  virtual AliasIterator* iterator() = 0;
};

class AliasManager : 
  public AliasTable,
  public AliasAdmin
{
  AliasRecord** ali_table;
  AliasRecord** adr_table;

  int table_size;
  int table_ptr;
  bool sorted;
public:
  AliasManager(): ali_table(0),adr_table(0), table_size(1024), table_ptr(0)
  {
    ali_table = new AliasRecord*[table_size];
    adr_table = new AliasRecord*[table_size];
  }
  virtual ~AliasManager()
  {
    for ( int i=0; i<table_ptr; ++i )
    {
      if ( ali_table[i] ) delete ali_table[i];
      adr_table[i] = 0;
    }
    delete ali_table;
    delete adr_table;
  }
  
  virtual bool AliasToAddress(const Address& alias, Address&  addr);
  virtual bool AddressToAlias(const Address& addr, Address& alias);
  
  virtual void addAlias(const AliasInfo& info);
  virtual void clean();
  virtual AliasIterator* iterator();
};

}; // namespace alias
}; // namespace smsc

#endif

