#ifndef __SMSC_STORE_INDEX_SMSINDEX_HPP__
#define __SMSC_STORE_INDEX_SMSINDEX_HPP__

#include <string>
#include <inttypes.h>

#include <sms/sms.h>
#include "core/buffers/Array.hpp"

namespace smsc{
namespace store{
namespace index{

using namespace smsc::sms;

static const uint8_t T_SMS_ID      = 10;
static const uint8_t T_FROM_DATE   = 20;
static const uint8_t T_TILL_DATE   = 30;
static const uint8_t T_SRC_ADDRESS = 40;
static const uint8_t T_DST_ADDRESS = 50;
static const uint8_t T_SRC_SME_ID  = 60;
static const uint8_t T_DST_SME_ID  = 70;
static const uint8_t T_ROUTE_ID    = 80;

struct Param{
  enum ParamType{
    tSmsId      = T_SMS_ID,
    tFromDate   = T_FROM_DATE,   tTillDate   = T_TILL_DATE,
    tSrcAddress = T_SRC_ADDRESS, tDstAddress = T_DST_ADDRESS,
    tSrcSmeId   = T_SRC_SME_ID,  tDstSmeId   = T_DST_SME_ID,
    tRouteId    = T_ROUTE_ID
  };
  ParamType type;
  std::string sValue;
  uint64_t iValue;
  time_t   dValue;
};

struct QueryResult{
  uint32_t lastTryTime;
  uint64_t offset;
};

typedef Array<Param> ParamArray;
typedef Array<QueryResult> ResultArray;

class SmsIndex{
public:
  void IndexateSms(const char* dir,SMSId id,uint64_t offset,SMS& sms){};
  int QuerySms(const char* dir,const ParamArray& params,ResultArray& res) {
      return 0;
  };
};

}//namespace index
}//namespace store
}//namespace smsc

#endif
