/*
  $Id$
*/
#if !defined __Cpp_Header_smsc_smeman_smeinfo_h__
#define __Cpp_Header_smsc_smeman_smeinfo_h__

#include <string>

#include "smetypes.h"
#include "smeproxy.h"

namespace smsc {
namespace smeman {


struct SmeInfo
{
  uint8_t typeOfNumber;
  uint8_t numberingPlan;
  uint8_t interfaceVersion;
  std::string rangeOfAddress;
  std::string systemType;
  SmePassword password;
  std::string hostname;
  int port;
  SmeSystemId systemId;
  SmeProxyPriority priority;
  SmeNType SME_N;
  bool  disabled;
  bool wantAlias;
  bool internal;
  SmeBindMode bindMode;
  std::string receiptSchemeName;
  uint32_t timeout;
  uint32_t proclimit;
  uint32_t schedlimit;
  uint32_t providerId;
  uint32_t accessMask;
  uint32_t flags;
  bool hasFlag(SmeFlags flag)
  {
    return (flags&flag)!=0;
  }
  ~SmeInfo(){}
  SmeInfo(){}
};

} // namespace smeman
} // namespace smsc
#endif
