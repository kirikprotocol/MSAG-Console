#ifndef SMSC_PROXYSME_CONFIGREADER
#define SMSC_PROXYSME_CONFIGREADER

#include <string>
#include <util/config/Config.h>

namespace smsc {
namespace proxysme {
enum {DEFAULTDCS_SMSC7BIT=0,DEFAULTDCS_LATIN1=3};
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
		int timeOut;		int defaultDCS;
	};
	ProxySmeConfig(const char * const config_filename); //throw(smsc::util::Exception)
	const sme & getLeft()  {return left;}
	const sme & getRigth() {return right;}
	int getQueueLength() {return queueLength;}
	int getAdminPort() {return adminPort;}
	const std::string & getAdminHost() {return adminHost;}
private:
	sme left, right;
	int queueLength;
	int adminPort;
	std::string adminHost;
	void fillSme(ProxySmeConfig::sme& smeItem, const char * const prefix, const Config config); //throw(smsc::util::Exception)
};

}
}

#endif
