/*
  $Id$
*/

#if !defined __Cpp_Header_smeman_smetypes_h__
#define __Cpp_Header_smeman_smetypes_h__

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
  sfSmppPlus             =0x00000040,
  sfDefaultDcLatin1      =0x00000080
};

inline uint32_t parseSmeFlag(const std::string& val)
{
  using namespace smsc::smeman;
  if(val=="carryOrgDescriptor")
  {
    return sfCarryOrgDescriptor;
  }else if(val=="carryOrgUserInfo")
  {
    return sfCarryOrgAbonentInfo;
  }else if(val=="carrySccpInfo")
  {
    return sfCarrySccpInfo;
  }else if(val=="fillExtraDescriptor")
  {
    return sfFillExtraDescriptor;
  }else if(val=="forceSmeReceipt")
  {
    return sfForceReceiptToSme;
  }else if(val=="forceGsmDataCoding")
  {
    return sfForceGsmDatacoding;
  }else if(val=="smppPlus")
  {
    return sfSmppPlus;
  }else if(val=="defaultDcLatin1")
  {
    return sfDefaultDcLatin1;
  }
  return 0;
}

inline uint32_t parseSmeFlags(const std::string& val)
{
  std::string::size_type oldPos=0,commaPos=0;
  uint32_t rv=0;
  while((commaPos=val.find(',',oldPos))!=std::string::npos)
  {
    rv|=parseSmeFlag(val.substr(oldPos,commaPos-oldPos));
    oldPos=commaPos+1;
  }
  rv|=parseSmeFlag(val.substr(oldPos));
  return rv;
}



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
