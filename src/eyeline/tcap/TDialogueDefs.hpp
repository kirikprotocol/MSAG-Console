/* ************************************************************************** *
 * TCAP dialogue related definitions and constants.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDIALOGUE_DEFS_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDIALOGUE_DEFS_HPP__

# include "eyeline/tcap/TDialogueId.hpp"

namespace eyeline {
namespace tcap {

struct PAbort {
  enum Cause_e {
    p_unrecognizedMessageType = 0
    , p_unrecognizedTransactionID = 1
    , p_badlyFormattedTransactionPortion = 2
    , p_incorrectTransactionPortion = 3
    , p_resourceLimitation = 4
    , p_dialogueTimeout = 126
    , p_genericError = 127  //That's a max value allowed by protocol
  };
};

struct TDialogueAssociate {
  enum Result_e { accepted = 0, reject_permanent = 1 };

  enum DiagnosticUser_e {
    dsu_null = 0, dsu_no_reason_given = 1, dsu_ac_not_supported = 2
  };
  enum DiagnosticProvider_e {
    dsp_null = 0, dsp_no_reason_given = 1, dsp_no_common_dialogue_portion = 2
  };
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TDIALOGUE_DEFS_HPP__ */

