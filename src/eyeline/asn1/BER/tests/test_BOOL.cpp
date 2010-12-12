#include <stdio.h>
#include <string.h>

#include "asn1c_gen/BoolValue.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeBOOL.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeBOOL.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "TestPatternsRegistry.hpp"
#include "eyeline/utilx/hexdmp.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_BOOL_enc(char* err_msg)
{
  BoolValue_t value= 1;
  asn_enc_rval_t retVal;
  char patternTrSyntax[MAX_PATTERN_LEN]={0};

  printf("test_BOOL_enc:\t\t\t\t");
  retVal = der_encode(&asn_DEF_BoolValue,
                      &value, write_transfer_syntax, patternTrSyntax);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_BOOL_enc:: BoolValue=1, trSyntax=%s, retVal=%ld\n", patternTrSyntax, retVal.encoded);
  TestPatternsRegistry::getInstance().insertResultPattern("test_BOOL", "true", patternTrSyntax);

  try {
    EncoderOfBOOL encBool;
    encBool.setValue(true);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encBool.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_BOOL_enc:: BoolValue=1, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax, trSyntaxAsStr);
      fprintf(logfile, "test_BOOL_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_BOOL_enc:: caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

// return true on success or false on error
bool
test_BOOL_dec(char* err_msg)
{
  try {
    printf("test_BOOL_dec:\t\t\t\t");
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_BOOL", "true");
    fprintf(logfile, "test_BOOL_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    DecoderOfBOOL decBool;
    bool expectedValue= false;
    decBool.setValue(expectedValue);
    DECResult decResult= decBool.decode(patternTrSyntaxBin, patternLen);

    fprintf(logfile, "test_BOOL_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    fprintf(logfile, "test_BOOL_dec:: expectedValue=%d\n", expectedValue);
    if ( expectedValue!= true )
      return false;
    return true;
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_BOOL_enc:: caught exception [%s]\n", ex.what());
    return false;
  }
}

}}}}
