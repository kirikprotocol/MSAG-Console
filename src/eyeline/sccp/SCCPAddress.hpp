/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef __EYELINE_SCCP_ADDRESS_HPP
#ident "@(#)$Id$"
#define __EYELINE_SCCP_ADDRESS_HPP

#include "eyeline/sccp/GlobalTitle.hpp"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace sccp {

//According to Q.713 clause 3.4.2
//SCCPAddress contains following elements:
// 1) - Address elements Indicators, 1 byte, mandatory
// 2) - Signalling Point Code, 2 bytes, optional
// 3) - Subsystem number, 1 byte, optional
// 4) - GlobalTitle, 2-N bytes, mandatory
class SCCPAddress {
public:
  //Maximum number of octet in packed/encoded GlobalTitle address
  static const unsigned _maxOctsLen = GlobalTitle::_maxOctsLen + 4;
  //
  static const unsigned _maxStringLen = GlobalTitle::_maxGTStringLen + sizeof(".route(SSN).ssn(uuu).spc(uuuuu)");
  typedef smsc::core::buffers::FixedLengthString<_maxStringLen> sccp_addr_string_t;

  typedef GlobalTitle::IndicatorKind_e GTKind_e;

  enum Routing_e {
    routeGT = 0, routeSSN = 1
  };
  struct AEIndicator { //SCCP Address elements indicators
    bool      ssnRouted;   //route by SSN or GT
    GTKind_e  gti;
    bool      hasSSN;
    bool      hasSPC;

    AEIndicator()
      : ssnRouted(false), gti(GlobalTitle::gtiNone), hasSSN(false), hasSPC(false)
    { }

    //indicators octet layout:
    //uint8_t reserved : 1; //0 - reserved for national use
    //uint8_t routing  : 1; //1 - route by SSN, 0 - by GT
    //uint8_t gti      : 4; // = GlobalTitle::Indicator_e
    //uint8_t ssn      : 1; //1 - SSN present, 0 - missed
    //uint8_t spc      : 1; //1 - SPC present, 0 - missed
    uint8_t pack2Oct(void) const
    {
      uint8_t val = ((gti & 0x0F) << 2);
      if (ssnRouted)
        val |= 0x40;
      if (hasSSN)
        val |= 0x02;
      if (hasSPC)
        val |= 0x01;
      return val;
    }

    void unpackOct(uint8_t use_val)
    {
      hasSPC = (bool)(use_val & 0x01);
      hasSSN = (bool)(use_val & 0x02);
      ssnRouted = (bool)(use_val & 0x40);
      gti = static_cast<GTKind_e>((use_val >> 2) & 0x0F);
    }

    //Returns number of octets packed SCCP address occupies excluding address signals
    //( SPC + SSN + GT{TrT, NPi, Sch, NoA} )
    unsigned octsSize(void) const
    {
      return GlobalTitle::GTIndicator::octsSize(gti) + (hasSSN ? 1 : 0) + (hasSPC ? 2 : 0);
    }
  };

protected:
  AEIndicator _ind;
  uint16_t    _spc;
  uint8_t     _ssn;
  GlobalTitle _gt;

public:
  SCCPAddress() : _spc(0), _ssn(0)
  { }
  SCCPAddress(const GlobalTitle & use_gt, uint8_t use_ssn,
                              Routing_e use_route = routeGT)
   : _spc(0), _ssn(use_ssn), _gt(use_gt)
  {
    _ind.ssnRouted = (use_route == routeSSN);
    _ind.gti = _gt.getGTIKind();
    _ind.hasSSN = (bool)(_ssn != 0);
  }
  SCCPAddress(uint16_t use_spc, uint8_t use_ssn,
                              Routing_e use_route = routeSSN)
   : _spc(use_spc), _ssn(use_ssn)
  {
    _ind.ssnRouted = (use_route == routeSSN);
    _ind.hasSSN = (bool)(_ssn != 0);
    _ind.hasSPC = (bool)(_spc != 0);
  }

  void construct(const GlobalTitle & use_gt,
                 uint8_t use_ssn, Routing_e use_route = routeGT)
  {
    _spc = 0;
    _ssn = use_ssn;
    _gt = use_gt;

    _ind.ssnRouted = (use_route == routeSSN);
    _ind.gti = _gt.getGTIKind();
    _ind.hasSSN = (bool)(_ssn != 0);
    _ind.hasSPC = false;
  }
  void construct(uint16_t use_spc,
                 uint8_t use_ssn, Routing_e use_route = routeSSN)
  {
    _spc = use_spc;
    _ssn = use_ssn;
    _gt.reset();

    _ind.gti = GlobalTitle::gtiNone;
    _ind.ssnRouted = (use_route == routeSSN);
    _ind.hasSSN = (bool)(_ssn != 0);
    _ind.hasSPC = (bool)(_spc != 0);
  }

  //
  const AEIndicator & getIndicator(void) const { return _ind; }
  //
  uint8_t getSSN(void) const { return _ssn; }
  //
  uint16_t getSPC(void) const { return _spc; }
  //
  Routing_e getRouting(void) const
  {
    return _ind.ssnRouted ? SCCPAddress::routeSSN : SCCPAddress::routeGT;
  }
  //
  const GlobalTitle & getGT(void) const { return _gt; }


  //Encodes/Packs address to octet buffer
  //Returns number of characters packed, 0 - in case of failure.
  //NOTE: specified buffer must be able to store _maxOctsLen bytes!!!
  unsigned pack2Octs(uint8_t * use_buf) const;

  //Decodes/Unpacks address from octet buffer
  //Returns number of characters unpacked, 0 - in case of failure.
  unsigned unpackOcts(const uint8_t * use_buf, unsigned buf_len);

  //Composes string representation of GlobalTitle
  //NOTE: length of buffer should be at least _maxGTCharsNum 
  size_t  toString(char * use_buf, size_t max_len) const;

  //Composes string representation of SCCPAddress
  sccp_addr_string_t toString(void) const
  {
    sccp_addr_string_t adrStr;
    toString(adrStr.str, adrStr.capacity());
    return adrStr;
  }
};

typedef SCCPAddress::sccp_addr_string_t sccp_addr_string_t;

} //sccp
} //eyeline

#endif /* __EYELINE_SCCP_ADDRESS_HPP */

