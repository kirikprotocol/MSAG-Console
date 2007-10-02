#ifndef __SCAG_TRANSPORT_MMS_ENTITY_H__
#define __SCAG_TRANSPORT_MMS_ENTITY_H__

#include <string>

#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace scag { namespace transport { namespace mms {

using std::string;
using smsc::logger::Logger;

enum MmsEntityType {
  etUnknown, etVASP, etRS
};

struct MmsEntityInfo {
  MmsEntityType type;
  string endpointId;
  string host;
  int port;
  int timeout;
  bool enabled;

  MmsEntityInfo():type(etUnknown), port(0), timeout(0), enabled(false) {};

  void print(Logger* logger) const {
    smsc_log_debug(logger, "MmsEntityInfo type=%d epId=\'%s\' host=\'%s\' port=%d enabled=%d timeout=%d",
                    type, endpointId.c_str(), host.c_str(), port, enabled, timeout);
  }

};

struct MmsEntity {
  MmsEntityInfo info;
  Mutex mutex;
  Logger* logger;
  MmsEntity() {
    logger = Logger::getInstance("mms.entity");
    smsc_log_debug(logger, "create \'%s\' entity", info.endpointId.c_str());
  }
  MmsEntity(const MmsEntityInfo& _info) {
    info = _info;
    logger = Logger::getInstance("mms.entity");
    smsc_log_debug(logger, "create \'%s\' entity", info.endpointId.c_str());
  }
  MmsEntity(const MmsEntity& entity) {
    info = entity.info;
    logger = Logger::getInstance("mms.entity");
    smsc_log_debug(logger, "create \'%s\' entity", info.endpointId.c_str());
  }
  ~MmsEntity() {
    smsc_log_debug(logger, "delete \'%s\' entity", info.endpointId.c_str());
  }
};

}//mms
}//transport
}//scag

#endif
