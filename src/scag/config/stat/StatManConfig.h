#ifndef StatManConf_dot_h
#define StatManConf_dot_h

#include "scag/config/base/ConfigView.h"
#include <core/buffers/IntHash.hpp>
namespace scag {
namespace config {

class StatManConfig {
public:    
    StatManConfig();
    StatManConfig(const ConfigView& cv)  throw(ConfigException);
    void init(const ConfigView& cv)  throw(ConfigException);   
    bool check(const ConfigView& cv)  throw(ConfigException);
    
#ifdef TESTING    
    StatManConfig::StatManConfig(std::string& directory,std::string& host,int genp,int svcp,int scp)  throw(ConfigException);
#endif

    std::string getDir() const;
    std::string getPerfHost() const;
    int getPerfGenPort() const;
    int getPerfSvcPort() const;
    int getPerfScPort() const;

	
	
protected:
        std::string dir;
        std::string perfHost;
		

        int perfGenPort;
        int perfSvcPort;
        int perfScPort;

		int connect_timeout;
		int queue_length;
		
public:
		int getSaccPort() const;
		std::string getSaccHost() const;
		IntHash<std::string> getEventFiler() const;
		int getReconnectTimeout()const;
		int getMaxQueueLength()const;

protected:
		std::string saccHost;
		int saccPort;
		IntHash<std::string> eventFilter;

};

}
}

namespace scag2 {
namespace config {
using scag::config::StatManConfig;
}
}

#endif
