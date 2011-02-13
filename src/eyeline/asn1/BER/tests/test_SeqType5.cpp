#include <stdio.h>
#include <string.h>

#include "asn1c_gen/SeqType5.h"
#include "common.hpp"

#include "TestPatternsRegistry.hpp"
#include "SeqType5.hpp"
#include "enc/MESeqType5.hpp"
#include "eyeline/utilx/hexdmp.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_SeqType5_enc(char* err_msg)
{
  printf("test_SeqType5_enc:\t\t\t");

  SeqType5_t value;

  memset(&value, 0, sizeof(value));
  asn_enc_rval_t retVal;
  char patternTrSyntax[MAX_PATTERN_LEN]={0};

  value.a.choice.case1= 100;
  value.a.present= ChoiceType1_PR_case1;
  value.b.choice.case2= true;
  value.b.present= ChoiceType2_PR_case2;
  value.c.choice.case2= false;
  value.c.present= ChoiceType1_PR_case2;
  value.d.choice.case1= 200;
  value.d.present= ChoiceType2_PR_case1;

  retVal = der_encode(&asn_DEF_SeqType5,
                      &value, write_transfer_syntax, patternTrSyntax);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_SeqType5_enc:: SeqType5_t={a.case1=100,b.case2=true,c.case2=false,d.case1=200}, trSyntax=%s, retVal=%ld\n",
          patternTrSyntax, retVal.encoded);
  TestPatternsRegistry::getInstance().insertResultPattern("test_SeqType5", "{a.case1=100,b.case2=true,c.case2=false,d.case1=200}", patternTrSyntax);

  try {
    SeqType5 value_2;
    value_2.a.case1().init() = 100;
    value_2.b.case2().init() = true;
    value_2.c.case2().init() = false;
    value_2.d.case1().init() = 200;

    enc::MESeqType5 encSeqType5;
    encSeqType5.setValue(value_2);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqType5.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqType5_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqType5_enc:: SeqType5Value={a.case1=100,b.case2=true,c.case2=false,d.case1=200}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax, trSyntaxAsStr);
      fprintf(logfile, "test_SeqType5_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType5_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}

