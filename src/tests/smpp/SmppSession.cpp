#include <tests/util/Configurator.hpp>
#include "SmppSession.hpp"

namespace smsc {
  namespace test {
	namespace smpp {
	  smsc::logger::Logger SmppSession::log = smsc::test::util::logger.getLog("smsc.test.smpp.SmppSession");
	}
  }
}
