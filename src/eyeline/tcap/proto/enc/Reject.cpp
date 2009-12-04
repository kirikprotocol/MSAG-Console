#include "Reject.hpp"
#include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging Reject::_generalProblem_selectionTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    Reject::GeneralProblem_SelectionTag,
                    asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging Reject::_invokeProblem_selectionTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    Reject::InvokeProblem_SelectionTag,
                    asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging Reject::_returnResultProblem_selectionTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    Reject::ReturnResultProblem_SelectionTag,
                    asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging Reject::_returnErrorProblem_selectionTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    Reject::ReturnErrorProblem_SelectionTag,
                    asn1::ASTagging::tagsIMPLICIT);

Reject::Reject(ros::InvokeId invoke_id,
               asn1::TSGroupBER::Rule_e use_rule)
: asn1::ber::EncoderOfSequence_T<Reject_Max_Num_Of_Fields>(use_rule),
  _problemEncoder(NULL), _problemSelectionEncoder(NULL)
{
  _derivableEncoder = new ( _memForDerivableEncoder.allocation ) InvokeIdType(invoke_id, use_rule);
  _invokeIdEncoder = new (_memForInvokeIdEncoder.allocation) asn1::ber::EncoderOfChoice(use_rule);
  _invokeIdEncoder->setSelection(*_derivableEncoder);
}

void
Reject::setProblem(ros::RejectProblem::GeneralProblem_e problem_value)
{
  if ( !_problemEncoder )
    _problemEncoder = new (_memForProblemEncoder.allocation) asn1::ber::EncoderOfChoice(getRule());
  _problemSelectionEncoder = new (_memForProblemSelectionEncoder.allocation) asn1::ber::EncoderOfINTEGER(problem_value, getRule());
  _problemEncoder->setSelection(*_problemSelectionEncoder, &_generalProblem_selectionTags);
}

void
Reject::setProblem(ros::RejectProblem::InvokeProblem_e problem_value)
{
  if ( !_problemEncoder )
    _problemEncoder = new (_memForProblemEncoder.allocation) asn1::ber::EncoderOfChoice(getRule());
  _problemSelectionEncoder = new (_memForProblemSelectionEncoder.allocation) asn1::ber::EncoderOfINTEGER(problem_value, getRule());
  _problemEncoder->setSelection(*_problemSelectionEncoder, &_invokeProblem_selectionTags);
}

void
Reject::setProblem(ros::RejectProblem::RResultProblem_e problem_value)
{
  if ( !_problemEncoder )
    _problemEncoder = new (_memForProblemEncoder.allocation) asn1::ber::EncoderOfChoice(getRule());
  _problemSelectionEncoder = new (_memForProblemSelectionEncoder.allocation) asn1::ber::EncoderOfINTEGER(problem_value, getRule());
  _problemEncoder->setSelection(*_problemSelectionEncoder, &_returnResultProblem_selectionTags);
}

void
Reject::setProblem(ros::RejectProblem::RErrorProblem_e problem_value)
{
  if ( !_problemEncoder )
    _problemEncoder = new (_memForProblemEncoder.allocation) asn1::ber::EncoderOfChoice(getRule());
  _problemSelectionEncoder = new (_memForProblemSelectionEncoder.allocation) asn1::ber::EncoderOfINTEGER(problem_value, getRule());
  _problemEncoder->setSelection(*_problemSelectionEncoder, &_returnErrorProblem_selectionTags);
}

void
Reject::arrangeFields()
{
  addField(*_invokeIdEncoder);
  if ( !_problemEncoder )
    throw utilx::SerializationException("Reject::arrangeFields::: missed mandatory field 'problem'");
  if ( !_problemSelectionEncoder )
    throw utilx::SerializationException("Reject::arrangeFields::: missed selection for CHOICE 'problem'");
  addField(*_problemEncoder);
}

}}}}
