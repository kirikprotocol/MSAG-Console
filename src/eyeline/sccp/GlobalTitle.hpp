/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef __EYELINE_SCCP_GLOBAL_TITLE_HPP
#ident "@(#)$Id$"
#define __EYELINE_SCCP_GLOBAL_TITLE_HPP

#include <inttypes.h>
#include "eyeline/util/TBCDString.hpp"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace sccp {

using eyeline::util::TBCDString;

//Q.713 clause 3.4.2.3
class GlobalTitle {
public:
  //The maximum number of the GTAI digits is normally determined by the
  //maximum of the E.164 numbering plan.
  static const unsigned _maxAdrSignalsNum = 15;
  typedef smsc::core::buffers::FixedLengthString<_maxAdrSignalsNum + 1> gt_adr_string_t;

  static const unsigned _maxGTEnumChars = 24;
  static const unsigned _maxGTStringLen = 5*(sizeof(".(ddd)") + _maxGTEnumChars)
                                          + _maxAdrSignalsNum + 1;
  typedef smsc::core::buffers::FixedLengthString<_maxGTStringLen> gt_string_t;

  //Maximum number of octet in packed/encoded GlobalTitle address
  static const unsigned _maxOctsLen = (_maxAdrSignalsNum + 1)/2 + 3/*sizeof(GTI_Inter)*/;

  //
  enum IndicatorKind_e { //4 bits value, contains combination of following 4 elemenst:
                     //  NoA - Nature Of Address, TrType - Translation type
                     //  NPi - Numbering Plan indicator, EnSch - encoding scheme
    gtiNone = 0x00, gtiNoA_only = 0x01, gtiTrT_only = 0x02,
    gtiTrT_NPi_Sch = 0x03, gtiInternational = 0x04,
    //Spare international indicators
    gtiSpareInter5 = 0x05, gtiSpareInter6 = 0x06, gtiSpareInter7 = 0x07,
    //Spare national indicators
    gtiSpareNational8 = 0x08, gtiSpareNational9 = 0x09, gtiSpareNational10 = 0x0A, 
    gtiSpareNational11 = 0x0B, gtiSpareNational12 = 0x0C, gtiSpareNational13 = 0x0D,
    gtiSpareNational14 = 0x0E,
    //
    gtiMaxReserved = 0x0F
  };
  static const char * nmIndicatorKind(IndicatorKind_e use_kind);

  enum NatureOfAddress_e { //7 bits value,
    noaUnknown = 0x00,
    noaSubscriber = 0x01,     //only SN (SubscriberNumber)
    noaNationalReserv = 0x02,
    noaNationalSign = 0x03,   //NDC + SN (National Destination Code + SubscriberNumber)
    noaInternational = 0x04,  //CC + NDC + SN (CountryCode + 
                              //          National Destination Code + SubscriberNumber)
    noaSpare05 = 0x05,
    noaSpareRange = 0x6F, //[0x06, 0x6F]
    noaRsrvNationalRange = 0x7E, // [0x70, 0x7E]
    //
    noaMaxReserved = 0x7F
  };
  static NatureOfAddress_e val2NatureOfAddress(uint8_t use_val)
  {
    if ((use_val >= 0x70) && (use_val <= 0x7E))
      return GlobalTitle::noaRsrvNationalRange;
    if ((use_val >= 0x06) && (use_val <= 0x6F))
      return GlobalTitle::noaSpareRange;
    return static_cast<NatureOfAddress_e>(use_val);
  }
  static const char * nmNatureOfAddress(NatureOfAddress_e use_val);


  enum NumberingPlan_e { //4 bits values
    npiUnknown = 0x00, npiISDNTele_e164 = 0x01, npiGeneric = 0x02,
    npiData_x121 = 0x03, npiTelex_f69 = 0x04, npiMaritimeMobile_e210 = 0x05,
    npiLandMobile_e212 = 0x06, npiISDNMobile_e214 = 0x07,
    npiSpare08 = 0x08, npiSpare09 = 0x09, npiSpare10 = 0x0A, 
    npiSpare11 = 0x0B, npiSpare12 = 0x0C, npiSpare13 = 0x0D,
    npiNetworkSpec = 0x0E,
    npiMaxReserved = 0x0F
  };
  static const char * nmNumberingPlan(NumberingPlan_e use_val);

