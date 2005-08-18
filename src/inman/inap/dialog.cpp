static char const ident[] = "$Id$";
// Реализация INAP интерфейса на платформе ETSI

#include "operations.hpp"
#include "dialog.hpp"
#include "session.hpp"
#include "factory.hpp"
#include "inman/common/util.hpp"

#include <assert.h>

using std::map;
using std::pair;

namespace smsc  {
namespace inman {
namespace inap  {

TcapOperation::TcapOperation(TcapDialog* dlg, UCHAR_T tg, UCHAR_T op)
	: dialog( dlg )
	, opcode( op )
	, tag( tg )
	, param( NULL )
{
}

TcapOperation::~TcapOperation()
{
	delete param;
}

void TcapOperation::invoke()
{
	dialog->invoke( this );
}

/////////////////////////////////////////////////////////////////////////////////////
// TcapDialog class
/////////////////////////////////////////////////////////////////////////////////////


TcapDialog::TcapDialog(Session* pSession, USHORT_T dlgId) 
	: did( dlgId )
	, session( pSession )
	, qSrvc(EINSS7_I97TCAP_QLT_BOTH)
	, priority(EINSS7_I97TCAP_PRI_HIGH_0)
{
}

TcapDialog::~TcapDialog()
{
}


void TcapDialog::setProtocolFactory( ProtocolFactory* pf )
{
	protFactory = pf;
	if( protocol ) delete protocol;
	protocol = protFactory->createProtocol( this );
}

TcapOperation* TcapDialog::createOperation(UCHAR_T tag, UCHAR_T opcode, const UCHAR_T* paramBuf, USHORT_T paramLen )
{
	TcapOperation* op = new TcapOperation( this, tag, opcode );
	if( paramBuf && (paramLen > 0))
	{
		Component* comp = compFactory.createComponent( opcode );
		if( comp )
		{
			comp->decode( vector<UCHAR_T>( paramBuf, paramBuf + paramLen ) );
			op->setParam( comp );
		}
	}
	return op; 
}

TcapOperation* TcapDialog::createOperation(UCHAR_T opcode)
{
	return createOperation( EINSS7_I97TCAP_OPERATION_TAG_LOCAL, opcode, NULL, 0 );
}

USHORT_T TcapDialog::beginDialog()
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

USHORT_T TcapDialog::continueDialog()
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

USHORT_T TcapDialog::endDialog(USHORT_T termination)
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


USHORT_T TcapDialog::invoke(TcapOperation* pop)
{
  assert( session );
  assert( pop );

  vector<UCHAR_T> op;
  op.push_back( pop->getOpcode() );

  vector<UCHAR_T> params;
  
  if( pop->getParam() ) pop->getParam()->encode( params );

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
    pop->getTag(), //operationTag,
    op.size(), //operationLength,
    const_cast<UCHAR_T*>(&op.front()), //*operationCode_p,
    params.size(), //paramLength,
    const_cast<UCHAR_T*>(&params.front()) //*parameters_p
    ); 

  if (result != 0)
  {
  	smsc_log_error(tcapLogger, "EINSS7_I97TInvokeReq failed with code %d(%s)", result,getTcapReasonDescription(result));
  }
  return result;
}

USHORT_T TcapDialog::resultLast(TcapOperation* pop)
{
  assert( session );
  assert( pop );

  vector<UCHAR_T> op;
  op.push_back( pop->getOpcode() );

  vector<UCHAR_T> params;

  if( pop->getParam() ) pop->getParam()->encode( params );

	USHORT_T result = EINSS7_I97TResultLReq(
    	session->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	did,
    	1, //invokeId
		pop->getTag(),
		op.size(), //operationLength,
    	const_cast<UCHAR_T*>(&op.front()), //*operationCode_p,
		params.size(), //paramLength,
    	const_cast<UCHAR_T*>(&params.front()) //*parameters_p
    	);

  if (result != 0)
  {
  	smsc_log_error(tcapLogger, "EINSS7_I97TResultLReq failed with code %d(%s)", result,getTcapReasonDescription(result));
  }

  return result;
}

USHORT_T TcapDialog::resultNotLast(TcapOperation* pop)
{
  assert( session );
  assert( pop );

  vector<UCHAR_T> op;
  op.push_back( pop->getOpcode() );

  vector<UCHAR_T> params;
  if( pop->getParam() ) pop->getParam()->encode( params );

	USHORT_T result = EINSS7_I97TResultNLReq(
    	session->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	did,
    	1, //invokeId
		pop->getTag(),
		op.size(), //operationLength,
    	const_cast<UCHAR_T*>(&op.front()), //*operationCode_p,
    	params.size(), //paramLength,
    	const_cast<UCHAR_T*>(&params.front()) //*parameters_p
    	);

  if (result != 0)
  {
  	smsc_log_error(tcapLogger, "EINSS7_I97TResultLReq failed with code %d(%s)", result,getTcapReasonDescription(result));
  }

  return result;
}

USHORT_T TcapDialog::userError(TcapOperation* pop)
{
  assert( session );
  assert( pop );

  vector<UCHAR_T> op;
  op.push_back( pop->getOpcode() );

  vector<UCHAR_T> params;
  if( pop->getParam() ) pop->getParam()->encode( params );

 	USHORT_T result = EINSS7_I97TUErrorReq
	(
    	session->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	did,
    	1,
		pop->getTag(),
		op.size(), //operationLength,
   		const_cast<UCHAR_T*>(&op.front()), //*operationCode_p,
		params.size(), //paramLength,
   		const_cast<UCHAR_T*>(&params.front()) //*parameters_p
	);

  if (result != 0)
  {
  	smsc_log_error(tcapLogger, "EINSS7_I97TResultLReq failed with code %d(%s)", result,getTcapReasonDescription(result));
  }
  return result;
}

USHORT_T TcapDialog::timerReset()
{
 	USHORT_T result = EINSS7_I97TTimerResetReq
 	(
    	session->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	did,
    	1
    );

   if (result != 0)
   {
  	 smsc_log_error(tcapLogger, "EINSS7_I97TTimerResetReq failed with code %d(%s)", result,getTcapReasonDescription(result));
   }
   return result;
}

//--------------------------------------- Callbacks ----------------------------------

USHORT_T TcapDialog::handleBeginDialog()
{
	return MSG_OK;
}

USHORT_T TcapDialog::handleContinueDialog()
{
	return MSG_OK;
}

USHORT_T TcapDialog::handleEndDialog()
{
	return MSG_OK;
}

USHORT_T TcapDialog::handleInvoke(TcapOperation* op)
{
	return MSG_OK;
}

USHORT_T TcapDialog::handleResultLast(TcapOperation* op)
{
	return MSG_OK;
}

USHORT_T TcapDialog::handleResultNotLast(TcapOperation* op)
{
	return MSG_OK;
}

USHORT_T TcapDialog::handleUserError(TcapOperation* op)
{
	return MSG_OK;
}

} // namespace inap
} // namespace inman
} // namespace smsc

