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
        cntrInfo.ab_type = static_cast<AbonentContractInfo::ContractType>(itmp);
    }
    {
        TonNpiAddressString stmp;
        in >> stmp;
        if (stmp.empty())
            cntrInfo.tdpSCF.clear();
        else {
            TonNpiAddress adr;
            if (!adr.fromText(stmp.c_str()))
                throw SerializerException("invalid gsmSCF address",
                                          SerializerException::invObjData, stmp.c_str());
            cntrInfo.tdpSCF[TDPCategory::dpMO_SM].scfAddress = adr;
        }
    }
    in >> errCode;
    if (cntrInfo.getSCFinfo(TDPCategory::dpMO_SM)) {
        cntrInfo.tdpSCF[TDPCategory::dpMO_SM].serviceKey = errCode;
        errCode = 0;
    }
    in >> cntrInfo.abImsi;
    in >> errMsg;
}

void AbntContractResult::save(ObjectBuffer& out) const
{
    out << nmPolicy;
    out << (uint8_t)cntrInfo.ab_type;

    GsmSCFinfo          smScf;
    const GsmSCFinfo *  p_scf = cntrInfo.getSCFinfo(TDPCategory::dpMO_SM);
    if (!p_scf) { //check if SCF for MO-BC may be used
        if ((p_scf = cntrInfo.getSCFinfo(TDPCategory::dpMO_BC)) != 0)
            smScf.scfAddress = p_scf->scfAddress;
    } else 
        smScf = *p_scf;

    if (smScf.scfAddress.empty())
        out << (uint8_t)0x00;
    else
        out << smScf.scfAddress.toString();

    out << (errCode ? errCode : smScf.serviceKey);
    out << cntrInfo.abImsi;
    out << errMsg;
}

void AbntContractResult::setError(uint32_t err_code, const char * err_msg/* = NULL*/)
{ 
    cntrInfo.ab_type = AbonentContractInfo::abtUnknown;
    cntrInfo.tdpSCF.clear();
    errCode = err_code;
    if (err_msg)
        errMsg = err_msg;
    else
        errMsg.clear();
}

} //interaction
} //inman
} //smsc
