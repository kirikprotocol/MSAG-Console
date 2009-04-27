/* ************************************************************************** *
 * TCAP structured dialogue association result diagnostic.
 * ************************************************************************** */
#ifndef __TC_STR_DIALOGUE_DIAGNOSTIC_HPP
#ident "@(#)$Id$"
#define __TC_STR_DIALOGUE_DIAGNOSTIC_HPP

namespace eyeline {
namespace tcap {
namespace proto {

class AssociateSourceDiagnostic {
public:
  enum SDKind_e {
    dlgSrvUser = 0x01, dlgSrvPrvd = 0x02
  };
  enum DlgSrvUser_e {
    dsu_null = 0, dsu_no_reason_given = 1, dsu_ac_not_supported = 2
  };
  enum DlgSrvProvider_e {
    dsp_null = 0, dsp_no_reason_given = 1, dsp_no_dialogue_portion = 2
  };

protected:
  SDKind_e  _sdKind;
  union {
    DlgSrvUser_e     user;
    DlgSrvProvider_e prvd;
  } _dsDiagnostic;

public:
  AssociateSourceDiagnostic()
  {
    setUserDiagnostic(dsu_null); //default: accepted by user 
  }

  void setUserDiagnostic(DlgSrvUser_e use_diagnostic = dsu_null)
  {
    _sdKind = dlgSrvUser;
    _dsDiagnostic.user = use_diagnostic;
  }
  void setPrvdDiagnostic(DlgSrvProvider_e use_diagnostic = dsp_null)
  {
    _sdKind = dlgSrvPrvd;
    _dsDiagnostic.prvd = use_diagnostic;
  }

  SDKind_e diagnosticKind(void) const { return _sdKind; }

  DlgSrvUser_e getUserDiagnostic(void) const
  {
    return _dsDiagnostic.user;
  }
  DlgSrvProvider_e getPrvdDiagnostic(void) const
  {
    return _dsDiagnostic.prvd;
  }
};

} //proto
} //tcap
} //eyeline

#endif /* __TC_STR_DIALOGUE_DIAGNOSTIC_HPP */

