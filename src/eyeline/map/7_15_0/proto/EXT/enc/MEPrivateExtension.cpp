#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/map/7_15_0/proto/EXT/enc/MEPrivateExtension.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

/* Type is defined in IMPLICIT tagging environment as follow:
  PrivateExtension ::= SEQUENCE {
    extId       MAP-EXTENSION.&extensionId ({ExtensionSet}),
    extType     MAP-EXTENSION.&ExtensionType ({ExtensionSet}{@extId}) OPTIONAL
  }
*/
void
MEPrivateExtension::setValue(const PrivateExtension& value)
{
  _eExtId.setValue(value.extId);
  if (value.extType.get()) {
    _eExtType.init(getTSRule()).setValue(*value.extType.get());
    setField(1, *_eExtType.get());
  } else
    clearField(1);
}

}}}}
