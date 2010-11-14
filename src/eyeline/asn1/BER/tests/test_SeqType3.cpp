#include <stdio.h>
#include <string.h>

#include "asn1c_gen/SeqType3.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "SeqType3.hpp"
#include "enc/MESeqType3.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_SeqType3(char* err_msg)
{
  SeqType3_t value;
  asn_enc_rval_t retVal;
  char patternTrSyntax_1[MAX_PATTERN_LEN]={0};

  value.a= 0xAA;
  value.b= new long(0xBB);

  printf("test_SeqType3:\t\t\t");
  retVal = der_encode(&asn_DEF_SeqType3,
                      &value, write_transfer_syntax, patternTrSyntax_1);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_SeqType3:: SeqType3_t={a=0xAA,b=0xBB}, trSyntax=%s, retVal=%ld\n",
          patternTrSyntax_1, retVal.encoded);

  SeqType3_t value_2;
  char patternTrSyntax_2[MAX_PATTERN_LEN]={0};

  value_2.a= 0xAA;
  value_2.b= NULL;

  retVal = der_encode(&asn_DEF_SeqType3,
                      &value_2, write_transfer_syntax, patternTrSyntax_2);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_SeqType3:: SeqType3_t={a=0xAA}, trSyntax=%s, retVal=%ld\n",
          patternTrSyntax_2, retVal.encoded);

  try {
    SeqType3 copy_value;
    copy_value.a = value.a;
    copy_value.b.init() = *value.b;
    enc::MESeqType3 encSeqType3;
    encSeqType3.setValue(copy_value);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqType3.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqType3:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqType3:: SeqType3Value={a=0xAA,b=0xBB}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax_1)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax_1, trSyntaxAsStr);
      fprintf(logfile, "test_SeqType3:: expected value='%s', calculated value='%s'\n", patternTrSyntax_1, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType3::caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    SeqType3 copy_value;
    copy_value.a = value_2.a;

    enc::MESeqType3 encSeqType3;
    encSeqType3.setValue(copy_value);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqType3.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqType3:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqType3:: SeqType3Value={ 0xAA}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax_2)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax_2, trSyntaxAsStr);
      fprintf(logfile, "test_SeqType3:: expected value='%s', calculated value='%s'\n", patternTrSyntax_2, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType3::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
