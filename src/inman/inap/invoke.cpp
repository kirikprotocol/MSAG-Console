static char const ident[] = "$Id$";
#include <assert.h>
#include <stdexcept>
#include <string>
#include "inman/common/util.hpp"

#include "session.hpp"
#include "invoke.hpp"
#include "dialog.hpp"
#include "infactory.hpp"

using std::runtime_error;
using smsc::inman::common::format;
using smsc::inman::common::getTcapReasonDescription;
using smsc::inman::common::dump;
using smsc::inman::common::dumpToLog;

namespace smsc  {
namespace inman {
namespace inap  {

extern Logger* tcapLogger;
extern Logger* dumpLogger;

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

void Invoke::notifyResultListeners(TcapEntity* resp)
{
    Invoke::ListenerList    cpList = listeners;
    for (Invoke::ListenerList::iterator it = cpList.begin();
                                      it != cpList.end(); it++) {
        InvokeListener* ptr = *it;
        ptr->result(resp);
    }
}

void Invoke::notifyResultNListeners(TcapEntity* resp)
{
    Invoke::ListenerList    cpList = listeners;
    for (Invoke::ListenerList::iterator it = cpList.begin();
                                      it != cpList.end(); it++) {
        InvokeListener* ptr = *it;
        ptr->resultNL(resp);
    }
}

void Invoke::notifyErrorListeners(TcapEntity* resp)
{
    Invoke::ListenerList    cpList = listeners;
    for (Invoke::ListenerList::iterator it = cpList.begin();
                                      it != cpList.end(); it++) {
        InvokeListener* ptr = *it;
        ptr->error(resp);
    }
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