  enum EncodingScheme_e { //4 bits values
    schUnknown = 0x00, schBCDodd = 0x01, schBCDeven = 0x02,
    schNational = 0x03,
    schSpare04 = 0x04, schSpare05 = 0x05, schSpare06 = 0x06,
    schSpare07 = 0x07, schSpare08 = 0x08, schSpare09 = 0x09,
    schSpare10 = 0x0A, schSpare11 = 0x0B, schSpare12 = 0x0C,
    schSpare13 = 0x0D, schSpare14 = 0x0E,
    //
    schMaxReserved = 0x0F
  };
  static const char * nmEncodingScheme(EncodingScheme_e use_val);

  enum TranslationType_e { //values, which address range of 8 bits numbers
    trtUnknown = 0x00,    //range [0, 0]
    trtInternationalSrv,  //range [1, 63]
    trtSpare,             //range [64, 127]
    trtNationalNetwork,   //range [128, 254]
    trtReserved           //range [255, 255]
  };
  static const char * nmTranslationType(TranslationType_e use_val);

  static TranslationType_e val2TransType(uint8_t tr_type_val)
  {
    if (!tr_type_val)
      return trtUnknown;
    if (tr_type_val <= 63)
      return trtInternationalSrv;
    if (tr_type_val <= 127)
      return trtSpare;
    if (tr_type_val <= 254)
      return trtNationalNetwork;
    return trtReserved; //(tr_type_val == 255)
  }

  struct GTI_NoA {
    bool    odd /*: 1*/; //odd or even number of address signals, 1 - odd, 0 - even
    uint8_t val /*: 7*/; //NatureOfAddress_e value

    static const unsigned _octsSZ = 1;

    unsigned pack2Octs(uint8_t * use_buf) const
    {
      *use_buf = (val & 0x7F) + (odd ? 0x80 : 0x00);
      return _octsSZ;
    }
    unsigned unpackOcts(const uint8_t * use_buf, unsigned buf_len)
    {
      if (buf_len) {
        odd = (bool)(*use_buf >> 7);
        val = *use_buf & 0x7F;
        return _octsSZ;
      }
      return 0;
    }

    NatureOfAddress_e getNoA(void) const { return val2NatureOfAddress(val); }
  };

  struct GTI_TrT {
    uint8_t val;      //TranslationType_e value

    static const unsigned _octsSZ = 1;
    //
    unsigned pack2Octs(uint8_t * use_buf) const
    {
      *use_buf = val;
      return _octsSZ;
    }
    unsigned unpackOcts(const uint8_t * use_buf, unsigned buf_len)
    {
      if (buf_len) {
        val = *use_buf;
        return _octsSZ;
      }
      return 0;
    }

    TranslationType_e getTrType(void) const { return val2TransType(val); }
  };

  struct GTI_TNS {
    uint8_t trT;          //TranslationType_e value
    uint8_t npi /*: 4*/;  //NumberingPlan_e value
    uint8_t sch /*: 4*/;  //EncodingScheme_e value

    static const unsigned _octsSZ = 2;
    //
    unsigned pack2Octs(uint8_t * use_buf) const
    {
      *use_buf = trT;
      use_buf[1] = (sch & 0x0F) + (npi << 4);
      return _octsSZ;
    }
    unsigned unpackOcts(const uint8_t * use_buf, unsigned buf_len)
    {
      if (buf_len > 1) {
        trT = *use_buf;
        npi = (use_buf[1]) >> 4;
        sch = (use_buf[1]) & 0x0F;
        return _octsSZ;
      }
      return 0;
    }

    TranslationType_e getTrType(void) const { return val2TransType(trT); }
    //
    NumberingPlan_e getNPi(void) const { return static_cast<NumberingPlan_e>(npi); }
    //
    EncodingScheme_e getScheme(void) const { return static_cast<EncodingScheme_e>(sch); }
  };

  struct GTI_Inter {
    uint8_t trT;          //TranslationType_e value
    uint8_t npi /*: 4*/;  //NumberingPlan_e value
    uint8_t sch /*: 4*/;  //EncodingScheme_e value
    uint8_t noa /*: 7*/;  //NatureOfAddress_e value

