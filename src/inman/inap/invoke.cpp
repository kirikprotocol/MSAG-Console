static char const ident[] = "$Id$";
#include "inman/inap/dialog.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

void Invoke::send(void) //throws runtime_error
{
    _dlg->sendInvoke(this);
}

void Invoke::notifyResultListener(TcapEntity* resp)
{
    if (_iResHdl)
        _iResHdl->result(resp);
    _status = Invoke::resLast;
}

void Invoke::notifyResultNListener(TcapEntity* resp)
{
    if (_iResHdl)
        _iResHdl->resultNL(resp);
    _status = Invoke::resNotLast;
}

void Invoke::notifyErrorListener(TcapEntity* resp)
{
    if (_iResHdl)
        _iResHdl->error(resp);
    _status = Invoke::resError;
}

void Invoke::notifyLCancelListener(void)
{
    if (_iResHdl)
        _iResHdl->lcancel();
    _status = Invoke::resLCancel;
}

} // namespace inap
} // namespace inmgr
} // namespace smsc

