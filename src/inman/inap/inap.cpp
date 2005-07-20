///////////////////////////////////////////////////////////////////////////////////////////
/// Callbacks implementation
///////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>

#include "logger/Logger.h"

#include "callbacks.hpp"
#include "factory.hpp"
#include "session.hpp"
#include "dialog.hpp"
#include "util.hpp"

using smsc::logger::Logger;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::Session;
using smsc::inman::inap::Factory;
using smsc::inman::inap::getReturnCodeDescription;
using smsc::inman::inap::inapLogger;

USHORT_T E94InapBindConf(       UCHAR_T         ssn, 
                                UCHAR_T         bindResult )
{
    Session* pSession = Factory::getInstance()->findSession( ssn );
    assert( pSession );
    if( BIND_RES_SUCCESS == bindResult )
    {
        pSession->setState( Session::BOUNDED );
    }
    else
    {
        smsc_log_error(inapLogger, "INAP_BIND request failed. Error code 0x%X", bindResult);
        pSession->setState( Session::ERROR );
    }
    return INAP_OK;
}                                

USHORT_T E94InapAbortInd(       UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                ABORT_T         *abort_sp )
{
    return INAP_OK;
}                                

USHORT_T E94InapPAbortInd(      UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                UCHAR_T         pAbortCause )
{
    return INAP_OK;
}                                

USHORT_T E94InapBeginInd(       UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                SS7_ADDRESS_T   *destAddress_sp,
                                SS7_ADDRESS_T   *origAddress_sp,
                                AC_NAMEREQ_T    *acName_sp,
                                USHORT_T        uInfoLen,
                                UCHAR_T         *uInfo_p,
                                USHORT_T        noOfComponents,
                                COMP_T          *comp_p )
{
    smsc_log_debug(inapLogger, "E94InapBeginInd(%d,%d,..)", ssn, dialogueId);
    Session* pSession = Factory::getInstance()->findSession( ssn );
    assert( pSession );
//    assert( !pSession->findDialog( dialogueId ) );
    Dialog* pDlg = new Dialog( pSession, dialogueId );
    return INAP_OK;
}                                


USHORT_T E94InapDataInd(        UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                UCHAR_T         acShort,
                                USHORT_T        uInfoLen,
                                UCHAR_T         *uInfo_p,
                                USHORT_T        noOfComponents,
                                COMP_T          *comp_p )
{
    smsc_log_debug(inapLogger, "E94InapDataInd(%d,%d,..)", ssn, dialogueId);
    return INAP_OK;
}                                

USHORT_T E94InapEndInd(         UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                UCHAR_T         acShort,
                                USHORT_T        uInfoLen,
                                UCHAR_T         *uInfo_p,
                                USHORT_T        noOfComponents,
                                COMP_T          *comp_p )
{
    smsc_log_debug(inapLogger, "E94InapEndInd(%d,%d,..)", ssn, dialogueId);
    return INAP_OK;
}                                

USHORT_T E94InapErrorInd(       UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         einReason,
                                BOOLEAN_T       invokeIdUsed,
                                UCHAR_T         invokeId,
                                UCHAR_T         disconnectIndicator )
{
    smsc_log_debug(inapLogger, "E94InapErrorInd(%d,%d,..)", ssn, dialogueId);
    return INAP_OK;
}                                

USHORT_T E94InapNoticeInd(      UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         priority,
                                UCHAR_T         acShort,
                                UCHAR_T         reportCause,
                                UCHAR_T         returnIndicator,
                                USHORT_T        relatedDialogueId,
                                USHORT_T        noOfComponents,
                                COMP_T          *comp_p )
{
    smsc_log_debug(inapLogger, "E94InapNoticeInd(%d,%d,..)", ssn, dialogueId);
    return INAP_OK;
}                                

USHORT_T E94InapStateInd(       UCHAR_T         ssn,
                                UCHAR_T         userStatus,
                                UCHAR_T         affectedSsn,
                                ULONG_T         affectedSpc,
                                ULONG_T         localSpc )
{
    smsc_log_debug(inapLogger, "E94InapStateInd(%d,%d,%d,%d,%d)", 
                                    ssn, userStatus, affectedSsn, affectedSpc, localSpc);
    return INAP_OK;
}                                

USHORT_T E94InapIndError(       USHORT_T        indErrorCode,
                                MSG_T           *msg_sp )
{
    smsc_log_debug(inapLogger, "E94InapIndError(%d,...)", indErrorCode );
    return INAP_OK;
}                                
