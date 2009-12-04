#ifndef __EYELINE_TCAP_PROTO_ENC_REJECT_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_REJECT_HPP__

# include <sys/types.h>

# include "eyeline/asn1/ASNTags.hpp"
# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

# include "eyeline/ros/ROSComponent.hpp"
# include "eyeline/ros/ROSRejectProblem.hpp"
# include "eyeline/tcap/proto/enc/InvokeIdType.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

enum { Reject_Max_Num_Of_Fields = 2 };

class Reject : public asn1::ber::EncoderOfSequence_T<Reject_Max_Num_Of_Fields> {
public:
  explicit Reject(asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER);

  Reject(ros::InvokeId invoke_id,
         asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER);

  void setProblem(ros::RejectProblem::GeneralProblem_e problem_value);
  void setProblem(ros::RejectProblem::InvokeProblem_e problem_value);
  void setProblem(ros::RejectProblem::RResultProblem_e problem_value);
  void setProblem(ros::RejectProblem::RErrorProblem_e problem_value);

  void arrangeFields();

private:
  InvokeIdType* _derivableEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(InvokeIdType)];
  } _memForDerivableEncoder;

  asn1::ber::EncoderOfChoice* _invokeIdEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(asn1::ber::EncoderOfChoice)];
  } _memForInvokeIdEncoder;

  asn1::ber::EncoderOfChoice* _problemEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(asn1::ber::EncoderOfChoice)];
  } _memForProblemEncoder;

  asn1::ber::EncoderOfINTEGER* _problemSelectionEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(asn1::ber::EncoderOfINTEGER)];
  } _memForProblemSelectionEncoder;

  static const asn1::ASTagging _generalProblem_selectionTags;
  static const asn1::ASTagging _invokeProblem_selectionTags;
  static const asn1::ASTagging _returnResultProblem_selectionTags;
  static const asn1::ASTagging _returnErrorProblem_selectionTags;

  enum SelectionTags_e {
    GeneralProblem_SelectionTag = 0, InvokeProblem_SelectionTag = 1,
    ReturnResultProblem_SelectionTag = 2, ReturnErrorProblem_SelectionTag = 3
  };
};

}}}}

#endif
