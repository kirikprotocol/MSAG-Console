/* ************************************************************************** *
 * TCAP structured dialogue association result diagnostic.
 * ************************************************************************** */
#ifndef __TC_STR_DIALOGUE_DIAGNOSTIC_HPP
#ident "@(#)$Id$"
#define __TC_STR_DIALOGUE_DIAGNOSTIC_HPP

#include "eyeline/tcap/TDialogueDefs.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

using eyeline::tcap::TDialogueAssociate;

class AssociateSourceDiagnostic : public TDialogueAssociate {
public:
  enum DiagnosticKind_e {
    dlgSrvUser = 0x01, dlgSrvPrvd = 0x02
  };

protected:
  DiagnosticKind_e  _sdKind;
  union {
    DiagnosticUser_e     user;
    DiagnosticProvider_e prvd;
  } _dsDiagnostic;

public:
  AssociateSourceDiagnostic()
  {
    setUserDiagnostic(dsu_null); //default: accepted by user 
  }

  void setUserDiagnostic(DiagnosticUser_e use_diagnostic = dsu_null)
  {
    _sdKind = dlgSrvUser;
    _dsDiagnostic.user = use_diagnostic;
  }
  void setPrvdDiagnostic(DiagnosticProvider_e use_diagnostic = dsp_null)
  {
    _sdKind = dlgSrvPrvd;
    _dsDiagnostic.prvd = use_diagnostic;
  }

  DiagnosticKind_e diagnosticKind(void) const { return _sdKind; }

  DiagnosticUser_e getUserDiagnostic(void) const
  {
    return _dsDiagnostic.user;
  }
  DiagnosticProvider_e getPrvdDiagnostic(void) const
  {
    return _dsDiagnostic.prvd;
  }
};

} //proto
} //tcap
} //eyeline

#endif /* __TC_STR_DIALOGUE_DIAGNOSTIC_HPP */

