#include <stdio.h>
#include <string.h>

#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
#include "eyeline/utilx/hexdmp.hpp"
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

}}}}
