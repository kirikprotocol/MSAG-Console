/*
  $Id$
*/

#if !defined __Cpp_Header__smeman_smetyes_h__
#define __Cpp_Header__smeman_smetyes_h__

#include <string>
#include <inttypes.h>
#include <exception>
#include "core/buffers/FixedLengthString.hpp"
#include "sms/sms_const.h"

namespace smsc {
namespace smeman {

//typedef const char* SmeSystemId;
typedef smsc::core::buffers::FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1> SmeSystemId;
typedef smsc::core::buffers::FixedLengthString<smsc::sms::MAX_SMEPASSWD_TYPE_LENGTH+1> SmePassword;
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
