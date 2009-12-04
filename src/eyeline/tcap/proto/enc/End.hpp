#ifndef __EYELINE_TCAP_PROTO_END_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_END_HPP__

# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/tcap/provd/SUARequests.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"
# include "eyeline/tcap/proto/enc/DialoguePortion.hpp"
# include "eyeline/tcap/proto/enc/ComponentPortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class End {
public:
  explicit End(TransactionId transaction_id)
  : _transactionId(transaction_id), _componentPortion(NULL), _dlgPortion(NULL)
  {}

  void setDialoguePortion(DialoguePortion* dlg_portion);

  void setComponentPortion(ComponentPortion* component_portion);

  asn1::ENCResult encode(provd::UDTDataBuffer& used_buffer,
                         asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER);
private:
  TransactionId _transactionId;
  ComponentPortion* _componentPortion;
  DialoguePortion* _dlgPortion;

  enum SelectionTags_e {
    End_SelectionTag = 4
  };
};

}}}}

#endif
