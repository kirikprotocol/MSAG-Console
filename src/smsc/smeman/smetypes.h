/*
  $Id$
*/

#if !defined __Cpp_Header__smeman_smetyes_h__
#define __Cpp_Header__smeman_smetyes_h__

#include <string>
#include <inttypes.h>
#include <exception>

namespace smsc {
namespace smeman {

typedef std::string SmeSystemId;
typedef std::string SmePassword;
typedef int32_t SmeIndex;
typedef int SmeNType;

const int32_t INVALID_SME_INDEX = -1;

const int32_t MAX_SME_PROXIES=2000;

enum SmeFlags{
  sfCarryOrgDescriptor   =0x00000001,
  sfCarryOrgAbonentInfo  =0x00000002,
  sfCarrySccpInfo        =0x00000004,
  sfFillExtraDescriptor  =0x00000008,
  sfForceReceiptToSme    =0x00000010,
  sfForceGsmDatacoding   =0x00000020,
  sfSmppPlus             =0x00000040
};

class SmeError : public std::exception
{
  virtual const char* what() const  throw()
  {
    return "SmeError";
  }
};

}
}


#endif
