#include "AARE_APdu.hpp"
#include "eyeline/tcap/proto/enc/AARE_APdu.hpp"
#include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging AARE_APdu::_typeTags =
    asn1::ASTagging(asn1::ASTag::tagApplication,
                    AARE_APdu::AARE_Tag,
                    asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging AARE_APdu::_protocolVersion_fieldTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    AARE_APdu::ProtocolVersion_FieldTag,
                    asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging AARE_APdu::_appCtxName_fieldTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    AARE_APdu::AppCtxName_FieldTag,
                    asn1::ASTagging::tagsEXPLICIT);

const asn1::ASTagging AARE_APdu::_associateResult_fieldTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    AARE_APdu::Result_FieldTag,
                    asn1::ASTagging::tagsEXPLICIT);

const asn1::ASTagging AARE_APdu::_associateSrcDiag_fieldTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    AARE_APdu::ResultSourceDiag_FieldTag,
                    asn1::ASTagging::tagsEXPLICIT);

const asn1::ASTagging AARE_APdu::_usrInfo_fieldTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    AARE_APdu::UserInfo_FieldTag,
                    asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging AARE_APdu::_dlgSvcUser_selectionTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    AARE_APdu::DlgSvcUser_SelectionTag,
                    asn1::ASTagging::tagsEXPLICIT);

const asn1::ASTagging AARE_APdu::_dlgSvcProvider_selectionTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    AARE_APdu::DlgSvcProvider_SelectionTag,
                    asn1::ASTagging::tagsEXPLICIT);

asn1::ber::EncoderOfINTEGER AARE_APdu::_dlgSvcUserValues[3] =
{
 asn1::ber::EncoderOfINTEGER(AARE_APdu::dsu_null),
 asn1::ber::EncoderOfINTEGER(AARE_APdu::dsu_no_reason_given),
 asn1::ber::EncoderOfINTEGER(AARE_APdu::dsu_ac_not_supported)
};

asn1::ber::EncoderOfINTEGER AARE_APdu::_dlgSvcPrvdValues[3] =
{
 asn1::ber::EncoderOfINTEGER(AARE_APdu::dsp_null),
 asn1::ber::EncoderOfINTEGER(AARE_APdu::dsp_no_reason_given),
 asn1::ber::EncoderOfINTEGER(AARE_APdu::dsp_no_common_dialogue_portion)
};

AARE_APdu::AARE_APdu(const asn1::EncodedOID* app_ctx,
                     asn1::TSGroupBER::Rule_e use_rule)
: EncoderOfSequence(_typeTags, use_rule),
  _resultEncoder(NULL), _resultSrcDiagEncoder(NULL), _usrInfo(NULL)
{
  _appCtxNameEncoder = new (_memForAppCtxNameEncoder.allocation) asn1::ber::EncoderOfEOID(*app_ctx);
}

void
AARE_APdu::acceptByUser(void)
{
  _resultEncoder = new (_memForResultEncoder.allocation ) asn1::ber::EncoderOfINTEGER(dlgAccepted);
  _resultSrcDiagEncoder = new (_memForResultSrcDiagEncoder.allocation) asn1::ber::EncoderOfChoice(getRule());
  _resultSrcDiagEncoder->setSelection(_dlgSvcUserValues[dsu_null], &_dlgSvcUser_selectionTags);
}

void
AARE_APdu::acceptByPrvd(void)
{
  _resultEncoder = new (_memForResultEncoder.allocation ) asn1::ber::EncoderOfINTEGER(dlgAccepted);
  _resultSrcDiagEncoder = new (_memForResultSrcDiagEncoder.allocation) asn1::ber::EncoderOfChoice(getRule());
  _resultSrcDiagEncoder->setSelection(_dlgSvcUserValues[dsu_null], &_dlgSvcProvider_selectionTags);
}

void
AARE_APdu::rejectByUser(AssociateSourceDiagnostic::DiagnosticUser_e cause)
{
  _resultEncoder = new (_memForResultEncoder.allocation ) asn1::ber::EncoderOfINTEGER(dlgRejectPermanent);
  _resultSrcDiagEncoder = new (_memForResultSrcDiagEncoder.allocation) asn1::ber::EncoderOfChoice(getRule());
  _resultSrcDiagEncoder->setSelection(_dlgSvcUserValues[cause], &_dlgSvcUser_selectionTags);
}

void
AARE_APdu::rejectByPrvd(AssociateSourceDiagnostic::DiagnosticProvider_e cause)
{
  _resultEncoder = new (_memForResultEncoder.allocation ) asn1::ber::EncoderOfINTEGER(dlgRejectPermanent);
  _resultSrcDiagEncoder = new (_memForResultSrcDiagEncoder.allocation) asn1::ber::EncoderOfChoice(getRule());
  _resultSrcDiagEncoder->setSelection(_dlgSvcPrvdValues[cause], &_dlgSvcProvider_selectionTags);
}

void
AARE_APdu::arrangeFields()
{
  if ( !_resultEncoder )
    throw utilx::SerializationException("AARE_APdu::arrangeFields::: missed mandatory field 'result'");

  if ( !_resultSrcDiagEncoder )
    throw utilx::SerializationException("AARE_APdu::arrangeFields::: missed mandatory field 'result-source-diagnostic'");

  addField(*_appCtxNameEncoder, &_appCtxName_fieldTags);
  addField(*_resultEncoder, &_associateResult_fieldTags);
  addField(*_resultSrcDiagEncoder, &_associateSrcDiag_fieldTags);
}

}}}}