    static const unsigned _octsSZ = 3;
    //
    unsigned pack2Octs(uint8_t * use_buf) const
    {
      *use_buf = trT;
      use_buf[1] = (sch & 0x0F) + (npi << 4);
      use_buf[2] = (noa & 0x7F);
      return _octsSZ;
    }
    unsigned unpackOcts(const uint8_t * use_buf, unsigned buf_len)
    {
      if (buf_len > 2) {
        trT = *use_buf;
        npi = (use_buf[1]) >> 4;
        sch = (use_buf[1]) & 0x0F;
        noa = use_buf[2] & 0x7F;
        return _octsSZ;
      }
      return 0;
    }

    TranslationType_e getTrType(void) const { return val2TransType(trT); }
    //
    NumberingPlan_e getNPi(void) const { return static_cast<NumberingPlan_e>(npi); }
    //
    EncodingScheme_e getScheme(void) const { return static_cast<EncodingScheme_e>(sch); }
    //
    NatureOfAddress_e getNoA(void) const { return val2NatureOfAddress(noa); }
  };

  struct GTIndicator {
    IndicatorKind_e   kind;
    union {
      GTI_NoA         NoA;
      GTI_TrT         TrT;
      GTI_TNS         TNS;
      GTI_Inter       All;
    } parm;

    GTIndicator() : kind(GlobalTitle::gtiNone)
    { }

    GTIndicator(bool use_odd, uint8_t use_noa_val)
      : kind(GlobalTitle::gtiNoA_only)
    {
      parm.NoA.odd = use_odd;
      parm.NoA.val = use_noa_val;
    }
    //TranslationType only:
    GTIndicator(uint8_t trans_type_val)
      : kind(GlobalTitle::gtiTrT_only)
    {
      parm.TrT.val = trans_type_val;
    }
    GTIndicator(uint8_t trans_type_val, NumberingPlan_e use_npi,
                EncodingScheme_e use_sch)
      : kind(GlobalTitle::gtiTrT_NPi_Sch)
    {
      parm.TNS.trT = trans_type_val;
      parm.TNS.npi = use_npi;
      parm.TNS.sch = use_sch;
    }
    GTIndicator(uint8_t trans_type_val, NumberingPlan_e use_npi,
                EncodingScheme_e use_sch, uint8_t use_noa_val)
      : kind(GlobalTitle::gtiInternational)
    {
      parm.All.trT = trans_type_val;
      parm.All.npi = use_npi;
      parm.All.sch = use_sch;
      parm.All.noa = use_noa_val;
    }
    GTIndicator(uint8_t trans_type_val, NumberingPlan_e use_npi,
                bool bcd_odd_scheme, uint8_t use_noa_val)
      : kind(GlobalTitle::gtiInternational)
    {
      parm.All.trT = trans_type_val;
      parm.All.npi = use_npi;
      parm.All.sch = bcd_odd_scheme ? GlobalTitle::schBCDodd : GlobalTitle::schBCDeven;
      parm.All.noa = use_noa_val;
    }

    NatureOfAddress_e getNoA(void) const
    {
      switch (kind) {
      case GlobalTitle::gtiNoA_only:  return parm.NoA.getNoA();
      case GlobalTitle::gtiInternational:  return parm.All.getNoA();
//      case GlobalTitle::gtiTrT_only:
//      case GlobalTitle::gtiTrT_NPi_Sch:  
      default:;
      }
      return GlobalTitle::noaUnknown;
    }

    uint8_t getNoAValue(void) const
    {
      switch (kind) {
      case GlobalTitle::gtiNoA_only:  return parm.NoA.val;
      case GlobalTitle::gtiInternational:  return parm.All.noa;
//      case GlobalTitle::gtiTrT_only:
//      case GlobalTitle::gtiTrT_NPi_Sch:  
      default:;
      }
      return 0;
    }


    NumberingPlan_e getNPi(void) const
    {
      switch (kind) {
      case GlobalTitle::gtiTrT_NPi_Sch:  return parm.TNS.getNPi();
      case GlobalTitle::gtiInternational:  return parm.All.getNPi();
//      case GlobalTitle::gtiTrT_only:
//      case GlobalTitle::gtiNoA_only:  
      default:;
      }
      return GlobalTitle::npiUnknown;
    }

