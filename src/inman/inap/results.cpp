static char const ident[] = "$Id$";

#include "inman/inap/dialog.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

void InvokeResultLast::send(void) throw(CustomException)
{
    _dlg->sendResultLast(this);
}

void InvokeResultNotLast::send(void) throw(CustomException)
{
    _dlg->sendResultNotLast(this);
}

void InvokeResultError::send(void) throw(CustomException)
{
    _dlg->sendResultError(this);
}


} // namespace inap
} // namespace inmgr
} // namespace smsc
