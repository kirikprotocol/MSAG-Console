/*
  $Id$
*/
#include <string>
#include "smetypes.h"
#include "util/debug.h"

#if !defined __Cpp_Header__smeman_smeinfo_h__
#define __Cpp_Header__smeman_smeinfo_h__

namespace smsc {
namespace smeman {

struct SmeInfo
{
  uint8_t typeOfNumber;
  uint8_t numberingPlan;
  uint8_t interfaceVersion;
  std::string rangeOfAddress;
  std::string systemType;
  std::string password;
  std::string hostname;
  int port;
  SmeSystemId systemId;
  SmeNType SME_N;
  bool  disabled;
  bool wantAlias;
  ~SmeInfo(){}
  SmeInfo(){}
};

}; // namespace smeman
}; // namespace smsc
#endif
