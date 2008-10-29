#include "logger/Logger.h"
#include "LoggerSingleton.h"

using namespace scag::util::singleton;

typedef SingletonHolder< LoggerSingleton > SingleSM;
unsigned GetLongevity( LoggerSingleton* ) { return 0xffffffff; }

namespace scag {
namespace util {
namespace singleton {

LoggerSingleton& LoggerSingleton::Instance()
{
    return SingleSM::Instance();
}

LoggerSingleton::LoggerSingleton()
{
    smsc::logger::Logger::Init();
}

LoggerSingleton::~LoggerSingleton()
{
    smsc::logger::Logger::Shutdown();
}

}
}
}
