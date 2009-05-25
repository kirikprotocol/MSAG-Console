/* ************************************************************************** *
 * Classes implementing structured TCAP dialogue requests.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__

# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/asn1/EncodedOID.hpp"
# include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"
# include "eyeline/tcap/proto/TCAssociateDiagnostic.hpp"

namespace eyeline {
namespace tcap {

using eyeline::sccp::SCCPAddress;
//using eyeline::asn1::EncodedOID;

class TDialogueRequestPrimitive : public TDialogueHandlingPrimitive {
protected:
  bool                _retOnErr;
  ROSComponentsList   _comps;
  const EncodedOID *  _acOId;

public:
  TDialogueRequestPrimitive()
    : _retOnErr(false), _acOId(0)
  { }
  virtual ~TDialogueRequestPrimitive()
  { }

  void setAppCtx(const EncodedOID & use_acid) { _acOId = &use_acid; }

  //These methods only for 1st response to T_Begin_Ind
  void acceptDlgByUser(void)
  { /*TODO: */ }
  void rejectDlgByUser(proto::AssociateSourceDiagnostic::DlgSrvUser_e use_cause =
                       proto::AssociateSourceDiagnostic::dsu_null)
  { /*TODO: */  }

  void setReturnOnError() { _retOnErr = true; }
  bool getReturnOnError() const { return _retOnErr; }

  // ------------------------------------------------
  // -- TDialogueHandlingPrimitive interface methods
  // ------------------------------------------------
  virtual const EncodedOID * getAppCtx(void) const { return _acOId; }
  virtual ROSComponentsList * CompList(void) { return &_comps; }
};

//
class TC_Begin_Req : public TDialogueRequestPrimitive {
protected:
  SCCPAddress   _orgAdr;
  SCCPAddress   _dstAdr;

public:
  TC_Begin_Req()
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }

  void setDestAddress(const SCCPAddress & use_adr) { _dstAdr = use_adr; }
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }

  //TODO: setters
};
//
class TC_Cont_Req : public TDialogueRequestPrimitive {
protected:
  SCCPAddress   _orgAdr;

public:
  TC_Cont_Req()
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }

  //TODO: setters
};
//
class TC_End_Req : public TDialogueRequestPrimitive {
public:
  TC_End_Req()
  { }

  typedef enum { PREARRANGED_END, BASIC_END } end_transaction_facility_t;

  end_transaction_facility_t getTermination() const;
  //TODO: setters
};

//
class TC_UAbort_Req : public TDialogueRequestPrimitive {
public:
  TC_UAbort_Req()
  { }

  //TODO: setters for TCAbrtPDU or TCExternal
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__ */

