#ifndef __SMSC_STORE_INDEX_SMSINDEX_HPP__
#define __SMSC_STORE_INDEX_SMSINDEX_HPP__

#include <string>
#include <inttypes.h>
#include "core/buffers/Array.hpp"

namespace smsc{
namespace store{
namespace index{

struct Param{
  enum ParamType{
    tSrcAddress,
    tDstAddress,
    tSrcSmeId,
    tDstSmeId,
    tRouteId,
    tSmsId,
    tFromDate,
    tTillDate,
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
  int QuerySms(const char* dir,const ParamArray& params,ResultArray& res){};
};

}//namespace index
}//namespace store
}//namespace smsc

#endif
