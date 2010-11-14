#include <stdio.h>
#include <string.h>

#include "asn1c_gen/SeqType4.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "SeqType4.hpp"
#include "enc/MESeqType4.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_SeqType4(char* err_msg)
{
  SeqType4_t value;
  asn_enc_rval_t retVal;
  char patternTrSyntax_1[MAX_PATTERN_LEN]={0};

  value.a= 0xAA;
  value.b= new long(0x77);
  value.c= new long(0xDD);

  printf("test_SeqType4:\t\t\t");
  retVal = der_encode(&asn_DEF_SeqType4,
                      &value, write_transfer_syntax, patternTrSyntax_1);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_SeqType4:: SeqType4_t={a=0xAA,b=0x77,c=0xDD}, trSyntax=%s, retVal=%ld\n",
          patternTrSyntax_1, retVal.encoded);

  SeqType4_t value_2;
  char patternTrSyntax_2[MAX_PATTERN_LEN]={0};

  value_2.a= 0xAA;
  value_2.b= NULL;
  value_2.c= new long(0xEE);

  retVal = der_encode(&asn_DEF_SeqType4,
                      &value_2, write_transfer_syntax, patternTrSyntax_2);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_SeqType4:: SeqType4_t={a=0xAA,c=0xEE}, trSyntax=%s, retVal=%ld\n",
          patternTrSyntax_2, retVal.encoded);

  try {
    SeqType4 copy_value;
    copy_value.a = value.a;
    copy_value.b.init() = *value.b;
    copy_value.c.init() = *value.c;
    enc::MESeqType4 encSeqType4;
    encSeqType4.setValue(copy_value);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqType4.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqType4:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqType4:: SeqType4Value={a=0xAA,b=0x77,c=0xDD}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax_1)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax_1, trSyntaxAsStr);
      fprintf(logfile, "test_SeqType4:: expected value='%s', calculated value='%s'\n", patternTrSyntax_1, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType4::caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    SeqType4 copy_value;
    copy_value.a = value_2.a;
    copy_value.c.init() = *value_2.c;

    enc::MESeqType4 encSeqType4;
    encSeqType4.setValue(copy_value);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqType4.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqType4:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqType4:: SeqType4Value={a=0xAA,c=0xEE}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax_2)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax_2, trSyntaxAsStr);
      fprintf(logfile, "test_SeqType4:: expected value='%s', calculated value='%s'\n", patternTrSyntax_2, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType4::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
