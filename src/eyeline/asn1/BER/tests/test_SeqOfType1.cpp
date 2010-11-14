#include <stdio.h>
#include <string.h>

#include "asn1c_gen/SeqOfType1.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "enc/MESeqOfType1.hpp"
#include "SeqOfType1.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_SeqOfType1(char* err_msg)
{
  SeqOfType1_t value;
  asn_enc_rval_t retVal;
  char patternTrSyntax[MAX_PATTERN_LEN]={0};

  SeqType1_t   element1;
  element1.a= 0xEE;
  element1.b= 0x11;
  asn_sequence_add(&value, &element1);

  SeqType1_t   element2;
  element2.a= 0x33;
  element2.b= 0xCC;
  asn_sequence_add(&value, &element2);

  SeqType1_t element3;
  element3.a= 0x55;
  element3.b= 0xFF;
  asn_sequence_add(&value, &element3);

  SeqType1_t element4;
  element4.a= 0xAA;
  element4.b= 0x77;
  asn_sequence_add(&value, &element4);

  printf("test_SeqOfType1:\t\t");
  retVal = der_encode(&asn_DEF_SeqOfType1,
                      &value, write_transfer_syntax, patternTrSyntax);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_SeqOfType1:: SeqOfType1_t={{a=0xEE,b=0x11,c=NULL},{a=0x33,b=0xCC,c=NULL},{a=0x55,b=0xFF,c=NULL},{a=0xAA,b=0x77,c=NULL}} trSyntax=%s, retVal=%ld\n",
          patternTrSyntax, retVal.encoded);

  try {
    SeqOfType1 value_2;

    value_2.push_back(SeqType1(0xEE, 0x11));
    value_2.push_back(SeqType1(0x33, 0xCC));
    value_2.push_back(SeqType1(0x55, 0xFF));
    value_2.push_back(SeqType1(0xAA, 0x77));

    enc::MESeqOfType1 encSeqOfType1;
    encSeqOfType1.setValue(value_2);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqOfType1.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqOfType1:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqOfType1:: SeqOfType1Value={{a=0xEE,b=0x11,c=NULL},{a=0x33,b=0xCC,c=NULL},{a=0x55,b=0xFF,c=NULL},{a=0xAA,b=0x77,c=NULL}}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax, trSyntaxAsStr);
      fprintf(logfile, "test_SeqOfType1:: expected value='%s', calculated value='%s'\n", patternTrSyntax, trSyntaxAsStr);
      return false;
    }

  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqOfType1::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
