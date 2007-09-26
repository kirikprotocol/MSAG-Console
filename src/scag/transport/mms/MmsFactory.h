#ifndef __SCAG_MMS_FACTORY_H__
#define __SCAG_MMS_FACTORY_H__

#include <string>

#include "MmsMsg.h"

namespace scag {
namespace transport {
namespace mms {

class MmsFactory_ {
public:
  MmsFactory_();
  ~MmsFactory_();
  MmsMsg* createMM7Msg(const char* command_name, const string& transaction_id);
};
  
class MmsFactory {
public:
  virtual MmsMsg* createMmsMsg(const string& transaction_id) = 0;
  static MmsMsg* getMmsMsg(const char* name, const string& transaction_id);
  static void initFactories();
  static void deleteFactories();
private:
  static Hash<MmsFactory*> factories;
};

}//mms
}//transport
}//scag

#endif
