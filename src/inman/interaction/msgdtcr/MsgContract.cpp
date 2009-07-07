#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
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
        std::string stmp;
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
    {
        std::string stmp;
        in >> stmp;
        if (stmp.empty())
            cntrInfo.abImsi[0] = 0;
        else {
            if (stmp.length() > MAP_MAX_IMSI_AddressValueLength)
                throw SerializerException("invalid IMSI",
                                          SerializerException::invObjData, stmp.c_str());
            cntrInfo.setImsi(stmp.c_str());
        }
    }
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
    if (cntrInfo.getImsi()) {
        std::string si(cntrInfo.getImsi());
        out << si;
    } else
        out << (uint8_t)0x00;
    out << errMsg;
}


} //interaction
} //inman
} //smsc
