#include <stdio.h>
#include <string.h>

#include "asn1c_gen/BoolValue.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeBOOL.hpp"
#include "eyeline/utilx/hexdmp.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_BOOL(char* err_msg)
{
  BoolValue_t value= 1;
  asn_enc_rval_t retVal;
  char patternTrSyntax[MAX_PATTERN_LEN]={0};

  printf("test_BOOL:\t\t\t");
  retVal = der_encode(&asn_DEF_BoolValue,
                      &value, write_transfer_syntax, patternTrSyntax);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_BOOL:: BoolValue=1, trSyntax=%s, retVal=%ld\n", patternTrSyntax, retVal.encoded);

  EncoderOfBOOL encBool;
  encBool.setValue(true);
  try {
    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encBool.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_BOOL:: BoolValue=1, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax, trSyntaxAsStr);
      fprintf(logfile, "test_BOOL:: expected value='%s', calculated value='%s'\n", patternTrSyntax, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_BOOL:: caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
