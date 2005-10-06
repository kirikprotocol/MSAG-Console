static char const ident[] = "$Id$";

#include <assert.h>
#include <string>
#include <stdexcept>
#include <memory>

#include "invoke.hpp"
#include "results.hpp"
#include "dialog.hpp"
#include "session.hpp"
#include "factory.hpp"

#include "inman/common/util.hpp"
#include "inman/comp/comfactory.hpp"

using std::runtime_error;
using std::auto_ptr;
using smsc::inman::common::format;
using smsc::inman::common::getTcapReasonDescription;
using smsc::inman::common::dump;
using smsc::inman::comp::ComponentFactory;

namespace smsc  {
namespace inman {
namespace inap  {

/////////////////////////////////////////////////////////////////////////////////////
// Dialog class
/////////////////////////////////////////////////////////////////////////////////////

extern Logger* tcapLogger;

Dialog::Dialog(Session* pSession, USHORT_T dlgId, const APP_CONTEXT_T& ac)
  : logger(Logger::getInstance("smsc.inman.inap.Dialog"))
  , did( dlgId )
  , session( pSession )
  , qSrvc(EINSS7_I97TCAP_QLT_BOTH)
  , priority(EINSS7_I97TCAP_PRI_HIGH_0)
  , timeout( 30 )
  , invokeId( 1 )
  , ac(ac)
{
}

Dialog::~Dialog()
{
}

void Dialog::beginDialog()
{
  smsc_log_debug(tcapLogger,"BEGIN_REQ");
  smsc_log_debug(tcapLogger," SSN: 0x%X", session->getSSN());
  smsc_log_debug(tcapLogger," UserID: 0x%X", MSG_USER_ID );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", TCAP_INSTANCE_ID );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", did );
  smsc_log_debug(tcapLogger," PriOrder: 0x%X", priority );
  smsc_log_debug(tcapLogger," QoS: 0x%X", qSrvc );
  smsc_log_debug(tcapLogger," Dest. address: %s", dump(session->scfAddr.addrLen ,session->scfAddr.addr  ).c_str() );
  smsc_log_debug(tcapLogger," Org. address: %s" , dump(session->ssfAddr.addrLen ,session->ssfAddr.addr  ).c_str() );
  smsc_log_debug(tcapLogger," App. context: %s" , dump(ac.acLen ,ac.ac ).c_str() );

  USHORT_T result = EINSS7_I97TBeginReq(
    session->getSSN(),
    MSG_USER_ID,
    TCAP_INSTANCE_ID,
    did,
    priority,
    qSrvc,
    session->scfAddr.addrLen,
    session->scfAddr.addr,
    session->ssfAddr.addrLen,
    session->ssfAddr.addr,
    ac.acLen,
    ac.ac,
    0,
    NULL);


  if(result != 0)
    throw runtime_error( format("BeginReq failed with code %d (%s)", result,getTcapReasonDescription(result)));
}

void Dialog::continueDialog()
{
  smsc_log_debug(tcapLogger,"CONTINUE_REQ");
  smsc_log_debug(tcapLogger," SSN: 0x%X", session->getSSN());
  smsc_log_debug(tcapLogger," UserID: 0x%X", MSG_USER_ID );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", TCAP_INSTANCE_ID );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", did );
  smsc_log_debug(tcapLogger," PriOrder: 0x%X", priority );
  smsc_log_debug(tcapLogger," QoS: 0x%X", qSrvc );
  smsc_log_debug(tcapLogger," Org. address: %s" , dump(session->ssfAddr.addrLen ,session->ssfAddr.addr  ).c_str() );
  smsc_log_debug(tcapLogger," App. context: %s" , dump(ac.acLen ,ac.ac ).c_str() );

  USHORT_T result = EINSS7_I97TContinueReq(
    session->getSSN(),
    MSG_USER_ID,
    TCAP_INSTANCE_ID,
    did,
    priority,
    qSrvc,
    session->ssfAddr.addrLen,
    session->ssfAddr.addr,
    ac.acLen,
    ac.ac,
    0,
    NULL);

