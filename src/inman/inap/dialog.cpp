static char const ident[] = "$Id$";
// Реализация INAP интерфейса на платформе ETSI

#include "operations.hpp"
#include "dialog.hpp"
#include "session.hpp"
#include "factory.hpp"
#include "util.hpp"

#include <assert.h>

using std::map;
using std::pair;

namespace smsc  {
namespace inman {
namespace inap  {


template <typename listener_t, typename source_t>
void notifyListeners(std::list<listener_t*>& listeners, source_t source, void (listener_t::*method)(source_t))
{
		for( typename std::list<listener_t*>::iterator it = listeners.begin();
		it != listeners.end(); it++)
	 	{
			 listener_t* ptr = *it;
			 (*ptr.*method)( source );
		}
}

TcapOperation::TcapOperation() : tag( EINSS7_I97TCAP_OPERATION_TAG_LOCAL )
{
}

TcapOperation::TcapOperation(USHORT_T opTag, USHORT_T opLen, UCHAR_T* op, USHORT_T parLen, UCHAR_T* par ) 
		: tag( opTag )
		, operation( op, op + opLen )
		, params( par, par + parLen )
{
}

TcapOperation::~TcapOperation()
{
}

/////////////////////////////////////////////////////////////////////////////////////
// Dialog class
/////////////////////////////////////////////////////////////////////////////////////


Dialog::Dialog(Session* pSession, USHORT_T dlgId) 
	: did( dlgId )
	, session( pSession )
	, qSrvc(EINSS7_I97TCAP_QLT_BOTH)
	, priority(EINSS7_I97TCAP_PRI_HIGH_0)
{
}

Dialog::~Dialog()
{
	listeners.clear();
}

void Dialog::addDialogListener(DialogListener* l)
{
	listeners.push_back( l );
}

void Dialog::removeDialogListener(DialogListener* l)
{
	listeners.remove( l );
}

USHORT_T Dialog::beginDialog()
{
 USHORT_T result = EINSS7_I97TBeginReq(
    session->getSSN(),
    MSG_USER_ID,
    TCAP_INSTANCE_ID,
    did,
    priority,
    qSrvc,
    session->scfAddr.addrLen,
    session->scfAddr.addr,
    session->inmanAddr.addrLen,
    session->inmanAddr.addr,
    session->ac.acLen,
    session->ac.ac,
    0,
    NULL);

  if(result != 0)
  {
  	smsc_log_error(tcapLogger, "EINSS7_I97TBeginReq failed with code %d(%s)", result,getTcapReasonDescription(result));
  }

  return result;
}

USHORT_T Dialog::continueDialog()
{
  USHORT_T result = EINSS7_I97TContinueReq(
    session->getSSN(),
    MSG_USER_ID,
    TCAP_INSTANCE_ID,
    did,
    priority,
    qSrvc,
    session->inmanAddr.addrLen,
    session->inmanAddr.addr,
    session->ac.acLen,
    session->ac.ac,
    0,
    NULL);

  if(result != 0)
  {
  	smsc_log_error(tcapLogger, "EINSS7_I97TContinueReq failed with code %d(%s)", result,getTcapReasonDescription(result));
  }
  return result;
}

USHORT_T Dialog::endDialog(USHORT_T termination)
{
  USHORT_T result = EINSS7_I97TEndReq(
    session->getSSN(),
    MSG_USER_ID,
    TCAP_INSTANCE_ID,
    did,
    priority,
    qSrvc,
    termination,
    session->ac.acLen,
    session->ac.ac,
    0,
    NULL);

  if(result != 0)
  {
  	smsc_log_error(tcapLogger, "EINSS7_I97TEndReq failed with code %d(%s)", result,getTcapReasonDescription(result));
  }

  return result;
}


USHORT_T Dialog::invoke(const TcapOperation& op)
{
  assert( session );

  USHORT_T result = EINSS7_I97TInvokeReq(
    session->getSSN(),
    MSG_USER_ID,
    TCAP_INSTANCE_ID,
    did,
    1, //invokeId
    0, //linkedIdUsed,
    0, //linkedId,
    EINSS7_I97TCAP_OP_CLASS_1, //opClass,
    30, //timeOut,
    op.getTag(), //operationTag,
    op.getOperation().size(), //operationLength,
    const_cast<UCHAR_T*>(op.getOperation().begin()), //*operationCode_p,
    op.getParams().size(), //paramLength,
    const_cast<UCHAR_T*>(op.getParams().begin()) //*parameters_p
    ); 

  if (result != 0)
  {
  	smsc_log_error(tcapLogger, "EINSS7_I97TInvokeReq failed with code %d(%s)", result,getTcapReasonDescription(result));
  }

  return result;
}

USHORT_T Dialog::invokeSuccessed(const TcapOperation& res)
{
  assert( session );

	USHORT_T result = EINSS7_I97TResultLReq(
    	session->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	did,
    	1, //invokeId
		res.getTag(),
    	res.getOperation().size(), //operationLength,
    	const_cast<UCHAR_T*>(res.getOperation().begin()), //*operationCode_p,
    	res.getParams().size(), //paramLength,
    	const_cast<UCHAR_T*>(res.getParams().begin()) //*parameters_p
    	);

  if (result != 0)
  {
  	smsc_log_error(tcapLogger, "EINSS7_I97TResultLReq failed with code %d(%s)", result,getTcapReasonDescription(result));
  }

  return result;
}

USHORT_T Dialog::invokeFailed(const TcapOperation& err)
{
 	USHORT_T result = EINSS7_I97TUErrorReq
	(
    	session->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	did,
    	1,
		err.getTag(),
   		err.getOperation().size(), //operationLength,
   		const_cast<UCHAR_T*>(err.getOperation().begin()), //*operationCode_p,
   		err.getParams().size(), //paramLength,
   		const_cast<UCHAR_T*>(err.getParams().begin()) //*parameters_p
	);

  if (result != 0)
  {
  	smsc_log_error(tcapLogger, "EINSS7_I97TResultLReq failed with code %d(%s)", result,getTcapReasonDescription(result));
  }
  return result;
}

void Dialog::fireInvoke(const TcapOperation& op)
{
	notifyListeners<DialogListener,const TcapOperation&>(listeners, op, &DialogListener::invoke);
}

void Dialog::fireInvokeSuccessed(const TcapOperation& op)
{
	notifyListeners<DialogListener,const TcapOperation&>(listeners, op, &DialogListener::invokeSuccessed);
}

void Dialog::fireInvokeFailed(const TcapOperation& op)
{
	notifyListeners<DialogListener,const TcapOperation&>(listeners, op, &DialogListener::invokeFailed);
}

} // namespace inap
} // namespace inman
} // namespace smsc
