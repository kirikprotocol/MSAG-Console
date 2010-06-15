#ifndef __SMSC_ALIAS_ALIASMAN_H__
#define __SMSC_ALIAS_ALIASMAN_H__

#include "sms/sms.h"

namespace smsc{
namespace alias{

struct AliasInfo
{
  smsc::sms::Address addr;
  smsc::sms::Address alias;
  bool hide;
};


class AliasManager{
public:
  virtual ~AliasManager(){}
  virtual void addAlias(const AliasInfo& ai)=0;
  virtual void deleteAlias(const smsc::sms::Address& alias)=0;
  virtual bool AliasToAddress(const smsc::sms::Address& alias,smsc::sms::Address& addr)=0;
  virtual bool AddressToAlias(const smsc::sms::Address& addr,smsc::sms::Address& alias)=0;
  virtual void Load()=0;
  virtual void enableControllerMode()=0;
};

}//alias
}//smsc

#endif
