#ifndef __EYELINE_INFORMER_ALM_IACTIVITYLOGMINTER_HPP__
#define __EYELINE_INFORMER_ALM_IACTIVITYLOGMINTER_HPP__
#include "informer/io/Typedefs.h"
#include <vector>
#include <set>
#include "core/buffers/FixedLengthString.hpp"
#include "informer/data/Message.h"

namespace eyeline{
namespace informer{
namespace alm{

enum ResultFields{
  rfState=1,
  rfDate=2,
  rfAbonent=4,
  rfText=8,
  rfErrorCode=16,
  rfUserData=32
};

struct ALMRequestFilter{
  std::set<MsgState> stateFilter;
  std::set<std::string> abonentFilter;
  std::set<int> codeFilter;
  msgtime_type startDate;
  msgtime_type endDate;
  int32_t resultFields;
};

struct ALMResult{
  msgid_type id;
  MsgState state;
  smsc::core::buffers::FixedLengthString<32> abonent;
  int code;
  msgtime_type date;
  std::string text;
  smsc::core::buffers::FixedLengthString<Message::USERDATA_LENGTH> userData;
};

class IActivityLogMiner{
public:
  virtual int createRequest(dlvid_type dlvId,const ALMRequestFilter& filter)=0;
  virtual bool getNext(int reqId,std::vector<ALMResult>& result,int count)=0;
};

}
}
}

#endif
