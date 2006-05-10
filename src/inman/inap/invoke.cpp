static char const ident[] = "$Id$";
#include "inman/inap/invoke.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

//NOTE: resWait is forbidden
void Invoke::notifyResultListener(TcapEntity* resp, InvokeStatus resKind)
{
    if (_iResHdl) {
        switch (resKind) {
        case Invoke::resLast: {
            _iResHdl->onInvokeResult(this, resp);
            _iResHdl = NULL;
        } break;
        case Invoke::resNotLast:
            _iResHdl->onInvokeResultNL(this, resp);
        break;
        case Invoke::resError: {
            _iResHdl->onInvokeError(this, resp);
            _iResHdl = NULL;
        } break;
        case Invoke::resLCancel: {
            _iResHdl->onInvokeLCancel(this);
            _iResHdl = NULL;
        } break;
        default:;
        }
    }
    _status = resKind;
}

} // namespace inap
} // namespace inmgr
} // namespace smsc

