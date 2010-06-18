/* ************************************************************************* *
 * EmbeddedEncoding helper type: used in EXTERNAL/EMBEDDED PDV definitions.
 * ************************************************************************* */
#ifndef __ASN1_EMBEDDED_ENCODING_DEFS
#ident "@(#)$Id$"
#define __ASN1_EMBEDDED_ENCODING_DEFS

#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BITSTR.hpp"
#include "eyeline/asn1/OCTSTR.hpp"

namespace eyeline {
namespace asn1 {

//According to X.690 cl.8.18.1,
//EmbeddedEncoding type is defined in EXPLICIT tagging environment as follow:
//
//  EmbeddedEncoding ::= CHOICE {
//    single-ASN1-type  [0] ABSTRACT-SYNTAX.&Type,
//    octet-aligned     [1] IMPLICIT OCTET STRING,
//    arbitrary         [2] IMPLICIT BIT STRING
//  }
class EmbeddedEncoding {
public:
  enum Kind_e {
    evASType = 0,     //encoding of value of defined single ASN.1 type
    evOctAligned = 1, //arbitrary octet aligned encoding
    evBitAligned = 2  //arbitrary bit aligned encoding
  };

private:
  union {
    void *  _aligner;
    uint8_t _buf[eyeline::util::MaxSizeOf3_T<TransferSyntax, OCTSTR, BITSTR>::VALUE];
  } _memEnc;

protected:
  Kind_e _kind;
  union {
    TransferSyntax * _ts;
    OCTSTR         * _octs;
    BITSTR         * _bits;
  } _enc;

public:
  EmbeddedEncoding() : _kind(evASType)
  {
    _memEnc._aligner = _enc._ts = 0;
  }
  ~EmbeddedEncoding()
  {
    reset();
  }

  Kind_e getKind(void) const { return _kind; }

  bool isTS(void)   const { return (_kind == evASType); }
  bool isOCTS(void) const { return (_kind == evOctAligned); }
  bool isBITS(void) const { return (_kind == evBitAligned); }

  //
  TransferSyntax * getTS(void) { return (_kind == evASType) ? _enc._ts : 0; }
  //
  OCTSTR::ArrayType * getOCTS(void) { return (_kind == evOctAligned) ? _enc._octs : 0; }
  //
  BITSTR::ArrayType * getBITS(void) { return (_kind == evBitAligned) ? _enc._bits : 0; }

  //
  const TransferSyntax * getTS(void) const { return (_kind == evASType) ? _enc._ts : 0; }
  //
  const OCTSTR::ArrayType * getOCTS(void) const { return (_kind == evOctAligned) ? _enc._octs : 0; }
  //
  const BITSTR::ArrayType * getBITS(void) const { return (_kind == evBitAligned) ? _enc._bits : 0; }

  void reset(void)
  {
    if (_enc._ts) {
      if (_kind == evASType)
        _enc._ts->~TransferSyntax();
      else if (_kind == evOctAligned) {
        _enc._octs->~OCTSTR();
      } else // (_kind == evBitAligned)
        _enc._bits->~BITSTR();
      _enc._ts = 0;
    }
  }

  TransferSyntax & initTS(void)
  {
    _kind = evASType;
    return *(_enc._ts = new (_memEnc._buf) TransferSyntax());
  }
  //
  OCTSTR::ArrayType & initOCTS(void)
  {
    _kind = evOctAligned;
    return *(_enc._octs = new (_memEnc._buf) OCTSTR());
  }
  //
  BITSTR::ArrayType & initBITS(void)
  {
    _kind = evBitAligned;
    return *(_enc._bits = new (_memEnc._buf) BITSTR());
  }
};

} //asn1
} //eyeline

#endif /* __ASN1_EMBEDDED_ENCODING_DEFS */

