static char const ident[] = "$Id$";
#include <assert.h>
#include <stdexcept>
#include <string>

#include "inman/common/util.hpp"
#include "inman/inap/inss7util.hpp"
#include "inman/inap/invoke.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/inap/session.hpp"

using std::runtime_error;

using smsc::inman::common::format;
using smsc::inman::common::dump;

namespace smsc  {
namespace inman {
namespace inap  {

extern Logger* tcapLogger;

void Invoke::send(Dialog* dialog)
{
  assert( dialog );
  assert( dialog->getSession() );

  RawBuffer op;
  RawBuffer params;
  encode( op, params );

  smsc_log_debug(tcapLogger,
                 "EINSS7_I97TInvokeReq("
                 "ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                 "invokeId=%d, lunkedused=\"NO\", linkedid=0, "
                 "tag=\"%s\", "
                 "opcode[%d]={%s}, "
                 "parameters[%d]={%s})",
                 dialog->getSession()->getSSN(), MSG_USER_ID, TCAP_INSTANCE_ID, dialog->getId(),
                 id,
                 tag==0x02?"LOCAL":"GLOBAL",
                 op.size(),dump(op.size() ,&op[0] ).c_str(),
                 params.size(),dump(params.size(),&params[0] ).c_str()
                );

  UCHAR_T result = EINSS7_I97TInvokeReq
  (
    dialog->getSession()->getSSN(),
    MSG_USER_ID,
    TCAP_INSTANCE_ID,
    dialog->getId(),
    id,
    0, //linkedIdUsed,
    0, //linkedId,
    EINSS7_I97TCAP_OP_CLASS_1, //opClass,
    dialog->getTimeout(),
    tag, //operationTag,
    op.size(), //operationLength,
    &op[0], //*operationCode_p,
    params.size(), //paramLength,
    &params[0] //*parameters_p
    );


  if(result != 0)
    throw runtime_error( format("InvokeReq failed with code %d (%s)", result,getTcapReasonDescription(result)));
}

void Invoke::notifyResultListener(TcapEntity* resp)
{
    if (_iResHdl)
        _iResHdl->result(resp);
}

void Invoke::notifyResultNListener(TcapEntity* resp)
{
    if (_iResHdl)
        _iResHdl->resultNL(resp);
}

void Invoke::notifyErrorListener(TcapEntity* resp)
{
    if (_iResHdl)
        _iResHdl->error(resp);
}

void Invoke::notifyLCancelListener(void)
{
    if (_iResHdl)
        _iResHdl->lcancel();
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
