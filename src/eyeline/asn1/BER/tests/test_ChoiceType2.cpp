#include <stdio.h>
#include <string.h>

#include "common.hpp"

#include "TestPatternsRegistry.hpp"
#include "ChoiceType2.hpp"
#include "enc/MEChoiceType2.hpp"
#include "dec/MDChoiceType2.hpp"
#include "eyeline/utilx/hexdmp.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_ChoiceType2_enc(char* err_msg)
{
  printf("test_ChoiceType2_enc:\t\t\t");

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_ChoiceType2", "{case1=100}");
    ChoiceType2 value_1;
    value_1.case1().init() = 100;
    enc::MEChoiceType2 encChoiceType1;
    encChoiceType1.setValue(value_1);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encChoiceType1.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_ChoiceType2_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_ChoiceType2_enc:: ChoiceType2Value={200}, trSyntax=%s\n",
        trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_ChoiceType2_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_ChoiceType2_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

// return true on success or false on error
bool
test_ChoiceType2_dec(char* err_msg)
{
  try {
    printf("test_ChoiceType2_dec:\t\t\t");
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_ChoiceType2", "{case1=100}");
    fprintf(logfile, "test_ChoiceType2_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    dec::MDChoiceType2 decChoiceType2;
    ChoiceType2 expectedValue;
    decChoiceType2.setValue(expectedValue);
    DECResult decResult= decChoiceType2.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_ChoiceType2_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    fprintf(logfile, "test_ChoiceType2_dec:: calculated value='{case1=%d}'\n", *expectedValue.case1().get());
    if (*expectedValue.case1().get() != 100 )
    {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='{case1=100}', calculated value='{%d}'", *expectedValue.case1().get());
      fprintf(logfile, "expected value='{case1=100}', calculated value='{%d}'", *expectedValue.case1().get());
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_ChoiceType2_dec:: caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
