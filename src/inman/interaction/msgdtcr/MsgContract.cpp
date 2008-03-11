#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */
/* ************************************************************************* *
 * INMan Abonent Contract and gsmSCFs parameters determination protocol
 * PDUs definition.
 * ************************************************************************* */

#include "inman/interaction/msgdtcr/MsgContract.hpp"

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
//    INPSerializer::getInstance()->registerCmdSet(csAbntContract, this);
}

INPCSAbntContract * INPCSAbntContract::getInstance(void)
{
    static INPCSAbntContract  cmdSet;
    return &cmdSet;
}

} //interaction
} //inman
} //smsc
