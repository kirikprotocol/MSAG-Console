#ifndef __SMSC_SYSTEM_MAPIO_FRAUDCONTROL_HPP__
#define __SMSC_SYSTEM_MAPIO_FRAUDCONTROL_HPP__

#include <string>
#include "core/buffers/Hash.hpp"
#include "core/buffers/RefPtr.hpp"
#include "core/synchronization/Mutex.hpp"

namespace smsc{
namespace system{
namespace mapio{

namespace buf=smsc::core::buffers;
namespace sync=smsc::core::synchronization;

class FraudControl{
public:
  FraudControl():ignoreTail(0),enabledCheck(true),enableReject(false)
  {
  }

  static void Init(const char* argFileName);
  static void Shutdown();

  static FraudControl* getInstance()
  {
    return instance;
  }

  void Reload();

  int ignoreTail;
  bool enabledCheck;
  bool enableReject;
  bool checkWhiteList(const char* msc)
  {
    HashRef wl=whiteList;
    return wl->Exists(msc);
  }
protected:
  static FraudControl* instance;
  std::string fileName;

  typedef buf::RefPtr<buf::Hash<int>,sync::Mutex> HashRef;

  HashRef whiteList;

};

}
}
}

#endif

