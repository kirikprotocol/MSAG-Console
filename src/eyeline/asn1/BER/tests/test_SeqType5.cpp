#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <stdio.h>
#include <string.h>

#include "asn1c_gen/SeqType5.h"
#include "common.hpp"

#include "TestPatternsRegistry.hpp"
#include "SeqType5.hpp"
#include "dec/MDSeqType5.hpp"
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

  try {
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_SeqType5", "{a.case1=100,b.case2=true,c.case2=false,d.case1=200}");
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
    if (strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_SeqType5_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType5_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

// return true on success or false on error
bool
test_SeqType5_dec(char* err_msg)
{
  printf("test_SeqType5_dec:\t\t\t");
  try {
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_SeqType5", "{a.case1=100,b.case2=true,c.case2=false,d.case1=200}");

    fprintf(logfile, "test_SeqType5_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    dec::MDSeqType5 decSeqType5(TransferSyntax::ruleDER);
    SeqType5 expectedValue;
    decSeqType5.setValue(expectedValue);
    DECResult decResult= decSeqType5.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_SeqType5_dec:: DECResult.status=%d, nbytes=%u\n", decResult.status, decResult.nbytes);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d, nbytes=%u", decResult.status, decResult.nbytes);
      return false;
    }

    fprintf(logfile, "test_SeqType5_dec:: calculated value='{%d,%d,%d,%d}'\n", *expectedValue.a.case1().get(),
            *expectedValue.b.case2().get(), *expectedValue.c.case2().get(), *expectedValue.d.case1().get());
    if (*expectedValue.a.case1().get() != 100 ||
        *expectedValue.b.case2().get() != true ||
        *expectedValue.c.case2().get() != false ||
        *expectedValue.d.case1().get() != 200)
    {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='{100,true,false,200}', calculated value='{%d,%d,%d,%d}'", *expectedValue.a.case1().get(),
               *expectedValue.b.case2().get(), *expectedValue.c.case2().get(), *expectedValue.d.case2().get());
      fprintf(logfile, "expected value='{100,true,false,200}', calculated value='{%d,%d,%d,%d}'", *expectedValue.a.case1().get(),
              *expectedValue.b.case2().get(), *expectedValue.c.case2().get(), *expectedValue.d.case2().get());
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType5_dec::caught exception [%s]\n", ex.what());
    return false;
  }
  return true;
}

}}}}

