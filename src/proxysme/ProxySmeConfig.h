#ifndef SMSC_PROXYSME_CONFIGREADER
#define SMSC_PROXYSME_CONFIGREADER

#include <string>
#include <util/config/Config.h>

namespace smsc {
namespace proxysme {

using smsc::util::config::Config;

class ProxySmeConfig {
public:
	struct sme {
		std::string host;
		int port;
		std::string sid;
		std::string password;
		std::string origAddr;
		std::string systemType;
		int timeOut;
		int queueLength;
	};
	ProxySmeConfig(const char * const config_filename); //throw(smsc::util::Exception)
	const sme & getLeft()  {return left;}
	const sme & getRigth() {return right;}
private:
	sme left, right;
	void fillSme(ProxySmeConfig::sme& smeItem, const char * const prefix, const Config config); //throw(smsc::util::Exception)
};

}
}

#endif
