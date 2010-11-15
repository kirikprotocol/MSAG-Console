#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */
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
  : INPCommandSetAC(INProtocol::csAbntContract)
{
  _pckFct.registerProduct(mkPckIdx(ABNT_CONTRACT_REQUEST_TAG, HDR_DIALOG),
                         new PckFactory::ProducerT< SPckContractRequest >());
  _pckFct.registerProduct(mkPckIdx(ABNT_CONTRACT_RESULT_TAG, HDR_DIALOG),
                         new PckFactory::ProducerT< SPckContractResult >());
}

INPCSAbntContract * INPCSAbntContract::getInstance(void)
{
  static INPCSAbntContract  cmdSet;
  return &cmdSet;
}

/* ************************************************************************** *
 * AbntContractResult implementation:
 * ************************************************************************** */
void AbntContractResult::load(ObjectBuffer& in) throw(SerializerException)
{
    in >> nmPolicy;
    {
        uint8_t itmp;
        in >> itmp;
        cntrType = (itmp > AbonentContractInfo::abtPrepaid)
                  ? AbonentContractInfo::abtUnknown 
                  : static_cast<AbonentContract_e>(itmp);
    }
    gsmSCF.Reset();
    {
        TonNpiAddressString stmp;
        in >> stmp;
        if (!stmp.empty() && !gsmSCF.scfAddress.fromText(stmp.c_str()))
            throw SerializerException("invalid gsmSCF address",
                                      SerializerException::invObjData, stmp.c_str());
    }
    in >> errCode;
    if (!gsmSCF.scfAddress.empty()) {
        gsmSCF.serviceKey = errCode;
        errCode = 0;
    }
    in >> abImsi;
    in >> errMsg;
}

void AbntContractResult::save(ObjectBuffer& out) const
{
    out << nmPolicy;
    out << (uint8_t)cntrType;

    if (gsmSCF.scfAddress.empty())
        out << (uint8_t)0x00;
    else
        out << gsmSCF.scfAddress.toString();

    out << (errCode ? errCode : gsmSCF.serviceKey);
    out << abImsi;
    out << errMsg;
}

void AbntContractResult::setError(uint32_t err_code, const char * err_msg/* = NULL*/)
{ 
    cntrType = AbonentContractInfo::abtUnknown;
    errCode = err_code;
    if (err_msg)
        errMsg = err_msg;
    else
        errMsg.clear();
}

} //interaction
} //inman
} //smsc
