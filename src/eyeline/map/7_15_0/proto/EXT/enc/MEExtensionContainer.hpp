#ifndef __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEEXTENSIONCONTAINER_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEEXTENSIONCONTAINER_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEPrivateExtensionList.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEPCS_Extensions.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

/*
 ExtensionContainer ::= SEQUENCE {
    privateExtensionList    [0] PrivateExtensionList OPTIONAL,
    pcs-Extensions          [1] PCS-Extensions       OPTIONAL,
    ...
 }
*/
class MEExtensionContainer : public asn1::ber::EncoderOfSequence_T<3,2> {
public:
  explicit MEExtensionContainer(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<3,2>(use_rule),
    _privateExtensionList(NULL), _pcsExtensions(NULL)
  {}

  MEExtensionContainer(const ExtensionContainer& ext_container,
                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<3,2>(use_rule),
    _privateExtensionList(NULL), _pcsExtensions(NULL)
  {
    setValue(ext_container);
  }

  MEExtensionContainer(const asn1::ASTag& outer_tag,
                       const asn1::ASTagging::Environment_e tag_env,
                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<3,2>(outer_tag, tag_env, use_rule),
    _privateExtensionList(NULL), _pcsExtensions(NULL)
  {}

  ~MEExtensionContainer() {
    if (_privateExtensionList)
      _privateExtensionList->~MEPrivateExtensionList();
    if (_pcsExtensions)
      _pcsExtensions->~MEPCS_Extensions();
  }

  void setValue(const ExtensionContainer& ext_container) {
    const PrivateExtensionList* privateExtList= ext_container.getPrivateExtensionList();
    if (privateExtList) {
      _privateExtensionList= new (_memAlloc_MEPrivateExtensionList.buf) MEPrivateExtensionList(*privateExtList, getTSRule());
      //_privateExtensionList->setValue(*privateExtList);
      setField(0, *_privateExtensionList);
    }
    const PCS_Extensions* pcsExts= ext_container.getPCS_Extensions();
    if (pcsExts) {
      _pcsExtensions= new (_memAlloc_MEPCS_Extensions.buf) MEPCS_Extensions(*pcsExts, getTSRule());
      setField(1, *_pcsExtensions);
    }
    //    if ( !value._unkExt._tsList.empty() ) {
    //      asn1::ber::EncoderOfUExtension* encoderOfUExt= new asn1::ber::EncoderOfUExtension();
    //      encoderOfUExt->setValue(value._unkExt);
    //    }

  }

private:
  union {
    void* aligner;
    uint8_t buf[sizeof(MEPrivateExtensionList)];
  } _memAlloc_MEPrivateExtensionList;

  union {
    void* aligner;
    uint8_t buf[sizeof(MEPCS_Extensions)];
  } _memAlloc_MEPCS_Extensions;

  MEPrivateExtensionList* _privateExtensionList;
  MEPCS_Extensions* _pcsExtensions;
};

}}}}

#endif
