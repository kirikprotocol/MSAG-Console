/* ************************************************************************* *
 * ASN.1 transfer syntaxes definition.
 * ************************************************************************* */
#ifndef __TRANSFER_SYNTAX_DEFS
#ident "@(#)$Id$"
#define __TRANSFER_SYNTAX_DEFS

#include <inttypes.h>

namespace eyeline {
namespace asn1 {

typedef uint32_t  TSLength; //type for overall encoding length
typedef uint16_t  TSELength; //type for encoding element length

struct TransferSyntax {
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
};

struct TSGroupBER { //octet aligned encodings
  enum Rule_e {
      ruleBER = TransferSyntax::ruleBER //ruleBasic
    , ruleDER = TransferSyntax::ruleDER //ruleDistinguished
    , ruleCER = TransferSyntax::ruleCER //ruleCanonical
  };
};

struct TSGroupPER { 
  enum Rule_e {
    //octet aligned encodings
      rulePER   = TransferSyntax::rulePER   //rulePacked_aligned
    , ruleCPER  = TransferSyntax::ruleCPER  //rulePacked_aligned_canonical
    //bit aligned encodings:
    , ruleUPER  = TransferSyntax::ruleUPER  //rulePacked_unaligned
    , ruleCUPER = TransferSyntax::ruleCUPER //rulePacked_unaligned_canonical
  };
};

struct TSGroupXER { //octet aligned encodings
  enum Rule_e {
      ruleXER = TransferSyntax::ruleXER   //ruleXml
    , ruleCXER = TransferSyntax::ruleCXER //ruleXml_canonical
    , ruleEXER = TransferSyntax::ruleEXER //ruleXml_extended
  };
};

struct ENCResult {
  enum Status_e {
    encErrInternal = -3 //internal encoder error
    , encMoreMem = -2   //not enough memory for resulted encoding
    , encOk = 0
    , encBadVal = 1     //invalid/illegal value is to encode
    , encBadArg = 2     //incorrect argument is passed to encoder
  };

  Status_e  status; //encoding status
  TSLength  nbytes; //number of bytes successfully encoded (even
                    //if encMoreMem indicated)

  ENCResult(Status_e use_status = encBadVal, TSLength nb_encoded = 0)
    : status(encBadVal), nbytes(nb_encoded)
  { }
};

struct DECResult {
  enum Status_e {
    decErrInternal = -3 //internal decoder error
    , decMoreInput = -2 //insufficient input encoding to complete
    , decBadEncoding = -1 //corrupted input encoding
    , decOk = 0
    , decBadVal = 1     //value that is decoded is invalid/illegal
    , decBadArg = 2     //incorrect argument is passed to decoder
  };

  Status_e  status; //decoding status
  TSLength  nbytes; //number of bytes processed (either succsefully
                    //decoded or utilized prior to error was detected)

  DECResult(Status_e use_status = decBadEncoding, TSLength nb_decoded = 0)
    : status(use_status), nbytes(nb_decoded)
  { }
};

} //asn1
} //eyeline

#endif /* __TRANSFER_SYNTAX_DEFS */

