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

/////////////////////////////////////////////////////////////////////////////////////
// Dialog class
/////////////////////////////////////////////////////////////////////////////////////


Dialog::Dialog(Session* pSession, USHORT_T dlgId) 
	: context( *this )
	, did( dlgId )
	, session( pSession )
	, qSrvc(EINSS7_I97TCAP_QLT_BOTH)
	, priority(EINSS7_I97TCAP_PRI_HIGH_0)
{
}

Dialog::~Dialog()
{
}

USHORT_T Dialog::start()
{
  assert( session );

  USHORT_T result;
  result = EINSS7_I97TInvokeReq(
    session->getSSN(),
    MSG_USER_ID,
    TCAP_INSTANCE_ID,
    did,
    1, //invokeId
    0, //linkedIdUsed,
    0, //linkedId,
    EINSS7_I97TCAP_OP_CLASS_1, //opClass,
    30, //timeOut,
    EINSS7_I97TCAP_OPERATION_TAG_LOCAL, //operationTag,
    0, //operationLength,
    NULL, //*operationCode_p,
    0, //paramLength,
    NULL); //*parameters_p

  if (result != 0)
  {
      smsc_log_error(inapLogger, "EINSS7_I97TInvokeReq failed with code %d(%s)", result,getTcapReasonDescription(result));
      context.error();
      session->closeDialog(this);
  }

  result = EINSS7_I97TBeginReq(
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

  if (result != 0)
  {
      smsc_log_error(inapLogger, "EINSS7_I97TBeginReq failed with code %d(%s)", result,getTcapReasonDescription(result));
      context.error();
      session->closeDialog(this);
  }
  return result;
}

// Begin dialog

USHORT_T Dialog::beginReq()
{
/*
    assert( session );

    USHORT_T result = E94InapBeginReq(
       session->getSSN(),
       id,
       qSrvc,
       priority,
       &session->scfAddr,
       &session->inmanAddr,
       &session->ac,
       0, NULL,
       0, NULL );

    if (result != 0)
    {
        smsc_log_error(inapLogger, "E94InapBeginReq failed with code %d(%s)", result,getInapReturnCodeDescription(result));
    }

    return result;
*/
    return 0;
}

USHORT_T Dialog::dataReq()
{
/*
    USHORT_T result = E94InapDataReq(
        session->getSSN(),
        id,
        qSrvc,
        priority,
        acShort,
        &session->inmanAddr,
        0, //uInfoLen,
        NULL, //uInfo_p,
        0,    //USHORT_T noOfComponents
        NULL); //COMP_T *comp_p

    if (result != 0)
    {
        smsc_log_error(inapLogger, "E94InapDataReq failed with code %d(%s)", result,getInapReturnCodeDescription(result));
    }

    return result;
*/
    return 0;
}

USHORT_T Dialog::endReq()
{
/*
    USHORT_T result = E94InapEndReq(
        session->getSSN(),
        id,
        qSrvc,
        priority,
        acShort,
        IN_E_BASIC_TERM, //UCHAR_T termination,
        0, //USHORT_T uInfoLen,
        NULL, //UCHAR_T *uInfo_p,
        0, //USHORT_T noOfComponents,
        NULL//COMP_T *comp_p);
    );

    if (result != 0)
    {
        smsc_log_error(inapLogger, "E94InapEndReq failed with code %d(%s)", result,getInapReturnCodeDescription(result));
    }

    return result;
*/
    return 0;
}

USHORT_T Dialog::initialDPSMS()
{
	return 0;
}

USHORT_T Dialog::eventReportSMS()
{
	return 0;
}

USHORT_T Dialog::applicationEnd()
{
	return 0;
}

} // namespace inap
} // namespace inman
} // namespace smsc
