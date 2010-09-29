/* ************************************************************************** *
 * TCAP API: error codes definition.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_ERRORS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_ERRORS_HPP

namespace eyeline {
namespace tcap {

struct TCError {
  enum Code_e {
      dlgOk = 0
    , errSCCP               //dialogue primitive processing failed due to SCCP error
    , errTCAPUnknown        //dialogue primitive processing failed due to unexpected TCAP error
    , errTCAPLimit          //dialogue primitive processing failed due to resource limitation
    , errSrcAddress         //illegal source SCCP address
    , errDstAddress         //illegal destination SCCP address
    , errDialogueId         //unknown/illegal dialogueId (transaction Id)
    , errDialoguePortion    //dialogue UserInfo serialization error or
                            //UserInfo data is too large
    , errComponentPortion   //invalid component or component serialization error,
    , errTooMuchComponents  //Too much components in TR_Begin|TR_End
    , errInvokeId           //unknown/illegal invoke id
    , errDialogueState      //dialogue request primitive inconsistent with dialogue state
  };

  //Returns string representation of ErrorCode_e value
  static const char * nmErrorCode(Code_e use_val);
};

typedef TCError::Code_e TCRCode_e;

}}

#endif /* __EYELINE_TCAP_ERRORS_HPP */

