/* ************************************************************************** *
 * TCAP dialogue related definitions and constants.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDIALOGUE_DEFS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_TDIALOGUE_DEFS_HPP__

#include <inttypes.h>

namespace eyeline {
namespace tcap {

typedef uint32_t TDialogueId;

typedef uint16_t TDlgTimeout; //units: seconds

struct PAbort {
  enum Cause_e {
    p_unrecognizedMessageType = 0
    , p_unrecognizedTransactionID = 1
    , p_badlyFormattedTransactionPortion = 2
    , p_incorrectTransactionPortion = 3
    , p_resourceLimitation = 4 //including dialogueTimeout
    , p_genericError = 127  //That's a max value allowed by protocol
  };
  typedef uint8_t Cause_t;
};

struct TDialogueAssociate {
  enum Result_e { dlg_accepted = 0, dlg_reject_permanent = 1 };
  typedef uint8_t Result_t;

  enum DiagnosticUser_e {
    dsu_null = 0, dsu_no_reason_given = 1, dsu_ac_not_supported = 2
  };
  enum DiagnosticProvider_e {
    dsp_null = 0, dsp_no_reason_given = 1, dsp_no_common_dialogue_portion = 2
  };
  typedef uint8_t Diagnostic_t;

  enum AbrtSource_e {
    abrtServiceUser = 0, abrtServiceProvider = 1
  };
  typedef uint8_t AbrtSource_t;
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TDIALOGUE_DEFS_HPP__ */

