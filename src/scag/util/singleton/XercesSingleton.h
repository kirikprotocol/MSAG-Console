#ifndef _SCAG_UTIL_SINGLETON_XERCESSINGLETON_H
#define _SCAG_UTIL_SINGLETON_XERCESSINGLETON_H

#include "Singleton2.h"

namespace scag {
namespace util {
namespace singleton {

class XercesSingletonImpl;

/// NOTE: this class is to provide proper and guaranteed order of logger destruction
class XercesSingleton
{
public:
    static XercesSingleton& Instance();
private:
    friend class XercesSingletonImpl;
    XercesSingleton();
    ~XercesSingleton();
    XercesSingleton( const XercesSingleton& );
};

}
}
}

#endif /* !_SCAG_UTIL_SINGLETON_XERCESSINGLETON_H */
