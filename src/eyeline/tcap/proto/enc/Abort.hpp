#ifndef __EYELINE_TCAP_PROTO_ABORTMESSAGE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ABORTMESSAGE_HPP__

# include "logger/Logger.h"

# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

# include "eyeline/tcap/provd/SUARequests.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class Abort {
public:
  Abort(TransactionId transaction_id, asn1::ber::TypeEncoderAC* reason)
  : _transactionId(transaction_id),
    _reason(reason)
  {}

  asn1::ENCResult encode(provd::UDTDataBuffer& used_buffer,
                         asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER);
private:
  TransactionId _transactionId;
  asn1::ber::TypeEncoderAC* _reason;

  enum SelectionTags_e {
    Abort_SelectionTag = 7
  };
};

}}}}

#endif
