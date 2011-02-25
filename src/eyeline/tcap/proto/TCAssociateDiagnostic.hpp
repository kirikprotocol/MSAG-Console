/* ************************************************************************** *
 * TCAP structured dialogue association result diagnostic.
 * ************************************************************************** */
#ifndef __TC_STR_DIALOGUE_DIAGNOSTIC_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __TC_STR_DIALOGUE_DIAGNOSTIC_HPP

#include "eyeline/tcap/TDialogueDefs.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

using eyeline::tcap::TDialogueAssociate;

struct AssociateSourceDiagnostic : public TDialogueAssociate {
  enum SourceKind_e {
    dlgSrvUser = 0x00, dlgSrvPrvd = 0x01
  };
  SourceKind_e  _sdKind;
  Diagnostic_t  _sdResult;

  AssociateSourceDiagnostic()
  {
    setUserDiagnostic(dsu_null); //default: accepted by user
  }

  void setUserDiagnostic(DiagnosticUser_e use_diagnostic = dsu_null)
  {
    _sdKind = dlgSrvUser;
    _sdResult = use_diagnostic;
  }
  void setPrvdDiagnostic(DiagnosticProvider_e use_diagnostic = dsp_null)
  {
    _sdKind = dlgSrvPrvd;
    _sdResult = use_diagnostic;
  }

  SourceKind_e getKind(void) const { return _sdKind; }

  DiagnosticUser_e getUserDiagnostic(void) const
  {
    return static_cast<DiagnosticUser_e>(_sdResult);
  }
  DiagnosticProvider_e getPrvdDiagnostic(void) const
  {
    return static_cast<DiagnosticProvider_e>(_sdResult);
  }
};

} //proto
} //tcap
} //eyeline

#endif /* __TC_STR_DIALOGUE_DIAGNOSTIC_HPP */

