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
EncoderOfExternal::~EncoderOfExternal()
{
  if (_encIRef)
    _encIRef->~EncoderOfINTEGER();
  if (_encDRef)
    _encDRef->~EncoderOfEOID();
  if (_encDescr)
    _encDescr->~EncoderOfObjDescriptor();
}

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
  if (val_ext.hasASyntaxOID()) {
    _encDRef = new (_memDRef.buf)EncoderOfEOID(val_ext._asOid);
    addField(*_encDRef);
  }
  if (val_ext.hasPrsContextId()) {
    _encIRef = new (_memIRef.buf)EncoderOfINTEGER(val_ext._prsCtxId);
    addField(*_encIRef);
  }
  if (!val_ext._descr.empty()) {
    _encDescr = new (_memDescr.buf)
                  EncoderOfObjDescriptor((TSLength)val_ext._descr.length(), val_ext._descr.c_str());
    addField(*_encDescr);
  }
  _encEnc.setValue(val_ext._enc);
  addField(_encEnc);
}

//
void EncoderOfExternal::setValue(int32_t prs_ctx, TypeEncoderAC & type_enc,
                                 const char * descr/* = NULL*/)
{
  _encIRef = new (_memIRef.buf)EncoderOfINTEGER(prs_ctx);
  addField(*_encIRef);

  if (descr && descr[0]) {
    _encDescr = new (_memDescr.buf)
                    EncoderOfObjDescriptor((TSLength)strlen(descr), descr);
    addField(*_encDescr);
  }
  _encEnc.setValue(type_enc);
  addField(_encEnc);
}

void EncoderOfExternal::setValue(const EncodedOID & as_oid, TypeEncoderAC & type_enc,
                                 const char * descr/* = NULL*/)
{
  _encDRef = new (_memDRef.buf)EncoderOfEOID(as_oid);
  addField(*_encDRef);

  if (descr && descr[0]) {
    _encDescr = new (_memDescr.buf)
                    EncoderOfObjDescriptor((TSLength)strlen(descr), descr);
    addField(*_encDescr);
  }
  _encEnc.setValue(type_enc);
  addField(_encEnc);
}

} //ber
} //asn1
} //eyeline

