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

TcapOperation::TcapOperation(TcapDialog* dlg) 
	: dialog( dlg )
	, tag( EINSS7_I97TCAP_OPERATION_TAG_LOCAL )
	, operation( NULL )
	, parameters( NULL )
{
}

TcapOperation::~TcapOperation()
{
	delete operation;
	delete parameters;
}

void TcapOperation::decode(USHORT_T opTag, USHORT_T opLen, UCHAR_T* op, USHORT_T parLen, UCHAR_T* par)
{
	tag =  opTag;
	operation  = dialog->getComponentFactory()->createComponent( vector<unsigned char>( op, op + opLen ) );
	parameters = dialog->getComponentFactory()->createComponent( vector<unsigned char>( par, par + parLen ) );
}

void TcapOperation::encode(USHORT_T& opTag, vector<unsigned char>& op, vector<unsigned char>& par)
{
	opTag = tag;
	if( operation ) operation->encode( op );
	if( parameters ) parameters->encode( par );
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

TcapOperation* TcapDialog::createOperation()
{
	return new TcapOperation( this );
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

  USHORT_T		  tag;
  vector<UCHAR_T> op;
  vector<UCHAR_T> params;
  pop->encode( tag, op, params );

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
    tag, //operationTag,
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

USHORT_T TcapDialog::resultLast(TcapOperation* res)
{
  assert( session );
  assert( res );

  USHORT_T		  tag;
  vector<UCHAR_T> op;
  vector<UCHAR_T> params;
  res->encode( tag, op, params );

	USHORT_T result = EINSS7_I97TResultLReq(
    	session->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	did,
    	1, //invokeId
		tag,
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

USHORT_T TcapDialog::resultNotLast(TcapOperation* res)
{
  assert( session );
  assert( res );

  USHORT_T		  tag;
  vector<UCHAR_T> op;
  vector<UCHAR_T> params;
  res->encode( tag, op, params );

	USHORT_T result = EINSS7_I97TResultNLReq(
    	session->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	did,
    	1, //invokeId
		tag,
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

USHORT_T TcapDialog::userError(TcapOperation* err)
{
  assert( session );
  assert( err );

  USHORT_T		  tag;
  vector<UCHAR_T> op;
  vector<UCHAR_T> params;
  err->encode( tag, op, params );

 	USHORT_T result = EINSS7_I97TUErrorReq
	(
    	session->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	did,
    	1,
		tag,
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
