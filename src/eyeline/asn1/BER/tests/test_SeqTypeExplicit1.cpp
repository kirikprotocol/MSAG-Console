#include <stdio.h>
#include <string.h>

#include "asn1c_gen/SeqTypeExplicit1.h"
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
  SeqTypeExplicit1_t value;
  asn_enc_rval_t retVal;
  char patternTrSyntax[MAX_PATTERN_LEN]={0};

  value.a= 0xEE;
  value.b= 0x11;

  printf("test_SeqTypeExplicit1_enc:\t\t");
  retVal = der_encode(&asn_DEF_SeqTypeExplicit1,
                      &value, write_transfer_syntax, patternTrSyntax);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_SeqTypeExplicit1_enc:: SeqTypeExplicit1_t={a=0xEE,b=0x11,c=NULL}, trSyntax=%s, retVal=%ld\n",
          patternTrSyntax, retVal.encoded);
  TestPatternsRegistry::getInstance().insertResultPattern("test_SeqTypeExplicit1", "{EE,11}", patternTrSyntax);

  try {
    SeqType1 value_2;
    value_2.a = value.a;
    value_2.b = value.b;
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
    if (strcmp(trSyntaxAsStr, patternTrSyntax)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax, trSyntaxAsStr);
      fprintf(logfile, "test_SeqTypeExplicit1_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax, trSyntaxAsStr);
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
