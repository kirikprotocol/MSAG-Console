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

namespace smsc  {
namespace inman {
namespace inap  {

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
    const_cast<UCHAR_T*>(&op.front()), //*operationCode_p,
    params.size(), //paramLength,
    const_cast<UCHAR_T*>(&params.front()) //*parameters_p
    ); 

  if(result != 0)
  	throw runtime_error( format("InvokeReq failed with code %d (%s)", result,getTcapReasonDescription(result)));
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
