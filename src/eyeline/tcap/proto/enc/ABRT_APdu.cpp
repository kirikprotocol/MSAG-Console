# include "ABRT_APdu.hpp"
# include "eyeline/utilx/Exception.hpp"
# include "eyeline/asn1/ASNTags.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

//asn1::ENCResult
//ABRT_APdu::encodeVAL(uint8_t * used_enc, asn1::TSLength max_len)
//{
//  if ( _rule == asn1::TSGroupBER::ruleDER ||
//       _rule == asn1::TSGroupBER::ruleBER ) {
    // asn1::ASTagging pduTags(2, asn1::ASTag(asn1::ASTag::tagApplication, ABRT_Tag_Value),
    //                         asn1::ASTag(ASTag::tagUniversal, asn1::_tagSEQOF, true));
    // pduTags.setEnvironment(asn1::ASTagging::tagsIMPLICIT);
    // asn1::EncoderOfSequence pdu(pduTags);

    // asn1::ASTagging fieldTags(2, asn1::ASTag(asn1::ASTag::tagContextSpecific, ABRT_Source_Tag_Value),
    //                           asn1::ASTag(asn1::ASTag::tagUniversal, asn1::_tagINTEGER));
    // fieldTags.setEnvironment(asn1::ASTagging::tagsIMPLICIT);
    // asn1::ber::EncoderOfINTEGER abortSource(_abortSource, fieldTags);
    // pdu.addField(abortSource);

    // pdu.encodeTLV(used_enc, max_len);
    // return asn1::ENCResult::encOk;
//    asn1::EncoderOfSequence pdu(pduTags);
//
//    asn1::ASTagging fieldTags(2, asn1::ASTag(asn1::ASTag::tagContextSpecific, ABRT_Source_Tag_Value),
//                              asn1::ASTag(asn1::ASTag::tagUniversal, asn1::_tagINTEGER));
//    fieldTags.setEnvironment(asn1::ASTagging::tagsIMPLICIT);
//    asn1::ber::EncoderOfINTEGER abortSource(_abortSource, fieldTags);
//    pdu.addField(abortSource);
//
//    pdu.encodeTLV(used_enc, max_len);
//    return asn1::ENCResult::encOk;
//  } else
//    throw utilx::SerializationException("ABRT_APdu::encode::: invalid rule coding value=%d",
//                                        use_rule);
//}

}}}}
