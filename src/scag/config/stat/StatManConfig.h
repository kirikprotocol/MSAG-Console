#ifndef StatManConf_dot_h
#define StatManConf_dot_h

#include "scag/config/ConfigView.h"
#include <core/buffers/IntHash.hpp>
namespace scag {
namespace config {

class StatManConfig {
public:    
    StatManConfig();
    StatManConfig(ConfigView& cv)  throw(ConfigException);
    void init(ConfigView& cv)  throw(ConfigException);   
    bool check(ConfigView& cv)  throw(ConfigException);
    
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

		
public:
		int getSaccPort() const;
		std::string getSaccHost() const;
		IntHash<std::string> getEventFiler() const;

protected:
		std::string saccHost;
		int saccPort;
		IntHash<std::string> eventFilter;

};

}
}

#endif
