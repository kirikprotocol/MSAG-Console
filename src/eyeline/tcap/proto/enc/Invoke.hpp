#ifndef __EYELINE_TCAP_PROTO_ENC_INVOKE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_INVOKE_HPP__

# include <sys/types.h>
# include "eyeline/ros/ROSComponent.hpp"
# include "eyeline/tcap/proto/enc/OperationOpCode.hpp"
# include "eyeline/tcap/proto/enc/InvokeIdType.hpp"
# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

enum { Invoke_Msg_Max_Num_Of_Fields = 4 };

class Invoke : public asn1::ber::EncoderOfSequence_T<Invoke_Msg_Max_Num_Of_Fields> {
public:
  Invoke()
  : _invokeIdEncoder(NULL), _linkedIdEncoder(NULL), _operationCodeEncoder(NULL)
  {}

  Invoke(ros::InvokeId invoke_id, uint8_t op_code,
         asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER);

  void setLinkedId(ros::InvokeId linked_id);

  void arrangeFields();

private:
  enum FieldTags_e {
    LinkedId_FieldTag = 0
  };

  InvokeIdType* _invokeIdEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(InvokeIdType)];
  } _memForInvokeIdEncoder;

  InvokeIdType* _linkedIdEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(InvokeIdType)];
  } _memForLinkedIdEncoder;

  Operation_OpCode* _operationCodeEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(Operation_OpCode)];
  } _memForOperationCodeEncoder;

  // _arguments;
  static const asn1::ASTagging _linkedId_fieldTags;
};

}}}}

#endif
