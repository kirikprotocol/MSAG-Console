static char const ident[] = "$Id$";

#include "inman/inap/dialog.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

void InvokeResultLast::send(void) //throws runtime_error
{
    _dlg->sendResultLast(this);
}

void InvokeResultNotLast::send(void) //throws runtime_error
{
    _dlg->sendResultNotLast(this);
}

void InvokeResultError::send(void) //throws runtime_error
{
    _dlg->sendResultError(this);
}


} // namespace inap
} // namespace inmgr
} // namespace smsc
