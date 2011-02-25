#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <stdio.h>
#include <string.h>

#include "eyeline/asn1/BER/tests/common.hpp"
#include "eyeline/asn1/BER/tests/dec/MDSeqType1.hpp"
#include "eyeline/asn1/BER/tests/enc/MESeqType1.hpp"
#include "eyeline/asn1/BER/tests/TestPatternsRegistry.hpp"

#include "eyeline/utilx/hexdmp.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_SeqType1_enc(char* err_msg)
{
  printf("test_SeqType1_enc:\t\t\t");

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_SeqType1", "{EE,11,NULL}");
    SeqType1 value_2;
    value_2.a = 0xEE;
    value_2.b = 0x11;
    enc::MESeqType1 encSeqType1;
    encSeqType1.setValue(value_2);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqType1.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqType1_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqType1_enc:: SeqType1Value={ 0xEE, 0x11, NULL}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_SeqType1_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType1_enc::caught exception [%s]\n", ex.what());
    return false;
  }
  return true;
}

// return true on success or false on error
bool
test_SeqType1_dec(char* err_msg)
{
  try {
    printf("test_SeqType1_dec:\t\t\t");
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_SeqType1", "{EE,11,NULL}");
    fprintf(logfile, "test_SeqType1_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    dec::MDSeqType1 decSeqType1;
    SeqType1 expectedValue;
    decSeqType1.setValue(expectedValue);
    DECResult decResult= decSeqType1.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_SeqType1_dec:: DECResult.status=%d, nbytes=%u\n", decResult.status, decResult.nbytes);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d, nbytes=%u", decResult.status, decResult.nbytes);
      return false;
    }

    fprintf(logfile, "test_SeqType1_dec:: expected value='{0x%x,0x%x}'\n", expectedValue.a, expectedValue.b);
    if (expectedValue.a != 0xEE || expectedValue.b != 0x11)
    {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='{0xEE,0x11}', calculated value='{0x%x,0x%x}'", expectedValue.a, expectedValue.b);
      fprintf(logfile, "test_SeqType1_dec:: expected value='{0xEE,0x11}', calculated value='{0x%x,0x%x}'", expectedValue.a, expectedValue.b);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType1_dec:: caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
