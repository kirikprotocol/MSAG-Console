#ifndef _SCAG_UTIL_PROPERTIES_ACCESSTYPE_H
#define _SCAG_UTIL_PROPERTIES_ACCESSTYPE_H

namespace scag { namespace re {
    typedef enum AccessType 
    {
        atNoAccess = 0,
        atRead  = 1,
        atWrite = 2,
        atReadWrite = 3
    } AccessType;

}};

namespace scag2 {
namespace re {
using scag::re::AccessType;
using scag::re::atNoAccess;
using scag::re::atRead;
using scag::re::atWrite;
using scag::re::atReadWrite;
}
}


#endif /* !_SCAG_UTIL_PROPERTIES_ACCESSTYPE_H */
