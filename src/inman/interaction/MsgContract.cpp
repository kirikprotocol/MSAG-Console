#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/interaction/MsgContract.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/* ************************************************************************** *
 * Abonent Contract detection CommandSet:
 * ************************************************************************** */
INPCSAbntContract::INPCSAbntContract()
{
    pckFct.registerProduct(mkPckIdx(ABNT_CONTRACT_REQUEST_TAG, HDR_DIALOG),
                           new PckFactory::ProducerT< SPckContractRequest >());
    pckFct.registerProduct(mkPckIdx(ABNT_CONTRACT_RESULT_TAG, HDR_DIALOG),
                           new PckFactory::ProducerT< SPckContractResult >());
}

INPCSAbntContract * INPCSAbntContract::getInstance(void)
{
    static INPCSAbntContract  cmdSet;
    return &cmdSet;
}

} //interaction
} //inman
} //smsc