    TranslationType_e getTrType(void) const
    {
      switch (kind) {
      case GlobalTitle::gtiTrT_NPi_Sch: return parm.TNS.getTrType();
//      case GlobalTitle::gtiInternational:
      case GlobalTitle::gtiTrT_only:    return parm.TrT.getTrType();
//      case GlobalTitle::gtiNoA_only:  
      default:;
      }
      return GlobalTitle::trtUnknown;
    }
    uint8_t getTrTypeValue(void) const
    {
      switch (kind) {
      case GlobalTitle::gtiTrT_NPi_Sch: return parm.TNS.trT;
//      case GlobalTitle::gtiInternational:
      case GlobalTitle::gtiTrT_only:    return parm.TrT.val;
//      case GlobalTitle::gtiNoA_only:  
      default:;
      }
      return 0;
    }

    static unsigned octsSize(GlobalTitle::IndicatorKind_e use_gti)
    {
      switch (use_gti) {
      case GlobalTitle::gtiNoA_only:  return GTI_NoA::_octsSZ;
      case GlobalTitle::gtiTrT_only:  return GTI_TrT::_octsSZ;
      case GlobalTitle::gtiTrT_NPi_Sch:  return GTI_TNS::_octsSZ;
      case GlobalTitle::gtiInternational:  return GTI_Inter::_octsSZ;
      default:;
      }
      return 0;
    }
    //
    unsigned octsSize(void) const { return octsSize(kind); }

    //Encodes/Packs header to octet buffer
    //NOTE: specified buffer must be able to store at least 3 bytes!!!
    unsigned pack2Octs(uint8_t * use_buf) const
    {
      switch (kind) {
      case GlobalTitle::gtiNoA_only:  return parm.NoA.pack2Octs(use_buf);
      case GlobalTitle::gtiTrT_only:  return parm.TrT.pack2Octs(use_buf);
      case GlobalTitle::gtiTrT_NPi_Sch:  return parm.TNS.pack2Octs(use_buf);
      case GlobalTitle::gtiInternational:  return parm.All.pack2Octs(use_buf);
      default:;
      }
      return 0;
    }
    //Decodes/Unpacks address from octet buffer
    unsigned unpackOcts(IndicatorKind_e use_gti, const uint8_t * use_buf, unsigned buf_len)
    {
      switch (kind = use_gti) {
      case GlobalTitle::gtiNoA_only:  return parm.NoA.unpackOcts(use_buf, buf_len);
      case GlobalTitle::gtiTrT_only:  return parm.TrT.unpackOcts(use_buf, buf_len);
      case GlobalTitle::gtiTrT_NPi_Sch:  return parm.TNS.unpackOcts(use_buf, buf_len);
      case GlobalTitle::gtiInternational:  return parm.All.unpackOcts(use_buf, buf_len);
      default:
        kind = GlobalTitle::gtiNone;
      }
      return 0;
    }
  };

protected:
  gt_adr_string_t _signals;
  GTIndicator     _hdr;

public:
  GlobalTitle()
  { }

  //GTI = gtiNoA_only, it's assumed translation type = Unknown
  GlobalTitle(uint8_t use_noa_val, const char * use_signals)
    : _signals(use_signals), _hdr(bool(_signals.length()%2), use_noa_val)
  { }

  //GTI = gtiTrT_only, it's assumed that Numbering Plan = E.164
  GlobalTitle(TranslationType_e use_tr_type, uint8_t use_trType_val,
              const char * use_signals)
    : _signals(use_signals), _hdr(use_trType_val)
 { }

  //GTI = gtiTrT_NPi_Sch
  GlobalTitle(uint8_t use_trType_val, NumberingPlan_e use_npi,
              EncodingScheme_e use_sch, const char * use_signals)
    : _signals(use_signals), _hdr(use_trType_val, use_npi, use_sch)
  { }

  //GTI = gtiInternational
  //NOTE: in international use this GTI implies only odd or even encoding scheme!
  GlobalTitle(uint8_t use_trType_val, NumberingPlan_e use_npi,
              uint8_t use_noa_val, const char * use_signals)
    : _signals(use_signals), _hdr(use_trType_val, use_npi, bool(_signals.length()%2), use_noa_val)
  { }

  //GTI = gtiInternational
  GlobalTitle(uint8_t use_trType_val, NumberingPlan_e use_npi,
              uint8_t use_noa_val, EncodingScheme_e use_sch,
              const char * use_signals)
    : _signals(use_signals), _hdr(use_trType_val, use_npi, use_sch, use_noa_val)
  { }

