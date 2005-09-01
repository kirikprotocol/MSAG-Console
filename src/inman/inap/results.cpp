static char const ident[] = "$Id$";
#include <assert.h>
#include <stdexcept>
#include <string>

#include "inman/common/util.hpp"

#include "factory.hpp"
#include "session.hpp"
#include "dialog.hpp"
#include "results.hpp"

using std::runtime_error;
using smsc::inman::common::format;
using smsc::inman::common::getTcapReasonDescription;

namespace smsc  {
namespace inman {
namespace inap  {

void InvokeResultLast::send(TcapDialog* dialog)
{
  assert( dialog );
  assert( dialog->getSession() );

  RawBuffer op;
  RawBuffer params;
  encode( op, params );

  UCHAR_T result = EINSS7_I97TResultLReq
  (
    	dialog->getSession()->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	dialog->getId(),
    	id,
		tag,
		op.size(), 	//operationLength,
    	const_cast<UCHAR_T*>(&op.front()), //*operationCode_p,
		params.size(), //paramLength,
    	const_cast<UCHAR_T*>(&params.front()) //*parameters_p
   );

  if(result != 0)
  	throw runtime_error( format("InvokeReq failed with code %d (%s)", result,getTcapReasonDescription(result)));

}

void InvokeResultNotLast::send(TcapDialog* dialog)
{
  assert( dialog );
  assert( dialog->getSession() );

  RawBuffer op;
  RawBuffer params;
  encode( op, params );

  UCHAR_T result = EINSS7_I97TResultNLReq
  (
    	dialog->getSession()->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	dialog->getId(),
    	id,
		tag,
		op.size(), 	//operationLength,
    	const_cast<UCHAR_T*>(&op.front()), //*operationCode_p,
		params.size(), //paramLength,
    	const_cast<UCHAR_T*>(&params.front()) //*parameters_p
   );

  if(result != 0)
  	throw runtime_error( format("InvokeReq failed with code %d (%s)", result,getTcapReasonDescription(result)));

}

void InvokeResultError::send(TcapDialog* dialog)
{
  assert( dialog );
  assert( dialog->getSession() );

  RawBuffer op;
  RawBuffer params;
  encode( op, params );

  UCHAR_T result = EINSS7_I97TUErrorReq
  (
    	dialog->getSession()->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	dialog->getId(),
    	id,
		tag,
		op.size(), 	//operationLength,
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
