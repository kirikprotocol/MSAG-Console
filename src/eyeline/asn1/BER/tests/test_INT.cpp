#include <stdio.h>
#include <string.h>

#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "TestPatternsRegistry.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_INT_enc(char* err_msg)
{
  printf("test_INT_enc:\t\t\t\t");

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_INT", "100");
    EncoderOfINTEGER encInt;
    encInt.setValue(100);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encInt.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_INT_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_INT_enc:: IntValue=100, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_INT_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT_enc:: caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

// return true on success or false on error
bool
test_INT_dec(char* err_msg)
{
  try {
    printf("test_INT_dec:\t\t\t\t");
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_INT", "100");
    fprintf(logfile, "test_INT_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    DecoderOfINTEGER decInt;
    int expectedValue=0;
    decInt.setValue(expectedValue);

    DECResult decResult= decInt.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_INT_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    fprintf(logfile, "test_INT_dec:: expectedValue=%d\n", expectedValue);
    if ( expectedValue!= 100 )
      return false;

    return true;
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT_enc:: caught exception [%s]\n", ex.what());
    return false;
  }
}

}}}}
