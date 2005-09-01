static char const ident[] = "$Id$";

#include <assert.h>
#include <string>
#include <stdexcept>

#include "invoke.hpp"
#include "results.hpp"
#include "dialog.hpp"
#include "session.hpp"
#include "factory.hpp"

#include "inman/common/util.hpp"
#include "inman/comp/comfactory.hpp"

using std::runtime_error;
using smsc::inman::common::format;
using smsc::inman::common::getTcapReasonDescription;
using smsc::inman::comp::ComponentFactory;

namespace smsc  {
namespace inman {
namespace inap  {

/////////////////////////////////////////////////////////////////////////////////////
// TcapDialog class
/////////////////////////////////////////////////////////////////////////////////////


TcapDialog::TcapDialog(Session* pSession, USHORT_T dlgId) 
	: did( dlgId )
	, session( pSession )
	, qSrvc(EINSS7_I97TCAP_QLT_BOTH)
	, priority(EINSS7_I97TCAP_PRI_HIGH_0)
	, timeout( 30 )
{
}

TcapDialog::~TcapDialog()
{
}

void TcapDialog::beginDialog()
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
  	throw runtime_error( format("BeginReq failed with code %d (%s)", result,getTcapReasonDescription(result)));
}

void TcapDialog::continueDialog()
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
  	throw runtime_error( format("ContinueReq failed with code %d (%s)", result,getTcapReasonDescription(result)));
}

void TcapDialog::endDialog(USHORT_T termination)
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
  	throw runtime_error( format("EndReq failed with code %d (%s)", result,getTcapReasonDescription(result)));
}


void TcapDialog::timerReset()
{
 	USHORT_T result = EINSS7_I97TTimerResetReq
 	(
    	session->getSSN(),
    	MSG_USER_ID,
    	TCAP_INSTANCE_ID,
    	did,
    	1
    );

  if(result != 0)
  	throw runtime_error( format("TimerResetReq failed with code %d (%s)", result,getTcapReasonDescription(result)));

}

Invoke* TcapDialog::invoke(UCHAR_T opcode)
{
	Invoke* invoke = new Invoke();
	invoke->setId( getNextInvokeId() );
	invoke->setTag( EINSS7_I97TCAP_OPERATION_TAG_LOCAL );
	invoke->setOpcode( opcode );
	originating.insert( InvokeMap::value_type(	invoke->getId(), invoke ) );
	return MSG_OK;
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

USHORT_T TcapDialog::handleInvoke(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
  assert( op );
  assert( oplen > 0 );
  assert( tag == EINSS7_I97TCAP_OPERATION_TAG_LOCAL );

  UCHAR_T opcode = op[0];

  Invoke* invoke = new Invoke();
  invoke->setId( invId );
  invoke->setTag( tag );
  invoke->setOpcode( opcode );
  invoke->setParam( ComponentFactory::getInstance()->createComponent( opcode ) );

  terminating.insert( InvokeMap::value_type( invoke->getId(), invoke ) );

  notify1( &TcapDialogListener::invoke, invoke );

  return MSG_OK;
}

USHORT_T TcapDialog::handleResultLast(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
  assert( op );
  assert( oplen > 0 );
  assert( tag == EINSS7_I97TCAP_OPERATION_TAG_LOCAL );

  UCHAR_T opcode = op[0];

  InvokeResultLast* result = new InvokeResultLast();
  result->setId( invId );
  result->setTag( tag );
  result->setOpcode( opcode );
  result->setParam( ComponentFactory::getInstance()->createComponent( opcode ) );

  InvokeMap::const_iterator it = originating.find( invId );
  if( it != originating.end() )
  {
  		Invoke* inv = (*it).second;
  		//inv->notify1( result );
  }

  delete result;

  return MSG_OK;
}

USHORT_T TcapDialog::handleResultNotLast(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
  return MSG_OK;
}

USHORT_T TcapDialog::handleUserError(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
  return MSG_OK;
}

} // namespace inap
} // namespace inman
} // namespace smsc

