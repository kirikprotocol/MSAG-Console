#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <stdarg.h>
#include <memory>

#include "eyeline/ros/ROSCompFactory.hpp"

namespace eyeline {
namespace ros {

/* ************************************************************************* *
 * class ROSComponentsFactory implementation
 * ************************************************************************* */

//binds error codes to OPERATION
void ROSComponentsFactory::bindErrors(unsigned opcode, unsigned errNum, ...)
{
  std::auto_ptr<OperationErrors> verr(new OperationErrors(errNum));
  va_list  errs;
  va_start(errs, errNum);
  for (unsigned i = 0; i < errNum; ++i) {
    unsigned curErr = va_arg(errs, unsigned);
    verr->insert(verr->begin(), curErr);
  }
  va_end(errs);
  _errMap.insert(ROSErrors::value_type(opcode, verr.get()));
  verr.release();
}

//returns TRUE if ERROR identified by errcode is defined for OPERATION
bool ROSComponentsFactory::hasError(unsigned opcode, unsigned errcode) const
{
  ROSErrors::const_iterator it = _errMap.find(opcode);
  if (it != _errMap.end()) {
    const OperationErrors * verr = it->second;
    for (unsigned i = 0; i < verr->size(); ++i) {
      if ((*verr)[i] == errcode)
        return true;
    }
  }
  return false;
}

} //ros
} //eyeline

