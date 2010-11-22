#include <stdio.h>
#include <string.h>

#include "asn1c_gen/IntValue.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
#include "eyeline/utilx/hexdmp.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_INT_edgeValue(char* err_msg)
{
  IntValue_t value1= -1, value2= 0x7fffffff, value3=0xFFFFFFFF;
  asn_enc_rval_t retVal;
  char patternTrSyntax_1[MAX_PATTERN_LEN]={0};
  char patternTrSyntax_2[MAX_PATTERN_LEN]={0};
  char patternTrSyntax_3[MAX_PATTERN_LEN]={0};

  printf("test_INT_edgeValue:\t\t");
  retVal = der_encode(&asn_DEF_IntValue,
                      &value1, write_transfer_syntax, patternTrSyntax_1);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_INT_edgeValue:: IntValue=-1, trSyntax=%s, retVal=%ld\n", patternTrSyntax_1, retVal.encoded);

  retVal = der_encode(&asn_DEF_IntValue,
                      &value2, write_transfer_syntax, patternTrSyntax_2);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_INT_edgeValue:: IntValue=0x7fffffff, trSyntax=%s, retVal=%ld\n", patternTrSyntax_2, retVal.encoded);

  retVal = der_encode(&asn_DEF_IntValue,
                      &value3, write_transfer_syntax, patternTrSyntax_3);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_INT_edgeValue:: IntValue=0xffffffff, trSyntax=%s, retVal=%ld\n", patternTrSyntax_3, retVal.encoded);

  try {
    EncoderOfINTEGER encInt;
    encInt.setValue(-1);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encInt.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_INT_edgeValue:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_INT_edgeValue:: IntValue=-1, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax_1)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax_1, trSyntaxAsStr);
      fprintf(logfile, "test_INT_edgeValue:: expected value='%s', calculated value='%s'\n", patternTrSyntax_1, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT_edgeValue:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    EncoderOfINTEGER encInt;
    encInt.setValue(0x7fffffff);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encInt.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_INT_edgeValue:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_INT_edgeValue:: IntValue=0x7fffffff, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax_2)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax_2, trSyntaxAsStr);
      fprintf(logfile, "test_INT_edgeValue:: expected value='%s', calculated value='%s'\n", patternTrSyntax_2, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT_edgeValue:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    EncoderOfINTEGER encInt;
    encInt.setValue(0xffffffff);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encInt.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_INT_edgeValue:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_INT_edgeValue:: IntValue=0xffffffff, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax_3)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax_3, trSyntaxAsStr);
      fprintf(logfile, "test_INT_edgeValue:: expected value='%s', calculated value='%s'\n", patternTrSyntax_3, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT_edgeValue:: caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
