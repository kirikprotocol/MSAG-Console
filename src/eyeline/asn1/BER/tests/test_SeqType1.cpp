#include <stdio.h>
#include <string.h>

#include "asn1c_gen/SeqType1.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "dec/MDSeqType1.hpp"
#include "enc/MESeqType1.hpp"
#include "SeqType1.hpp"
#include "TestPatternsRegistry.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_SeqType1_enc(char* err_msg)
{
  SeqType1_t value;
  asn_enc_rval_t retVal;
  char patternTrSyntax[MAX_PATTERN_LEN]={0};

  value.a= 0xEE;
  value.b= 0x11;

  printf("test_SeqType1_enc:\t\t\t");
  retVal = der_encode(&asn_DEF_SeqType1,
                      &value, write_transfer_syntax, patternTrSyntax);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_SeqType1_enc:: SeqType1_t={a=0xEE,b=0x11,c=NULL}, trSyntax=%s, retVal=%ld\n",
          patternTrSyntax, retVal.encoded);
  TestPatternsRegistry::getInstance().insertResultPattern("test_SeqType1", "{EE,11}", patternTrSyntax);

  try {
    SeqType1 value_2;
    value_2.a = value.a;
    value_2.b = value.b;
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
    if (strcmp(trSyntaxAsStr, patternTrSyntax)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax, trSyntaxAsStr);
      fprintf(logfile, "test_SeqType1_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax, trSyntaxAsStr);
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
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_SeqType1", "{EE,11}");
    fprintf(logfile, "test_SeqType1_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    dec::MDSeqType1 decSeqType1;
    SeqType1 expectedValue;
    decSeqType1.setValue(expectedValue);
    DECResult decResult= decSeqType1.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_SeqType1_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    fprintf(logfile, "test_SeqType1_enc:: expected value='{0x%u,0x%u}'\n", expectedValue.a, expectedValue.b);
    if (expectedValue.a != 0xEE || expectedValue.b != 0x11)
    {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='{0xEE,0x11}', calculated value='{0x%u,0x%u}'", expectedValue.a, expectedValue.b);
      fprintf(logfile, "test_SeqType1_dec:: expected value='{0xEE,0x11}', calculated value='{0x%u,0x%u}'", expectedValue.a, expectedValue.b);
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
