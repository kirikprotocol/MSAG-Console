#include <tests/util/Configurator.hpp>
#include "SmppSession.hpp"

namespace smsc {
  namespace test {
	namespace smpp {
	  log4cpp::Category& SmppSession::log = smsc::test::util::logger.getLog("smsc.test.smpp.SmppSession");
	}
  }
}