  ~GlobalTitle()
  { }

  //
  void reset(void)
  {
    _signals.clear();
    _hdr.kind = GlobalTitle::gtiNone;
  }
  //GTI = gtiNoA_only, it's assumed translation type = Unknown
  void constructNoA(uint8_t use_noa_val, const char * use_signals)
  {
    _signals = use_signals;
    _hdr.kind = GlobalTitle::gtiNoA_only;
    _hdr.parm.NoA.val = use_noa_val;
    _hdr.parm.NoA.odd = bool(_signals.length()%2);
  }

  //GTI = gtiTrT_only, it's assumed that Numbering Plan = E.164
  void constructTrT(uint8_t use_trType_val, const char * use_signals)
  {
    _signals = use_signals;
    _hdr.kind = GlobalTitle::gtiTrT_only;
    _hdr.parm.TrT.val = use_trType_val;
  }

  //GTI = gtiTrT_NPi_Sch
  void constructTNS(uint8_t use_trType_val, NumberingPlan_e use_npi,
              EncodingScheme_e use_sch, const char * use_signals)
  {
    _signals = use_signals;
    _hdr.kind = GlobalTitle::gtiTrT_NPi_Sch;
    _hdr.parm.TNS.trT = use_trType_val;
    _hdr.parm.TNS.npi = use_npi;
    _hdr.parm.TNS.sch = use_sch;
  }

  //GTI = gtiInternational
  //NOTE: in international use this GTI implies only BCD odd or BCD even encoding scheme!
  void construct(uint8_t use_trType_val, NumberingPlan_e use_npi,
                 uint8_t use_noa_val, const char * use_signals)
  {
    _signals = use_signals;
    _hdr.kind = GlobalTitle::gtiInternational;
    _hdr.parm.All.trT = use_trType_val;
    _hdr.parm.All.npi = use_npi;
    _hdr.parm.All.sch = (_signals.length()%2) ? GlobalTitle::schBCDodd
                                                     : GlobalTitle::schBCDeven;
    _hdr.parm.All.noa = use_noa_val;
  }

  //GTI = gtiInternational
  void construct(uint8_t use_trType_val, NumberingPlan_e use_npi,
                 uint8_t use_noa_val, EncodingScheme_e use_sch,
                 const char * use_signals)
  {
    _signals = use_signals;
    _hdr.kind = GlobalTitle::gtiInternational;
    _hdr.parm.All.trT = use_trType_val;
    _hdr.parm.All.npi = use_npi;
    _hdr.parm.All.sch = use_sch;
    _hdr.parm.All.noa = use_noa_val;
  }

  //
  IndicatorKind_e getGTIKind(void) const { return _hdr.kind; }
  //
  const GTIndicator & getGTIndicator(void) const { return _hdr; }
  //
  const char * getSignals(void) const { return _signals.c_str(); }

  //Encodes/Packs address to octet buffer
  //Returns number of characters packed, 0 - in case of failure.
  //NOTE: specified buffer must be able to store _maxOctsLen bytes!!!
  unsigned pack2Octs(uint8_t * use_buf) const
  {
    unsigned n = _hdr.pack2Octs(use_buf);
    if (n)
      n += TBCDString::pack2Octs(use_buf + n, _signals.c_str(), (unsigned)_signals.length());
    return n;
  }
  //Decodes/Unpacks address from octet buffer
  //Returns number of characters unpacked, 0 - in case of failure.
  unsigned unpackOcts(IndicatorKind_e use_gti, const uint8_t * use_buf, unsigned buf_len)
  {
    unsigned n = _hdr.unpackOcts(use_gti, use_buf, buf_len);
    if (n)
      n += TBCDString::unpackOcts(use_buf + n, buf_len - n, _signals.str);
    return n;
  }

  //Composes string representation of GlobalTitle
  //NOTE: length of buffer should be at least _maxGTCharsNum 
  size_t      toString(char * use_buf, size_t max_len) const;

  //Composes string representation of GlobalTitle
  gt_string_t toString(void) const
  {
    gt_string_t gtStr;
    toString(gtStr.str, gtStr.capacity());
    return gtStr;
  }
};

typedef GlobalTitle::gt_string_t gt_string_t;

} //sccp
} //eyeline

#endif /* __EYELINE_SCCP_GLOBAL_TITLE_HPP */

