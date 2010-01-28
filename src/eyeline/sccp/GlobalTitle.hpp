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

  //Maximum number of octet in packed/encoded GlobalTitle address
  static const unsigned _maxOctsLen = (_maxAdrSignalsNum + 1)/2 + 3/*sizeof(GTI_Inter)*/;

  //
  enum Indicator_e { //4 bits value, contains combination of following 4 elemenst:
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
//  static const uint16_t _gtiMask = 0x0F;

  enum NatureOfAddress_e { //7 bits value,
    noaUnknown = 0x00,
    noaSubscriber = 0x01,     //only SN (SubscriberNumber)
    noaNationalReserv = 0x02,
    noaNationalSign = 0x03,   //NDC + SN (National Destination Code + SubscriberNumber)
    noaInternational = 0x04,  //CC + NDC + SN (CountryCode + 
                              //          National Destination Code + SubscriberNumber)
    //
    noaSpareMin = 0x05, noaSpareMax = 0x6F,
    noaRsrvNationalMin = 0x70, noaRsrvNationalMax = 0x7E,
    //
    noaMaxReserved = 0x7F
  };
//  static const uint16_t _noaMask = 0x7F;

  enum NumberingPlan_e { //4 bits values
    npiUnknown = 0x00, npiISDNTele_e164 = 0x01, npiGeneric = 0x02,
    npiData_x121 = 0x03, npiTelex_f69 = 0x04, npiMaritimeMobile_e210 = 0x05,
    npiLandMobile_e212 = 0x06, npiISDNMobile_e214 = 0x07,
    npiSpare8 = 0x08, npiSpare9 = 0x09, npiSpare10 = 0x0A, 
    npiSpare11 = 0x0B, npiSpare12 = 0x0C, npiSpare13 = 0x0D,
    npiNetworkSpec = 0x0E,
    npiMaxReserved = 0x0F
  };
//  static const uint16_t _npiMask = 0x0F;

  enum EncodingScheme_e { //4 bits values
    schUnknown = 0x00, schBCDodd = 0x01, schBCDeven = 0x02,
    schNational = 0x03,
    schSpareMin = 0x04, schSpareMax = 0x0E,
    //
    schMaxReserved = 0x0F
  };
//  static const uint16_t _schMask = 0x0F;

  enum TranslationType_e { //values, which address range of 8 bits numbers
    trtUnknown = 0x00,    //range [0, 0]
    trtInternationalSrv,  //range [1, 63]
    trtSpare,             //range [64, 127]
    trtNationalNetwork,   //range [128, 254]
    trtReserved           //range [255, 255]
  };

  static TranslationType_e transType(uint8_t tr_type_val)
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
    uint8_t odd : 1; //odd or even number of address signals, 1 - odd, 0 - even
    uint8_t val : 7; //NatureOfAddress_e value

    static const unsigned _octsSZ = 1;

    unsigned pack2Octs(uint8_t * use_buf) const
    {
      *use_buf = val + (odd ? 0x80 : 0x00);
      return _octsSZ;
    }
    unsigned unpackOcts(const uint8_t * use_buf, unsigned buf_len)
    {
      if (buf_len) {
        odd = *use_buf >> 7;
        val = *use_buf & 0x7F;
        return _octsSZ;
      }
      return 0;
    }

    NatureOfAddress_e NoA(void) const
    {
      return static_cast<NatureOfAddress_e>(val);
    }
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

    TranslationType_e TrType(void) const { return transType(val); }
  };

  struct GTI_TNS {
    uint8_t trT;      //TranslationType_e value
    struct {
      uint8_t npi : 4; //NumberingPlan_e value
      uint8_t sch : 4; //EncodingScheme_e value
    } npiSch;

    static const unsigned _octsSZ = 2;
    //
    unsigned pack2Octs(uint8_t * use_buf) const
    {
      *use_buf = trT;
      use_buf[1] = npiSch.sch + (npiSch.npi << 4);
      return _octsSZ;
    }
    unsigned unpackOcts(const uint8_t * use_buf, unsigned buf_len)
    {
      if (buf_len > 1) {
        trT = *use_buf;
        npiSch.npi = (use_buf[1]) >> 4;
        npiSch.sch = (use_buf[1]) & 0x0F;
        return _octsSZ;
      }
      return 0;
    }

    TranslationType_e TrType(void) const { return transType(trT); }
    //
    NumberingPlan_e NPi(void) const
    {
      return static_cast<NumberingPlan_e>(npiSch.npi);
    }
    //
    EncodingScheme_e Scheme(void) const
    {
      return static_cast<EncodingScheme_e>(npiSch.sch);
    }
  };

  struct GTI_Inter {
    uint8_t trT;      //TranslationType_e value
    struct {
      uint8_t npi : 4; //NumberingPlan_e value
      uint8_t sch : 4; //EncodingScheme_e value
    } npiSch;
    struct  {
      uint8_t reserved : 1; // = 0
      uint8_t val : 7; //NatureOfAddress_e value
    } noa;

    static const unsigned _octsSZ = 3;
    //
    unsigned pack2Octs(uint8_t * use_buf) const
    {
      *use_buf = trT;
      use_buf[1] = npiSch.sch + (npiSch.npi << 4);
      use_buf[2] = noa.val;
      return _octsSZ;
    }
    unsigned unpackOcts(const uint8_t * use_buf, unsigned buf_len)
    {
      if (buf_len > 2) {
        trT = *use_buf;
        npiSch.npi = (use_buf[1]) >> 4;
        npiSch.sch = (use_buf[1]) & 0x0F;
        noa.val = use_buf[2] & 0x7F;
        return _octsSZ;
      }
      return 0;
    }

    TranslationType_e TrType(void) const { return transType(trT); }
    //
    NumberingPlan_e NPi(void) const
    {
      return static_cast<NumberingPlan_e>(npiSch.npi);
    }
    //
    EncodingScheme_e Scheme(void) const
    {
      return static_cast<EncodingScheme_e>(npiSch.sch);
    }
    //
    NatureOfAddress_e NoA(void) const
    {
      return static_cast<NatureOfAddress_e>(noa.val);
    }
  };

  struct GTIndicator {
    Indicator_e       kind;
    union {
      GTI_NoA         NoA;
      GTI_TrT         TrT;
      GTI_TNS TNS;
      GTI_Inter       All;
    } parm;

    GTIndicator() : kind(GlobalTitle::gtiNone)
    { }
    GTIndicator(NatureOfAddress_e use_noa, bool use_odd = false)
      : kind(GlobalTitle::gtiNoA_only)
    {
      parm.NoA.odd = use_odd;
      parm.NoA.val = use_noa;
    }
    GTIndicator(TranslationType_e use_tr_type)
      : kind(GlobalTitle::gtiTrT_only)
    {
      parm.TrT.val = use_tr_type;
    }
    GTIndicator(TranslationType_e use_tr_type,
             NumberingPlan_e use_npi, EncodingScheme_e use_sch)
      : kind(GlobalTitle::gtiTrT_NPi_Sch)
    {
      parm.TNS.trT = use_tr_type;
      parm.TNS.npiSch.npi = use_npi;
      parm.TNS.npiSch.sch = use_sch;
    }
    GTIndicator(TranslationType_e use_tr_type, NumberingPlan_e use_npi,
                EncodingScheme_e use_sch, NatureOfAddress_e use_noa)
      : kind(GlobalTitle::gtiInternational)
    {
      parm.All.trT = use_tr_type;
      parm.All.npiSch.npi = use_npi;
      parm.All.npiSch.sch = use_sch;
      parm.All.noa.reserved = 0;
      parm.All.noa.val = use_noa;
    }
    GTIndicator(TranslationType_e use_tr_type, NumberingPlan_e use_npi,
                bool bcd_odd_scheme, NatureOfAddress_e use_noa)
      : kind(GlobalTitle::gtiInternational)
    {
      parm.All.trT = use_tr_type;
      parm.All.npiSch.npi = use_npi;
      parm.All.npiSch.sch = bcd_odd_scheme ? GlobalTitle::schBCDodd : GlobalTitle::schBCDeven;
      parm.All.noa.reserved = 0;
      parm.All.noa.val = use_noa;
    }

    NatureOfAddress_e getNoA(void) const
    {
      switch (kind) {
      case GlobalTitle::gtiNoA_only:  return parm.NoA.NoA();
      case GlobalTitle::gtiInternational:  return parm.All.NoA();
//      case GlobalTitle::gtiTrT_only:
//      case GlobalTitle::gtiTrT_NPi_Sch:  
      default:;
      }
      return GlobalTitle::noaUnknown;
    }

    NumberingPlan_e getNPi(void) const
    {
      switch (kind) {
      case GlobalTitle::gtiTrT_NPi_Sch:  return parm.TNS.NPi();
      case GlobalTitle::gtiInternational:  return parm.All.NPi();
//      case GlobalTitle::gtiTrT_only:
//      case GlobalTitle::gtiNoA_only:  
      default:;
      }
      return GlobalTitle::npiUnknown;
    }

    static unsigned octsSize(GlobalTitle::Indicator_e use_gti)
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
    unsigned octsSize(void) const
    {
      return octsSize(kind);
    }

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
    unsigned unpackOcts(Indicator_e use_gti, const uint8_t * use_buf, unsigned buf_len)
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
  std::string _signals;
  GTIndicator _hdr;

public:
  GlobalTitle()
  { }

  //GTI = gtiNoA_only, it's assumed translation type = Unknown
  GlobalTitle(NatureOfAddress_e use_noa, const char * use_signals)
    : _signals(use_signals), _hdr(use_noa, bool(_signals.length()%2))
  { }

  //GTI = gtiTrT_only, it's assumed that Numbering Plan = E.164
  GlobalTitle(TranslationType_e use_tr_type, const char * use_signals)
    : _signals(use_signals), _hdr(use_tr_type)
  { }

  //GTI = gtiTrT_NPi_Sch
  GlobalTitle(TranslationType_e use_tr_type, NumberingPlan_e use_npi,
              EncodingScheme_e use_sch, const char * use_signals)
    : _signals(use_signals), _hdr(use_tr_type, use_npi, use_sch)
  { }

  //GTI = gtiInternational
  //NOTE: in international use this GTI implies only odd or even encoding scheme!
  GlobalTitle(TranslationType_e use_tr_type, NumberingPlan_e use_npi,
              NatureOfAddress_e use_noa, const char * use_signals)
    : _signals(use_signals), _hdr(use_tr_type, use_npi, bool(_signals.length()%2), use_noa)
  { }

  //GTI = gtiInternational
  GlobalTitle(TranslationType_e use_tr_type, NumberingPlan_e use_npi,
              NatureOfAddress_e use_noa, EncodingScheme_e use_sch,
              const char * use_signals)
    : _signals(use_signals), _hdr(use_tr_type, use_npi, use_sch, use_noa)
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
  void construct(NatureOfAddress_e use_noa, const char * use_signals)
  {
    _signals = use_signals;
    _hdr.kind = GlobalTitle::gtiNoA_only;
    _hdr.parm.NoA.val = use_noa;
    _hdr.parm.NoA.odd = uint8_t(_signals.length()%2);
  }

  //GTI = gtiTrT_only, it's assumed that Numbering Plan = E.164
  void construct(TranslationType_e use_tr_type, const char * use_signals)
  {
    _signals = use_signals;
    _hdr.kind = GlobalTitle::gtiTrT_only;
    _hdr.parm.TrT.val = use_tr_type;
  }

  //GTI = gtiTrT_NPi_Sch
  void construct(TranslationType_e use_tr_type, NumberingPlan_e use_npi,
              EncodingScheme_e use_sch, const char * use_signals)
  {
    _signals = use_signals;
    _hdr.kind = GlobalTitle::gtiTrT_NPi_Sch;
    _hdr.parm.TNS.trT = use_tr_type;
    _hdr.parm.TNS.npiSch.npi = use_npi;
    _hdr.parm.TNS.npiSch.sch = use_sch;
  }

  //GTI = gtiInternational
  //NOTE: in international use this GTI implies only BCD odd or BCD even encoding scheme!
  void construct(TranslationType_e use_tr_type, NumberingPlan_e use_npi,
                 NatureOfAddress_e use_noa, const char * use_signals)
  {
    _signals = use_signals;
    _hdr.kind = GlobalTitle::gtiInternational;
    _hdr.parm.All.trT = use_tr_type;
    _hdr.parm.All.npiSch.npi = use_npi;
    _hdr.parm.All.npiSch.sch = (_signals.length()%2) ? GlobalTitle::schBCDodd
                                                     : GlobalTitle::schBCDeven;
    _hdr.parm.All.noa.val = use_noa;
  }

  //GTI = gtiInternational
  void construct(TranslationType_e use_tr_type, NumberingPlan_e use_npi,
                 NatureOfAddress_e use_noa, EncodingScheme_e use_sch,
                 const char * use_signals)
  {
    _signals = use_signals;
    _hdr.kind = GlobalTitle::gtiInternational;
    _hdr.parm.All.trT = use_tr_type;
    _hdr.parm.All.npiSch.npi = use_npi;
    _hdr.parm.All.npiSch.sch = use_sch;
    _hdr.parm.All.noa.val = use_noa;
  }

  //
  Indicator_e GTi(void) const { return _hdr.kind; }
  //
  const GTIndicator & Indicator(void) const { return _hdr; }
  //
  const std::string & Signals(void) const { return _signals; }

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
  unsigned unpackOcts(Indicator_e use_gti, const uint8_t * use_buf, unsigned buf_len)
  {
    unsigned n = _hdr.unpackOcts(use_gti, use_buf, buf_len);
    if (n)
      n += TBCDString::unpackOcts(use_buf + n, buf_len - n, _signals);
    return n;
  }

  // TODO:: implement toString()
  typedef smsc::core::buffers::FixedLengthString<_maxOctsLen*2> gt_string_t;
  gt_string_t toString() const;
};

typedef GlobalTitle::gt_string_t gt_string_t;

} //sccp
} //eyeline

#endif /* __EYELINE_SCCP_GLOBAL_TITLE_HPP */

