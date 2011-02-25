#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <stdio.h>
#include <string.h>

#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "SeqType2.hpp"
#include "dec/MDSeqType2.hpp"
#include "enc/MESeqType2.hpp"
#include "TestPatternsRegistry.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_SeqType2_enc(char* err_msg)
{
  printf("test_SeqType2_enc:\t\t\t");

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_SeqType2", "{AA,BB,CC}");
    SeqType2 copy_value;
    copy_value.a = 0xAA;
    copy_value.b.init() = 0xBB;
    copy_value.c.init() = 0xCC;

    enc::MESeqType2 encSeqType2;
    encSeqType2.setValue(copy_value);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqType2.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqType2_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqType2_enc:: SeqType2Value={a=0xAA,b=0xBB,b=0xCC}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_SeqType2_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType2_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_SeqType2", "{AA,NULL,NULL}");

    SeqType2 copy_value;
    copy_value.a = 0xAA;

    enc::MESeqType2 encSeqType2;
    encSeqType2.setValue(copy_value);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqType2.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqType2_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqType2_enc:: SeqType2Value={ 0xAA}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_SeqType2_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType2_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

// return true on success or false on error
bool
test_SeqType2_dec(char* err_msg)
{
  try {
    printf("test_SeqType2_dec:\t\t\t");
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_SeqType2", "{AA,BB,CC}");
    fprintf(logfile, "test_SeqType2_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    dec::MDSeqType2 decSeqType2;
    SeqType2 expectedValue;
    decSeqType2.setValue(expectedValue);
    DECResult decResult= decSeqType2.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_SeqType2_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    if (!expectedValue.b.get() || !expectedValue.c.get()) {
      fprintf(logfile, "test_SeqType2_dec:: expected value='{0xAA,0xBB}', calculated value='{,");
      if (expectedValue.b.get())
        fprintf(logfile, "0x%x,'", *expectedValue.b.get());
      else
        fprintf(logfile, "NULL,'");

      if (expectedValue.c.get())
        fprintf(logfile, "0x%x'}\n", *expectedValue.c.get());
      else
        fprintf(logfile, "NULL}'\n");
    } else
      fprintf(logfile, "test_SeqType2_dec:: expected value='{0xAA,0xBB}', calculated value='{0x%x,0x%x,0x%x}'\n", expectedValue.a, *expectedValue.b.get(), *expectedValue.c.get());

    if (expectedValue.a != 0xAA || !expectedValue.b.get() || *expectedValue.b.get() != 0xBB ||
        !expectedValue.c.get() || *expectedValue.c.get() != 0xCC)
    {
      int off= snprintf(err_msg, sizeof(err_msg),  "test_SeqType2_dec:: expected value='{0xAA,0xBB}', calculated value='{");
      if (expectedValue.b.get())
        off+= snprintf(err_msg + off, sizeof(err_msg) - off, "0x%x,'", *expectedValue.b.get());
      else
        off+= snprintf(err_msg + off, sizeof(err_msg) - off, "NULL,'");

      if (expectedValue.c.get())
        off+= snprintf(err_msg, sizeof(err_msg), "0x%x'}\n", *expectedValue.c.get());
      else
        off+= snprintf(err_msg, sizeof(err_msg), "NULL}'\n");

      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType2_dec:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_SeqType2", "{AA,NULL,NULL}");
    fprintf(logfile, "test_SeqType2_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    dec::MDSeqType2 decSeqType2;
    SeqType2 expectedValue;
    decSeqType2.setValue(expectedValue);
    DECResult decResult= decSeqType2.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_SeqType2_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    if (!expectedValue.b.get())
      fprintf(logfile, "test_SeqType2_dec:: expected value='{0xAA,NULL}', calculated value='{0x%x,NULL}'\n", expectedValue.a);
    else
      fprintf(logfile, "test_SeqType2_dec:: expected value='{0xAA,NULL}', calculated value='{0x%x,0x%x}'\n", expectedValue.a, *expectedValue.b.get());

    if (expectedValue.a != 0xAA || expectedValue.b.get())
    {
      if (expectedValue.b.get())
        snprintf(err_msg, sizeof(err_msg), "test_SeqType2_dec:: expected value='{0xAA,NULL}', calculated value='{0x%x,NULL}'\n", expectedValue.a);
      else
        snprintf(err_msg, sizeof(err_msg), "test_SeqType2_dec:: expected value='{0xAA,NULL}', calculated value='{0x%x,0x%x}'\n", expectedValue.a, *expectedValue.b.get());
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType2_dec:: caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
