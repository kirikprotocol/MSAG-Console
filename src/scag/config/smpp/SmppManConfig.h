#ifndef SCAG_CONFIG_SMEMANCONFIG
#define SCAG_CONFIG_SMEMANCONFIG

#include "smeman_structures.h"
#include <vector>

namespace scag {
namespace config {


class SmppManConfig
{

public:
  SmppManConfig();
  virtual ~SmppManConfig();
  
protected:
  smsc::logger::Logger *logger;

private:
};

}
}

namespace scag2 {
namespace config {
using scag::config::SmppManConfig;
}
}

#endif //ifndef SMSC_UTIL_CONFIG_SMEMAN_SMEMANCONFIG
