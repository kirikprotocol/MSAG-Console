#include "StatManConfig.h"

namespace scag {
namespace config {

StatManConfig::StatManConfig()
{
    dir = "";
    perfHost = "";
    perfGenPort = 0;
    perfSvcPort = 0;
    perfScPort = 0;
    saccPort=0;
    saccHost="";
    eventFilter.Empty();
    connect_timeout=1000;
    queue_length=100000;

}
#ifdef TESTING
/*
for testing only!
created by 
Gregory Panin green@sbingo.com
*/
StatManConfig::StatManConfig(std::string& directory,std::string& host,int genp,int svcp,int scp) throw(ConfigException)
{	    
	try{
	if(directory.length()==0)
	                    throw ConfigException("StatManConfig.StatManConfig, stat dir. length ==0! ");
	if(host.length()==0)
                	    throw ConfigException("StatManConfig.StatManConfig, host. length ==0! ");			    
	    dir = directory;
	    perfHost = host;
	    perfGenPort = genp;
	    perfSvcPort = svcp;
	    perfScPort  = scp;	

	}
	catch(ConfigException& e)
	{
	    throw(ConfigException(e.what()));
	}
}
#endif

StatManConfig::StatManConfig(const ConfigView& cv)  throw(ConfigException)
{
    try{
        std::auto_ptr<char> dir_( cv.getString("statisticsDir") );
        dir = dir_.get();

        std::auto_ptr<char> perfHost_( cv.getString("perfHost") );
        perfHost = perfHost_.get();
        perfGenPort = cv.getInt("perfGenPort");
        perfSvcPort = cv.getInt("perfSvcPort");
        perfScPort  = cv.getInt("perfScPort");

 		
		std::auto_ptr<char> sch (cv.getString("saccHost"));
		saccHost    = sch.get();
		saccPort    = cv.getInt("saccPort");
		connect_timeout=cv.getInt("connect_timeout");;
		queue_length   =cv.getInt("queue_length");;

		//zalipa
		eventFilter.Insert(0,"deliver");
		eventFilter.Insert(1,"submit");
		eventFilter.Insert(3,"billing");

    }
	catch(ConfigException& e)
	{
        throw ConfigException(e.what());
    }
	catch(...)
	{
        throw ConfigException("StatManConfig.StatManConfig, Unknown exception.");
    }
}

void StatManConfig::init(const ConfigView& cv) throw(ConfigException)
{
    try {
        std::auto_ptr<char> dir_( cv.getString("statisticsDir") );
        dir = dir_.get();

        std::auto_ptr<char> perfHost_( cv.getString("perfHost") );
        perfHost = perfHost_.get();
        perfGenPort = cv.getInt("perfGenPort");
        perfSvcPort = cv.getInt("perfSvcPort");
        perfScPort  = cv.getInt("perfScPort");

 		connect_timeout=cv.getInt("connect_timeout");;
		queue_length   =cv.getInt("queue_length");;

		std::auto_ptr<char> sch(cv.getString("saccHost"));
		saccHost    = sch.get();
		saccPort    = cv.getInt("saccPort");


    }
	catch(ConfigException& e)
	{
        throw ConfigException(e.what());
    }
	catch(...)
	{
        throw ConfigException("StatManConfig.init, Unknown exception.");
    }
}

bool StatManConfig::check(const ConfigView& cv)  throw(ConfigException)
{   
    try {
        std::auto_ptr<char> dir_( cv.getString("statisticsDir") );
        if(   !strcmp( dir.c_str(), dir_.get() )   )
            return false;

        std::auto_ptr<char> perfHost_( cv.getString("perfHost") );
        if(   !strcmp( perfHost.c_str(), perfHost_.get() )   )
            return false;

        if(perfGenPort != cv.getInt("perfGenPort"))
            return false;
        if(perfSvcPort != cv.getInt("perfSvcPort"))
            return false;
        if(perfScPort  != cv.getInt("perfScPort"))
            return false;

        return true;

    }
	catch(ConfigException& e)
	{
        throw ConfigException(e.what());
    }
	catch(...)
	{
        throw ConfigException("StatManConfig.check, Unknown exception.");
    }
}

std::string StatManConfig::getDir() const { return dir; }
std::string StatManConfig::getPerfHost() const { return perfHost; }
int StatManConfig::getPerfGenPort() const { return perfGenPort; }
int StatManConfig::getPerfSvcPort() const { return perfSvcPort; }
int StatManConfig::getPerfScPort() const { return perfScPort; }
int StatManConfig::getReconnectTimeout()const {return connect_timeout;}
int StatManConfig::getMaxQueueLength()const {return queue_length;}

 
int StatManConfig::getSaccPort() const{return saccPort;}
std::string StatManConfig::getSaccHost()const{return saccHost;}
IntHash<std::string> StatManConfig::getEventFiler()const{return eventFilter;}

}
}