  if(result != 0)
    throw runtime_error( format("ContinueReq failed with code %d (%s)", result,getTcapReasonDescription(result)));
}

void Dialog::endDialog(USHORT_T termination)
{
  smsc_log_debug(tcapLogger,"END_REQ");
  smsc_log_debug(tcapLogger," SSN: 0x%X", session->getSSN());
  smsc_log_debug(tcapLogger," UserID: 0x%X", MSG_USER_ID );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", TCAP_INSTANCE_ID );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", did );
  smsc_log_debug(tcapLogger," PriOrder: 0x%X", priority );
  smsc_log_debug(tcapLogger," QoS: 0x%X", qSrvc );
  smsc_log_debug(tcapLogger," Termination: 0x%X", termination );
  smsc_log_debug(tcapLogger," App. context: %s" , dump(ac.acLen ,ac.ac ).c_str() );

  USHORT_T result = EINSS7_I97TEndReq(
    session->getSSN(),
    MSG_USER_ID,
    TCAP_INSTANCE_ID,
    did,
    priority,
    qSrvc,
    termination,
    ac.acLen,
    ac.ac,
    0,
    NULL);

  if(result != 0)
    throw runtime_error( format("EndReq failed with code %d (%s)", result,getTcapReasonDescription(result)));
}


void Dialog::timerReset()
{
  smsc_log_debug(tcapLogger,"TIME_RESET_REQ");
  smsc_log_debug(tcapLogger," SSN: 0x%X", session->getSSN());
  smsc_log_debug(tcapLogger," UserID: 0x%X", MSG_USER_ID );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", TCAP_INSTANCE_ID );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", did );

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

Invoke* Dialog::invoke(UCHAR_T opcode)
{
  smsc_log_debug(logger, "Invoke (opcode=0x%X)", opcode);
  Invoke* invoke = new Invoke();
  invoke->setId( getNextInvokeId() );
  invoke->setTag( EINSS7_I97TCAP_OPERATION_TAG_LOCAL );
  invoke->setOpcode( opcode );
//  originating.insert( InvokeMap::value_type(  invoke->getId(), invoke ) );
  return invoke;
}

//--------------------------------------- Callbacks ----------------------------------

USHORT_T Dialog::handleBeginDialog()
{
  return MSG_OK;
}

USHORT_T Dialog::handleContinueDialog()
{
  return MSG_OK;
}

USHORT_T Dialog::handleEndDialog()
{
  return MSG_OK;
}

USHORT_T Dialog::handleInvoke(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
  assert( op );
  assert( oplen > 0 );
  assert( tag == EINSS7_I97TCAP_OPERATION_TAG_LOCAL );

  UCHAR_T opcode = op[0];

  auto_ptr<Invoke> invoke( new Invoke() );
  invoke->setId( invId );
  invoke->setTag( tag );
  invoke->setOpcode( opcode );

  Component* comp = ComponentFactory::getInstance()->create( opcode );

  if( comp )
  {
    std::vector<unsigned char> code( pm, pm + pmlen );
    comp->decode( code );
    invoke->setParam( comp );
  }

  for( ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++)
  {
  DialogListener* ptr = *it;
  ptr->onDialogInvoke( invoke.get() );
  }

  return MSG_OK;
}

USHORT_T Dialog::handleResultLast(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
  assert( op );
  assert( oplen > 0 );
  assert( tag == EINSS7_I97TCAP_OPERATION_TAG_LOCAL );

  UCHAR_T opcode = op[0];

  InvokeResultLast* result = new InvokeResultLast();
  result->setId( invId );
  result->setTag( tag );
  result->setOpcode( opcode );
  result->setParam( ComponentFactory::getInstance()->create( opcode ) );

  InvokeMap::const_iterator it = originating.find( invId );
  if( it != originating.end() )
  {
      Invoke* inv = (*it).second;
  }

  delete result;

  return MSG_OK;
}

USHORT_T Dialog::handleResultNotLast(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
  return MSG_OK;
}

USHORT_T Dialog::handleUserError(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
    return MSG_OK;
}

} // namespace inap
} // namespace inman
} // namespace smsc
