#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {

const char * TransferSyntax::nmRule(TransferSyntax::Rule_e rule_id) /*throw()*/
{
  switch (rule_id) {
  case TransferSyntax::ruleBER: return "BER";
  case TransferSyntax::ruleDER: return "DER";
  case TransferSyntax::ruleCER: return "CER";
  case TransferSyntax::rulePER: return "PER";
  case TransferSyntax::ruleCPER: return "CPER";
  case TransferSyntax::ruleXER: return "XER";
  case TransferSyntax::ruleCXER: return "CXER";
  case TransferSyntax::ruleEXER: return "EXER";
  case TransferSyntax::ruleUPER: return "UPER";
  case TransferSyntax::ruleCUPER: return "CUPER";
  default:;
  }
  return "undefined";
}

} //asn1
} //eyeline

