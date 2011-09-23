#ifndef StatManConf_dot_h
#define StatManConf_dot_h

#include "logger/Logger.h"
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
    StatManConfig::StatManConfig(std::string& directory,std::string& host,int genp,int svcp,int scp, const std::string& saadir, int interval)  throw(ConfigException);
#endif

    std::string getDir() const;
    std::string getPerfHost() const;
    int getPerfGenPort() const;
    int getPerfSvcPort() const;
    int getPerfScPort() const;

    std::string getSaaDir() const;
    std::string getFilesPrefix() const;
    // db: returning a string via value: herovo, but we goes along with the style
    std::string getFilesSuffix() const;
    int getRollingInterval() const;
    bool getEnabled() const;
	
	
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
    smsc::core::buffers::IntHash<std::string> getEventFiler() const;
		int getReconnectTimeout()const;
		int getMaxQueueLength()const;

protected:
		std::string saccHost;
		int saccPort;
    smsc::core::buffers::IntHash<std::string> eventFilter;

        std::string saaDir;
        std::string filesPrefix;
        std::string filesSuffix;
        int rollingInterval;
        bool enabled;
        smsc::logger::Logger* logger;

};

}
}

namespace scag2 {
namespace config {
using scag::config::StatManConfig;
}
}

#endif
