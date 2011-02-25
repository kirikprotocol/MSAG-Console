#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <stdio.h>
#include <string.h>

#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "SeqType3.hpp"
#include "dec/MDSeqType3.hpp"
#include "enc/MESeqType3.hpp"
#include "TestPatternsRegistry.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_SeqType3_enc(char* err_msg)
{
  printf("test_SeqType3_enc:\t\t\t");

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_SeqType3", "{AA,BB}");

    SeqType3 copy_value;
    copy_value.a = 0xAA;
    copy_value.b.init() = 0xBB;
    enc::MESeqType3 encSeqType3;
    encSeqType3.setValue(copy_value);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqType3.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqType3_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqType3_enc:: SeqType3Value={a=0xAA,b=0xBB}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_SeqType3_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType3_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_SeqType3", "{AA,NULL}");
    SeqType3 copy_value;
    copy_value.a = 0xAA;

    enc::MESeqType3 encSeqType3;
    encSeqType3.setValue(copy_value);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqType3.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqType3_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqType3_enc:: SeqType3Value={ 0xAA}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_SeqType3_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType3_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

// return true on success or false on error
bool
test_SeqType3_dec(char* err_msg)
{
  try {
    printf("test_SeqType3_dec:\t\t\t");
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_SeqType3", "{AA,BB}");
    fprintf(logfile, "test_SeqType3_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    dec::MDSeqType3 decSeqType3;
    SeqType3 expectedValue;
    decSeqType3.setValue(expectedValue);
    DECResult decResult= decSeqType3.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_SeqType3_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    if (!expectedValue.b.get())
      fprintf(logfile, "test_SeqType3_enc:: expected value='{0xAA,0xBB}', calculated value='{0x%x,NULL}'\n", expectedValue.a);
    else
      fprintf(logfile, "test_SeqType3_enc:: expected value='{0xAA,0xBB}', calculated value='{0x%x,0x%x}'\n", expectedValue.a, *expectedValue.b.get());

    if (expectedValue.a != 0xAA || !expectedValue.b.get() || *expectedValue.b.get() != 0xBB)
    {
      if (!expectedValue.b.get())
        snprintf(err_msg, sizeof(err_msg), "test_SeqType2_enc:: expected value='{0xAA,0xBB}', calculated value='{0x%x,NULL}'\n", expectedValue.a);
      else
        snprintf(err_msg, sizeof(err_msg), "test_SeqType2_enc:: expected value='{0xAA,0xBB}', calculated value='{0x%x,0x%x}'\n", expectedValue.a, *expectedValue.b.get());
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType2_dec:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_SeqType3", "{AA,NULL}");
    fprintf(logfile, "test_SeqType3_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    dec::MDSeqType3 decSeqType3;
    SeqType3 expectedValue;
    decSeqType3.setValue(expectedValue);
    DECResult decResult= decSeqType3.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_SeqType2_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    if (!expectedValue.b.get())
      fprintf(logfile, "test_SeqType3_enc:: expected value='{0xAA,NULL}', calculated value='{0x%x,NULL}'\n", expectedValue.a);
    else
      fprintf(logfile, "test_SeqType3_enc:: expected value='{0xAA,NULL}', calculated value='{0x%x,0x%x}'\n", expectedValue.a, *expectedValue.b.get());

    if (expectedValue.a != 0xAA || expectedValue.b.get())
    {
      if (expectedValue.b.get())
        snprintf(err_msg, sizeof(err_msg), "test_SeqType3_enc:: expected value='{0xAA,NULL}', calculated value='{0x%x,NULL}'\n", expectedValue.a);
      else
        snprintf(err_msg, sizeof(err_msg), "test_SeqType3_enc:: expected value='{0xAA,NULL}', calculated value='{0x%x,0x%x}'\n", expectedValue.a, *expectedValue.b.get());
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType3_dec:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    // get encoding for SeqType2 containing three fields of INTEGER and try to decode it using extensible sequence SeqType3
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_SeqType2", "{AA,BB,CC}");
    fprintf(logfile, "test_SeqType3_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    dec::MDSeqType3 decSeqType3;
    SeqType3 expectedValue;
    decSeqType3.setValue(expectedValue);
    DECResult decResult= decSeqType3.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_SeqType3_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    if (!expectedValue.b.get())
      fprintf(logfile, "test_SeqType3_enc:: expected value='{0xAA,0xBB}', calculated value='{0x%x,NULL}'\n", expectedValue.a);
    else
      fprintf(logfile, "test_SeqType3_enc:: expected value='{0xAA,0xBB}', calculated value='{0x%x,0x%x}'\n", expectedValue.a, *expectedValue.b.get());

    if (expectedValue.a != 0xAA || !expectedValue.b.get() || *expectedValue.b.get() != 0xBB)
    {
      if (expectedValue.b.get())
        snprintf(err_msg, sizeof(err_msg), "test_SeqType3_enc:: expected value='{0xAA,0xBB}', calculated value='{0x%x,NULL}'\n", expectedValue.a);
      else
        snprintf(err_msg, sizeof(err_msg), "test_SeqType3_enc:: expected value='{0xAA,0xBB}', calculated value='{0x%x,0x%x}'\n", expectedValue.a, *expectedValue.b.get());
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType3_dec:: caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
