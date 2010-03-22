#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/sccp/GlobalTitle.hpp"

namespace eyeline {
namespace sccp {

//NOTE: all string representaitons of enumerated values must not be longer than
//    _maxGTEnumChars
 
const char * GlobalTitle::nmIndicatorKind(IndicatorKind_e use_kind)
{
  if ((use_kind >= gtiSpareNational8) && (use_kind <= gtiSpareNational14))
    return "gtiSpareNational";

  switch (use_kind) {
  case GlobalTitle::gtiNoA_only:      return "gtiNoA_only";
  case GlobalTitle::gtiTrT_only:      return "gtiTrT_only";
  case GlobalTitle::gtiTrT_NPi_Sch:   return "gtiTrT_NPi_Sch";
  case GlobalTitle::gtiInternational: return "gtiInternational";
  case GlobalTitle::gtiSpareInter5:
  case GlobalTitle::gtiSpareInter6:
  case GlobalTitle::gtiSpareInter7:   return "gtiSpareInter";
  case GlobalTitle::gtiMaxReserved:   return "gtiMaxReserved";
  default:; //case gtiNone
  }
  return "gtiNone";
}

const char * GlobalTitle::nmNatureOfAddress(NatureOfAddress_e use_val)
{
  switch (use_val) {
  case GlobalTitle::noaSubscriber:      return "noaSubscriber";
  case GlobalTitle::noaNationalReserv:  return "noaNationalReserv";
  case GlobalTitle::noaNationalSign:    return "noaNationalSign";
  case GlobalTitle::noaInternational:   return "noaInternational";
  case GlobalTitle::noaSpare05:         return "noaSpare05";
  case GlobalTitle::noaSpareRange:      return "noaSpareRange";
  case GlobalTitle::noaMaxReserved:     return "noaMaxReserved";
  case GlobalTitle::noaRsrvNationalRange:  return "noaRsrvNationalRange";
  default:;
  }
  return "noaUnknown";
}

const char * GlobalTitle::nmNumberingPlan(NumberingPlan_e use_val)
{
  if ((use_val >= GlobalTitle::npiSpare08) && (use_val <= GlobalTitle::npiSpare13))
    return "npiSpare";
  
  switch (use_val) {
  case GlobalTitle::npiISDNTele_e164:   return "npiISDNTele_e164";
  case GlobalTitle::npiGeneric:         return "npiGeneric";
  case GlobalTitle::npiData_x121:       return "npiData_x121";
  case GlobalTitle::npiTelex_f69:       return "npiTelex_f69";
  case GlobalTitle::npiMaritimeMobile_e210:  return "npiMaritimeMobile_e210";
  case GlobalTitle::npiLandMobile_e212: return "npiLandMobile_e212";
  case GlobalTitle::npiISDNMobile_e214: return "npiISDNMobile_e214";
  case GlobalTitle::npiNetworkSpec:     return "npiNetworkSpec";
  case GlobalTitle::npiMaxReserved:     return "npiMaxReserved";
  //
  default:;
  }
  return "npiUnknown";
}

const char * GlobalTitle::nmEncodingScheme(EncodingScheme_e use_val)
{
  if ((use_val >= GlobalTitle::schSpare04) && (use_val <= GlobalTitle::schSpare14))
    return "schSpare";
  switch (use_val) {
  case GlobalTitle::schBCDodd:      return "schBCDodd";
  case GlobalTitle::schBCDeven:     return "schBCDeven";
  case GlobalTitle::schNational:    return "schNational";
  case GlobalTitle::schMaxReserved: return "schMaxReserved";
  default:;
  }
  return "schUnknown";
}

const char * GlobalTitle::nmTranslationType(TranslationType_e use_val)
{
  switch (use_val) {
  case trtInternationalSrv: return "trtInternationalSrv";
  case trtNationalNetwork:  return "trtNationalNetwork";
  case trtReserved:         return "trtReserved";
  case trtSpare:            return "trtSpare";
  }
  return "trtUnknown";
}

#define CHECK_RES(pr_res, pr_cnt, buf_len) \
  if ((pr_res > (buf_len - pr_cnt)) || (pr_res < 1)) \
    return pr_cnt; pr_cnt += pr_res;

size_t GlobalTitle::toString(char * use_buf, size_t max_len) const
{
  size_t rval = 0;

  int n = snprintf(use_buf, max_len, ".%s(%u)",
           GlobalTitle::nmIndicatorKind(_hdr.kind), _hdr.kind);
  CHECK_RES(n, rval, max_len);

  switch (_hdr.kind) {
  case gtiNoA_only: {
    n = snprintf(use_buf + rval, max_len - rval, ".%s(%u)",
                  GlobalTitle::nmNatureOfAddress(_hdr.parm.NoA.getNoA()), _hdr.parm.NoA.val);
    CHECK_RES(n, rval, max_len);
  } break;

  case gtiTrT_only: {
    n = snprintf(use_buf + rval, max_len - rval, ".%s(%u)",
                  GlobalTitle::nmTranslationType(_hdr.parm.TrT.getTrType()), _hdr.parm.TrT.val);
    CHECK_RES(n, rval, max_len);
  } break;

  case gtiTrT_NPi_Sch: {
    n = snprintf(use_buf + rval, max_len - rval, ".%s(%u).%s(%u).%s(%u)",
                  GlobalTitle::nmTranslationType(_hdr.parm.TrT.getTrType()), _hdr.parm.TNS.trT,
                  GlobalTitle::nmNumberingPlan(_hdr.parm.TNS.getNPi()), _hdr.parm.TNS.npi,
                  GlobalTitle::nmEncodingScheme(_hdr.parm.TNS.getScheme()), _hdr.parm.TNS.sch);
    CHECK_RES(n, rval, max_len);
  } break; 

  case gtiInternational: {
    n = snprintf(use_buf + rval, max_len - rval, ".%s(%u).%s(%u).%s(%u).%s(%u)",
                  GlobalTitle::nmTranslationType(_hdr.parm.All.getTrType()), _hdr.parm.All.trT,
                  GlobalTitle::nmNumberingPlan(_hdr.parm.All.getNPi()), _hdr.parm.All.npi,
                  GlobalTitle::nmEncodingScheme(_hdr.parm.All.getScheme()), _hdr.parm.All.sch,
                  GlobalTitle::nmNatureOfAddress(_hdr.parm.All.getNoA()), _hdr.parm.All.noa);
    CHECK_RES(n, rval, max_len);
  } break;
  default:;
  } //eosw

  if (_signals.length()) {
    n = snprintf(use_buf + rval, max_len - rval, ".%s", _signals.c_str());
    CHECK_RES(n, rval, max_len);
  }

  return rval;
}

} //sccp
} //eyeline

