static char const ident[] = "$Id$";
#include <assert.h>
#include <stdexcept>
#include <string>
#include "inman/common/util.hpp"

#include "factory.hpp"
#include "session.hpp"
#include "invoke.hpp"
#include "dialog.hpp"

using std::runtime_error;
using smsc::inman::common::format;
using smsc::inman::common::getTcapReasonDescription;
using smsc::inman::common::dump;

namespace smsc  {
namespace inman {
namespace inap  {

extern Logger* tcapLogger;

void Invoke::send(TcapDialog* dialog)
{
  assert( dialog );
  assert( dialog->getSession() );

  RawBuffer op;
  RawBuffer params;
  encode( op, params );
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

  smsc_log_debug( tcapLogger, "INVOKE_REQ" );
  smsc_log_debug( tcapLogger, " id=%d", id );
  smsc_log_debug( tcapLogger, " tag=%d", tag );
  smsc_log_debug( tcapLogger, " op=%s", dump( op.size(), &op[0]).c_str() );
  smsc_log_debug( tcapLogger, " params=%s", dump( params.size(), &params[0]).c_str());

  if(result != 0)
  	throw runtime_error( format("InvokeReq failed with code %d (%s)", result,getTcapReasonDescription(result)));
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
