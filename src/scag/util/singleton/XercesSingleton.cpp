#include "util/xml/init.h"
#include "XercesSingleton.h"

using namespace scag::util::singleton;

namespace scag {
namespace util {
namespace singleton {

struct XercesSingletonImpl : public XercesSingleton
{
    XercesSingletonImpl() {}
    ~XercesSingletonImpl() {}
};

}
}
}

typedef SingletonHolder< XercesSingletonImpl > SingleSM;
unsigned GetLongevity( XercesSingletonImpl* ) { return 999; }

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
