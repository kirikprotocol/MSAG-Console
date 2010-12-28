#include <stdio.h>
#include <string.h>

#include "asn1c_gen/BitStrValue.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeBITSTR.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeBITSTR.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "TestPatternsRegistry.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_BIT_STRING_enc(char* err_msg)
{
  printf("test_BIT_STRING_enc:\t\t\t");

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_BITSTRING", "F00FAA");
    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];
    EncoderOfBITSTR encBitStr;
    uint8_t buf4value[] = { 0xF0, 0x0F, 0xAA };
    encBitStr.setValue((TSLength)sizeof(buf4value)*8 - 2, buf4value);

    ENCResult encResult= encBitStr.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_BIT_STRING_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_BIT_STRING_enc:: BitStrValue, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_BIT_STRING_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_BIT_STRING_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

bool
test_BIT_STRING_dec(char* err_msg)
{
  printf("test_BIT_STRING_dec:\t\t\t");

  try {
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_BITSTRING", "F00FAA");
    fprintf(logfile, "test_BIT_STRING_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    DecoderOfBITSTR decBitStr;
    eyeline::util::BITArray_T<uint16_t, 1> expectedValue;
    decBitStr.setValue(expectedValue);

    DECResult decResult= decBitStr.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_BIT_STRING_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    const std::string expectedValueAsStr = utilx::hexdmp(expectedValue.getOcts(), expectedValue.numOcts());
    fprintf(logfile, "test_BIT_STRING_dec:: expectedValue=%s\n", expectedValueAsStr.c_str());
    if ( expectedValueAsStr != "F00FA8" )
      return false;

  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_BIT_STRING_dec::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
