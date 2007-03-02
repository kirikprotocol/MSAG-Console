#ifndef __SCAG_MMS_FACTORY_H__
#define __SCAG_MMS_FACTORY_H__

#include <string>

#include "MmsMsg.h"

namespace scag {
namespace transport {
namespace mms {

class MmsFactory {
public:
  MmsFactory();
  ~MmsFactory();
  MmsMsg* createMM7Msg(const char* command_name, std::string transaction_id);
};

}//mms
}//transport
}//scag

#endif
