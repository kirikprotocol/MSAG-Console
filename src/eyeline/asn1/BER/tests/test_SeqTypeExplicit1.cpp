#include <stdio.h>
#include <string.h>

#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "dec/MDSeqTypeExplicit1.hpp"
#include "enc/MESeqTypeExplicit1.hpp"
#include "SeqType1.hpp"
#include "TestPatternsRegistry.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_SeqTypeExplicit1_enc(char* err_msg)
{
  printf("test_SeqTypeExplicit1_enc:\t\t");

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_SeqTypeExplicit1", "{EE,11}");

    SeqType1 value_2;
    value_2.a = 0xEE;
    value_2.b = 0x11;
    enc::MESeqTypeExplicit1 encSeqTypeExplicit1;
    encSeqTypeExplicit1.setValue(value_2);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqTypeExplicit1.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqTypeExplicit1_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqTypeExplicit1_enc:: SeqType1ExplicitValue={ 0xEE, 0x11, NULL}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_SeqTypeExplicit1_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqTypeExplicit1_enc::caught exception [%s]\n", ex.what());
    return false;
  }
  return true;
}

bool
test_SeqTypeExplicit1_dec(char* err_msg)
{
  printf("test_SeqTypeExplicit1_dec:\t\t");

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_SeqTypeExplicit1", "{EE,11}");
    fprintf(logfile, "test_SeqTypeExplicit1_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    dec::MDSeqTypeExplicit1 decSeqTypeExplicit1;
    SeqType1 expectedValue;
    decSeqTypeExplicit1.setValue(expectedValue);
    DECResult decResult= decSeqTypeExplicit1.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_SeqTypeExplicit1_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk)
    {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    fprintf(logfile, "test_SeqTypeExplicit1_dec:: expected value='{0xEE,0x11}', calculated value='{0x%x,0x%x}'\n", expectedValue.a, expectedValue.b);

    if (expectedValue.a != 0xEE || expectedValue.b != 0x11)
    {
      snprintf(err_msg, sizeof(err_msg), "test_SeqTypeExplicit1_dec:: expected value='{0xEE,0x11}', calculated value='{0x%x,0x%x}'\n", expectedValue.a, expectedValue.b);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqTypeExplicit1_dec:: caught exception [%s]\n", ex.what());
    return false;
  }
  return true;
}

}}}}
