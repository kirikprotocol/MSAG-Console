// $Id$
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


Dialog::Dialog(Session* pSession, USHORT_T dlgId) : id( dlgId ), session( pSession )
{
}

Dialog::~Dialog()
{
}

// Begin dialog

USHORT_T Dialog::beginReq()
{
    assert( session );

    USHORT_T result = E94InapBeginReq(
       session->getSSN(),
       id, 
       qSrvc, 
       priority,
       &destAddress,
       &origAddress,
       &name,
       0, NULL,
       0, NULL );

    if (result != 0) 
    {
        smsc_log_error(inapLogger, "E94InapBeginReq failed with code %d(%s)", result,getReturnCodeDescription(result));
    }

    return result;
}

USHORT_T Dialog::dataReq()
{
    USHORT_T result = E94InapDataReq(
        session->getSSN(),
        id,
        qSrvc,
        priority,
        acShort,
        &origAddress,
        0, //uInfoLen,
        NULL, //uInfo_p,
        0,    //USHORT_T noOfComponents
        NULL); //COMP_T *comp_p

    if (result != 0) 
    {
        smsc_log_error(inapLogger, "E94InapDataReq failed with code %d(%s)", result,getReturnCodeDescription(result));
    }

    return result;
}

USHORT_T Dialog::endReq()
{
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
        smsc_log_error(inapLogger, "E94InapEndReq failed with code %d(%s)", result,getReturnCodeDescription(result));
    }

    return result;
}

USHORT_T Dialog::invoke(const Operation* op)
{
/*    UCHAR_T opcode = IN_OP_INITIAL_DP;
    COMP_T comp;
    comp.compType = INVOKE;
    comp.invoke.invokeId = opcode;
    comp.invoke.linkedIdUsed = FALSE;
    comp.op.operation = opcode;
    comp.op.initialDPArg;
 */
    return 0;
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
