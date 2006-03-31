#ifndef __SMSC_CLUSTER_LISTENERS_CGMCOMMANDLISTENER_HPP__
#define __SMSC_CLUSTER_LISTENERS_CGMCOMMANDLISTENER_HPP__

#include "cluster/Interconnect.h"

namespace smsc {
namespace cluster {

class CgmCommandListener : public CommandListener
{
public:
  CgmCommandListener()
  {
    logger=smsc::logger::Logger::getInstance("icon.cgm");
  }
  void handle(const Command& command);
protected:
  smsc::logger::Logger* logger;
};

}
}

#endif
