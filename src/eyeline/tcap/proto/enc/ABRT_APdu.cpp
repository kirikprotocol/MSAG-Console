#include "ABRT_APdu.hpp"
#include "eyeline/utilx/Exception.hpp"


namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging ABRT_APdu::_typeTags =
    asn1::ASTagging(asn1::ASTag::tagApplication,
                    ABRT_APdu::ABRT_Tag,
                    asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging ABRT_APdu::_abortSource_fieldTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    ABRT_APdu::AbortSource_FieldTag,
                    asn1::ASTagging::tagsIMPLICIT);

ABRT_APdu::ABRT_APdu(TDialogueAssociate::AbrtSource_e abort_source,
                     asn1::TSGroupBER::Rule_e use_rule)
: EncoderOfSequence(_typeTags, use_rule),
  _abortSourceEncoder(NULL), _usrInfo(NULL)
{
  _abortSourceEncoder = new (_memForAbortSourceEncoder.allocation) asn1::ber::EncoderOfINTEGER(abort_source),
  addField(*_abortSourceEncoder, &_abortSource_fieldTags);
}

void
ABRT_APdu::arrangeFields()
{
  if ( !_abortSourceEncoder )
    throw utilx::SerializationException("ABRT_APdu::arrangeFields::: missed mandatory field 'abort-source'");
  addField(*_abortSourceEncoder);
}

}}}}
