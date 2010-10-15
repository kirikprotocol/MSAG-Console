#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/EncodeExternal.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
/* ************************************************************************* *
 * Class EncoderOfExternal implementation:
 * ************************************************************************* */
//Constructs SEQUENCE representing EXTERNAL type, according to X.690 cl.8.18.1
//Tagging environment is EXPLICIT. 
//
//[UNIVERSAL 8] IMPLICIT SEQUENCE {
//           direct-reference       OBJECT IDENTIFIER OPTIONAL,
//           indirect-reference     INTEGER OPTIONAL,
//           data-value-descriptor  ObjectDescriptor OPTIONAL,
//           encoding               EmbeddedEncoding
//          }
void EncoderOfExternal::setValue(const ASExternal & val_ext)
    /*throw(std::exception)*/
{
  clearFields();

  if (val_ext.hasASyntaxOID()) {
    _encDRef.init(getTSRule()).setValue(val_ext._asOid);
    setField(0, *_encDRef.get());
  }
  if (val_ext.hasPrsContextId()) {
    _encIRef.init(getTSRule()).setValue(val_ext._prsCtxId);
    setField(1, *_encIRef.get());
  }
  if (!val_ext._descr.empty()) {
    _encDescr.init(getTSRule()).setValue((TSLength)val_ext._descr.length(), val_ext._descr.c_str());
    setField(2, *_encDescr.get());
  }
  _encEnc.setValue(val_ext._enc);
  setField(3, _encEnc);
}

//
void EncoderOfExternal::setValue(int32_t prs_ctx, TypeEncoderAC & type_enc,
                                 const char * descr/* = NULL*/)
{
  clearFields();

  _encIRef.init(getTSRule()).setValue(prs_ctx);
  setField(1, *_encIRef.get());

  if (descr && descr[0]) {
    _encDescr.init(getTSRule()).setValue((TSLength)strlen(descr), descr);
    setField(2, *_encDescr.get());
  }
  _encEnc.setValue(type_enc);
  setField(3, _encEnc);
}

void EncoderOfExternal::setValue(const EncodedOID & as_oid, TypeEncoderAC & type_enc,
                                 const char * descr/* = NULL*/)
{
  clearFields();

  _encDRef.init(getTSRule()).setValue(as_oid);
  setField(0, *_encDRef.get());

  if (descr && descr[0]) {
    _encDescr.init(getTSRule()).setValue((TSLength)strlen(descr), descr);
    setField(2, *_encDescr.get());
  }
  _encEnc.setValue(type_enc);
  setField(3, _encEnc);
}

} //ber
} //asn1
} //eyeline

