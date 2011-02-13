#include <stdio.h>
#include <string.h>

#include "asn1c_gen/ChoiceType2.h"
#include "common.hpp"

#include "TestPatternsRegistry.hpp"
#include "ChoiceType2.hpp"
#include "enc/MEChoiceType2.hpp"
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
  {
    ChoiceType2_t value;

    memset(&value, 0, sizeof(value));
    asn_enc_rval_t retVal;
    char patternTrSyntax[MAX_PATTERN_LEN]={0};

    value.choice.case1= 100;
    value.present= ChoiceType2_PR_case1;
    retVal = der_encode(&asn_DEF_ChoiceType2,
                        &value, write_transfer_syntax, patternTrSyntax);
    if (retVal.encoded == -1)
      return false;

    fprintf(logfile, "test_ChoiceType2_enc:: ChoiceType2_t={case1=100}, trSyntax=%s, retVal=%ld\n",
            patternTrSyntax, retVal.encoded);
    TestPatternsRegistry::getInstance().insertResultPattern("test_ChoiceType2", "{case1=100}", patternTrSyntax);

    try {
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
      if (strcmp(trSyntaxAsStr, patternTrSyntax)) {
        snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax, trSyntaxAsStr);
        fprintf(logfile, "test_ChoiceType2_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax, trSyntaxAsStr);
        return false;
      }
    } catch (std::exception& ex) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
      fprintf(logfile, "test_ChoiceType2_enc::caught exception [%s]\n", ex.what());
      return false;
    }
  }

  {
    ChoiceType2_t value;

    memset(&value, 0, sizeof(value));
    asn_enc_rval_t retVal;
    char patternTrSyntax[MAX_PATTERN_LEN]={0};

    value.choice.case2= true;
    value.present= ChoiceType2_PR_case2;
    retVal = der_encode(&asn_DEF_ChoiceType2,
                        &value, write_transfer_syntax, patternTrSyntax);
    if (retVal.encoded == -1)
      return false;

    fprintf(logfile, "test_ChoiceType2_enc:: ChoiceType2_t={case2=true}, trSyntax=%s, retVal=%ld\n",
        patternTrSyntax, retVal.encoded);
    TestPatternsRegistry::getInstance().insertResultPattern("test_ChoiceType2", "{case2=true}", patternTrSyntax);
  }
  return true;
}

}}}}
