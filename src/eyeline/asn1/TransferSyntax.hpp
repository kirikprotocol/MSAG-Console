/* ************************************************************************* *
 * ASN.1 transfer syntaxes definition.
 * ************************************************************************* */
#ifndef __TRANSFER_SYNTAX_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __TRANSFER_SYNTAX_DEFS

#include <inttypes.h>

namespace eyeline {
namespace asn1 {

typedef uint32_t  TSLength; //type for overall encoding length

//Octet buffer storing transferSyntax encoding
class TSBuffer {
private:
  bool _isConst;
  union {
    uint8_t * toVolat;
    const uint8_t * toConst;
  } _ptr;

public:
  TSLength  _maxlen;  //encoding length in units(bytes or bits depending
                      //on TransferSyntax alignment

  //Verifies that length of TSLength type fit into _SizeTypeArg type.
  template <
    typename _SizeTypeArg //MUST be an unsigned integer type
  >
  static _SizeTypeArg adoptRange(TSLength max_len)
  {
    return (((max_len) > (_SizeTypeArg)(-1)) ? (_SizeTypeArg)(-1) : (_SizeTypeArg)(max_len));
  }

  explicit TSBuffer(uint8_t * use_buf = 0, TSLength enc_len = 0)
    : _isConst(false), _maxlen(enc_len)
  {
    _ptr.toVolat = use_buf;
  }
  TSBuffer(const uint8_t * use_buf, TSLength enc_len)
    : _isConst(true), _maxlen(enc_len)
  {
    _ptr.toConst = use_buf;
  }
  ~TSBuffer()
  { }

  void setPtr(uint8_t * use_ptr)
  {
    _isConst = false; _ptr.toVolat = use_ptr;
  }
  void setPtr(const uint8_t * use_ptr)
  {
    _isConst = true; _ptr.toConst = use_ptr;
  }

  uint8_t * getPtr(void) { return _isConst ? 0 : _ptr.toVolat; }

  const uint8_t * getPtr(void) const { return _ptr.toConst; }
};

class TransferSyntax : public TSBuffer {
public:
  enum Rule_e {
    undefinedER = 0
    //octet aligned encodings:
    , ruleBER   //ruleBasic
    , ruleDER   //ruleDistinguished
    , ruleCER   //ruleCanonical
    , rulePER   //rulePacked_aligned
    , ruleCPER  //rulePacked_aligned_canonical
    , ruleXER   //ruleXml
    , ruleCXER  //ruleXml_canonical
    , ruleEXER  //ruleXml_extended
    //bit aligned encodings:
    , ruleUPER  //rulePacked_unaligned
    , ruleCUPER //rulePacked_unaligned_canonical
  };

  Rule_e  _rule;

  TransferSyntax() : TSBuffer(), _rule(undefinedER)
  { }
  TransferSyntax(const TSBuffer & use_buf, Rule_e use_rule)
    : TSBuffer(use_buf), _rule(use_rule)
  { }
  ~TransferSyntax()
  { }

  static const char * nmRule(TransferSyntax::Rule_e rule_id); /*throw()*/

  const char * nmRule(void) const { return nmRule(_rule); }
};

struct ENCResult {
  enum Status_e {
    encErrInternal = -3 //internal encoder error
    , encMoreMem = -2   //not enough memory for resulted encoding
    , encUnsupported = -1 //unsupported encoder action
    , encOk = 0
    , encBadVal = 1     //invalid/illegal value is to encode
    , encBadArg = 2     //incorrect argument is passed to encoder
  };

  Status_e  status; //encoding status
  TSLength  nbytes; //number of bytes successfully encoded (even
                    //if encMoreMem indicated)

  explicit ENCResult(Status_e use_status = encBadVal, TSLength nb_encoded = 0)
    : status(use_status), nbytes(nb_encoded)
  { }

  bool isOk(void) const { return (status == encOk); }

  ENCResult & operator+=(const ENCResult & use_res)
  {
    status = use_res.status;
    nbytes += use_res.nbytes;
    return *this;
  }
};

struct DECResult {
  enum Status_e {
    decErrInternal = -3 //internal decoder error
    , decMoreInput = -2 //insufficient input encoding to complete
    , decBadEncoding = -1 //corrupted or unsupported input encoding
    , decOk = 0
    , decOkRelaxed = 1  //value is decoded but input encoding violates
                        //TransferSyntax requirements (f.ex. canonical restrictions)
    , decBadVal = 2     //value may be decoded but it's illegal/unsupported/too large
    , decBadArg = 3     //incorrect argument is passed to decoder
  };

  Status_e  status; //decoding status
  TSLength  nbytes; //number of bytes processed (either succsefully
                    //decoded or utilized prior to error was detected)

  explicit DECResult(Status_e use_status = decBadEncoding, TSLength nb_decoded = 0)
    : status(use_status), nbytes(nb_decoded)
  { }

  bool isOk(void) const { return (status == decOk); }
  bool isOkRelaxed(void) const
  {
    return (status == decOk) || (status == decOkRelaxed);
  }
  bool isOk(bool use_relaxed) const
  {
    return (status == decOk) || (use_relaxed && (status == decOkRelaxed)); 
  }

  DECResult & operator+=(const DECResult & use_res)
  {
    status = use_res.status;
    nbytes += use_res.nbytes;
    return *this;
  }
};

} //asn1
} //eyeline

#endif /* __TRANSFER_SYNTAX_DEFS */

