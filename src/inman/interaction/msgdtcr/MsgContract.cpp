#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/msgdtcr/MsgContract.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/* ************************************************************************** *
 * AbntContractResult implementation:
 * ************************************************************************** */
void AbntContractResult::load(PacketBufferAC & in_buf) throw(SerializerException)
{
  in_buf >> nmPolicy;
  {
    uint8_t itmp;
    in_buf >> itmp;
    cntrType = (itmp > AbonentContractInfo::abtPrepaid) ?
                AbonentContractInfo::abtUnknown : static_cast<AbonentContract_e>(itmp);
  }
  gsmSCF.Reset();
  {
    TonNpiAddressString stmp;
    in_buf >> stmp;
    if (!stmp.empty() && !gsmSCF.scfAddress.fromText(stmp.c_str()))
      throw SerializerException("invalid gsmSCF address",
                                SerializerException::invObjData, stmp.c_str());
  }
  in_buf >> errCode;
  if (!gsmSCF.scfAddress.empty()) {
    gsmSCF.serviceKey = errCode;
    errCode = 0;
  }
  in_buf >> abImsi;
  in_buf >> errMsg;
}

void AbntContractResult::save(PacketBufferAC & out_buf) const  throw(SerializerException)
{
  out_buf << nmPolicy;
  out_buf << (uint8_t)cntrType;

  if (gsmSCF.scfAddress.empty())
    out_buf << (uint8_t)0x00;
  else
    out_buf << gsmSCF.scfAddress.toString();

  out_buf << (errCode ? errCode : gsmSCF.serviceKey);
  out_buf << abImsi;
  out_buf << errMsg;
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

void AbntContractResult::setContractInfo(AbonentContract_e cntr_type, const char * ab_imsi/* = NULL*/)
{
  cntrType = cntr_type;
  if(ab_imsi)
    abImsi = ab_imsi;
}

} //interaction
} //inman
} //smsc
