#include "util/xml/init.h"
#include "XercesSingleton.h"

using namespace scag::util::singleton;

typedef SingletonHolder< XercesSingleton > SingleSM;
unsigned GetLongevity( XercesSingleton* ) { return 999; }

namespace scag {
namespace util {
namespace singleton {

XercesSingleton& XercesSingleton::Instance()
{
    return SingleSM::Instance();
}

XercesSingleton::XercesSingleton()
{
    smsc::util::xml::initXerces();
}

XercesSingleton::~XercesSingleton()
{
    smsc::util::xml::TerminateXerces();
}

}
}
}
