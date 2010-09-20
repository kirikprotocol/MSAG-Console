#include "InfosmeComponent.h"

namespace smsc {
namespace infosme {

InfosmeComponent::InfosmeComponent( InfosmeCore& core ) :
log_(0),
core_(core) {}


smsc::admin::service::Variant InfosmeComponent::call
    ( const smsc::admin::service::Method& method,
      const smsc::admin::service::Arguments& args )
    throw (smsc::admin::AdminException)
{
    throw smsc::admin::AdminException("not impl");
}


}
}
