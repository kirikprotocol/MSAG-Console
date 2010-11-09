/* ************************************************************************* *
 * BER group of transfer syntaxes.
 * ************************************************************************* */
#ifndef __BER_TRANSFER_SYNTAXES_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __BER_TRANSFER_SYNTAXES_DEFS

#include "eyeline/asn1/TransferSyntax.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

struct TSGroupBER { //octet aligned encodings
  enum Rule_e {
      ruleBER = asn1::TransferSyntax::ruleBER //ruleBasic
    , ruleDER = asn1::TransferSyntax::ruleDER //ruleDistinguished
    , ruleCER = asn1::TransferSyntax::ruleCER //ruleCanonical
  };

  static bool isBERSyntax(asn1::TransferSyntax::Rule_e use_rule) /*throw()*/
  {
    return ((use_rule >= asn1::TransferSyntax::ruleBER) || (use_rule <= asn1::TransferSyntax::ruleCER));
  }
  //Returns true if encoding of curr_rule is considered as valid encoding for tgt_rule
  static bool isPortable(TSGroupBER::Rule_e tgt_rule, TSGroupBER::Rule_e curr_rule) /*throw()*/
  {
    return (tgt_rule == ruleBER) ? true : (tgt_rule == curr_rule);
  }
  //
  static Rule_e getBERRule(asn1::TransferSyntax::Rule_e use_rule) /*throw(std::exception)*/
  {
    if (!TSGroupBER::isBERSyntax(use_rule))
      throw smsc::util::Exception("transferSyntax(%s) isn't of BER group",
                                  asn1::TransferSyntax::nmRule(use_rule));
    return static_cast<TSGroupBER::Rule_e>(use_rule);
  }
  //
  static asn1::TransferSyntax::Rule_e getTSRule(TSGroupBER::Rule_e ber_rule) /*throw()*/
  {
    return static_cast<asn1::TransferSyntax::Rule_e>(ber_rule);
  }
  //
  static const char * nmRule(TSGroupBER::Rule_e rule_id) /*throw()*/
  {
    return asn1::TransferSyntax::nmRule(getTSRule(rule_id));
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __BER_TRANSFER_SYNTAXES_DEFS */

