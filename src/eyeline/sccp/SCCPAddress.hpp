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

  typedef GlobalTitle::Indicator_e GTKind_e;

  enum Routing_e {
    routeGT = 0, routeSSN = 1
  };
  struct AEIndicator { //SCCP Address elements indicators
    uint8_t reserved : 1; //0 - reserved for national use
    uint8_t routing  : 1; // = Routing_e value
    uint8_t gti      : 4; // = GlobalTitle::Indicator_e
    uint8_t ssn      : 1; //1 - SSN present, 0 - missed
    uint8_t spc      : 1; //1 - SPC present, 0 - missed

    GlobalTitle::Indicator_e GTi(void) const
    {
      return static_cast<GlobalTitle::Indicator_e>(gti);
    }

    //Returns number of octets packed SCCP address occupies excluding address signals
    //( SPC + SSN + GT{TrT, NPi, Sch, NoA} )
    unsigned octsSize(void) const
    {
      return GlobalTitle::GTIndicator::octsSize(GTi())
             + (ssn ? 1 : 0) + (spc ? 2 : 0);
    }
  };

protected:
  union {
    uint8_t     val;
    AEIndicator parm;
  }           _ind;

  uint16_t    _spc;
  uint8_t     _ssn;
  GlobalTitle _gt;

public:
  SCCPAddress() : _spc(0), _ssn(0)
  {
    _ind.val = 0;
  }
  SCCPAddress(const GlobalTitle & use_gt, uint8_t use_ssn,
                              Routing_e use_route = routeGT)
   : _spc(0), _ssn(use_ssn), _gt(use_gt)
  {
    _ind.val = 0;
    _ind.parm.routing = use_route;
    _ind.parm.gti = _gt.GTi();
    _ind.parm.ssn = _ssn ? 1 : 0;
  }
  SCCPAddress(uint16_t use_spc, uint8_t use_ssn,
                              Routing_e use_route = routeSSN)
   : _spc(use_spc), _ssn(use_ssn)
  {
    _ind.val = 0;
    _ind.parm.routing = use_route;
    _ind.parm.ssn = _ssn ? 1 : 0;
    _ind.parm.spc = _spc ? 1 : 0;
  }

  void construct(const GlobalTitle & use_gt, uint8_t use_ssn,
                              Routing_e use_route = routeGT)
  {
    _spc = 0;
    _ssn = use_ssn;
    _gt = use_gt;

    _ind.val = 0;
    _ind.parm.routing = use_route;
    _ind.parm.gti = _gt.GTi();
    _ind.parm.ssn = _ssn ? 1 : 0;
  }
  void construct(uint16_t use_spc, uint8_t use_ssn,
                              Routing_e use_route = routeSSN)
  {
    _spc = use_spc;
    _ssn = use_ssn;
    _gt.reset();

    _ind.val = 0;
    _ind.parm.routing = use_route;
    _ind.parm.ssn = _ssn ? 1 : 0;
    _ind.parm.spc = _spc ? 1 : 0;
  }

  //
  const AEIndicator & getIndicator(void) const { return _ind.parm; }
  //
  uint8_t getSSN(void) const { return _ssn; }
  //
  uint16_t getSPC(void) const { return _spc; }
  //
  Routing_e getRouting(void) const
  {
    return static_cast<Routing_e>(_ind.parm.routing);
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

  typedef smsc::core::buffers::FixedLengthString<_maxOctsLen*2> sccp_addr_string_t;
  // TODO:: implement toString()
  sccp_addr_string_t toString() const;
};

typedef SCCPAddress::sccp_addr_string_t sccp_addr_string_t;

} //sccp
} //eyeline

#endif /* __EYELINE_SCCP_ADDRESS_HPP */

