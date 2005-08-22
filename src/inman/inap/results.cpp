static char const ident[] = "$Id$";
#include <assert.h>
#include "factory.hpp"
#include "session.hpp"
#include "dialog.hpp"
#include "results.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

UCHAR_T InvokeResultLast::send(TcapDialog* dialog)
{
  assert( dialog );
  assert( dialog->getSession() );

  RawBuffer op;
  RawBuffer params;
  encode( op, params );

  return EINSS7_I97TResultLReq
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
}

UCHAR_T InvokeResultNotLast::send(TcapDialog* dialog)
{
  assert( dialog );
  assert( dialog->getSession() );

  RawBuffer op;
  RawBuffer params;
  encode( op, params );

  return EINSS7_I97TResultNLReq
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
}

UCHAR_T InvokeResultError::send(TcapDialog* dialog)
{
  assert( dialog );
  assert( dialog->getSession() );

  RawBuffer op;
  RawBuffer params;
  encode( op, params );

  return EINSS7_I97TUErrorReq
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
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
