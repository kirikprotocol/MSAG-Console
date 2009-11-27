#include <cstdio>
#include "logger/Logger.h"
#include "util/config/Config.h"
#include "core/buffers/File.hpp"

void usage( const char* prog )
{
    fprintf(stderr,"Usage: %s FULLPATH/config.xml\n",prog);
    exit(-1);
}

using namespace smsc::util::config;

int main( int argc, char** argv )
{
    if ( argc != 2 ) usage( argv[0] );
    smsc::logger::Logger::initForTest(smsc::logger::Logger::LEVEL_DEBUG);

    const std::string xmlFile(argv[1]);

    std::auto_ptr<Config> rootConfig( Config::createFromFile(xmlFile.c_str()) );

    // extract tasks section
    std::auto_ptr<Config> taskConfig( rootConfig->getSubConfig("InfoSme.Tasks",true) );

    const std::string storeLocation = rootConfig->getString("InfoSme.storeLocation");
    if ( ! smsc::core::buffers::File::Exists(storeLocation.c_str()) ) {
        smsc::core::buffers::File::MkDir(storeLocation.c_str());
    }

    std::auto_ptr<CStrSet> taskNames( taskConfig->getRootSectionNames() );
    for ( CStrSet::const_iterator i = taskNames->begin(); i != taskNames->end(); ++i ) {
        std::cout << "task #" << *i << std::endl;
        const std::string taskLoc = storeLocation + "/" + *i;
        if ( ! smsc::core::buffers::File::Exists(taskLoc.c_str()) ) {
            smsc::core::buffers::File::MkDir(taskLoc.c_str());
        }
        std::auto_ptr<Config> conf( taskConfig->getSubConfig(i->c_str(),false) );
        conf->saveToFile( (taskLoc + "/config.xml").c_str() );
    }

    rootConfig->saveToFile( (xmlFile + ".root").c_str() );
    return 0;
}
