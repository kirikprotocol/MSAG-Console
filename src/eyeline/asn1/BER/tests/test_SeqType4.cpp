#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <stdio.h>
#include <string.h>

#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "SeqType4.hpp"
#include "enc/MESeqType4.hpp"
#include "dec/MDSeqType4.hpp"
#include "TestPatternsRegistry.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_SeqType4_enc(char* err_msg)
{
  printf("test_SeqType4_enc:\t\t\t");

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_SeqType4", "{AA,77,DD}");
    SeqType4 copy_value;
    copy_value.a = 0xAA;
    copy_value.b.init() = 0x77;
    copy_value.c.init() = 0xDD;
    enc::MESeqType4 encSeqType4;
    encSeqType4.setValue(copy_value);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqType4.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqType4_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqType4_enc:: SeqType4Value={a=0xAA,b=0x77,c=0xDD}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_SeqType4_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType4_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_SeqType4", "{AA,NULL,EE}");
    SeqType4 copy_value;
    copy_value.a = 0xAA;
    copy_value.c.init() = 0xEE;

    enc::MESeqType4 encSeqType4;
    encSeqType4.setValue(copy_value);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqType4.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqType4_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqType4_enc:: SeqType4Value={a=0xAA,c=0xEE}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_SeqType4_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType4_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

bool
test_SeqType4_dec(char* err_msg)
{
  printf("test_SeqType4_dec:\t\t\t");

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_SeqType4", "{AA,77,DD}");

    fprintf(logfile, "test_SeqType4_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    dec::MDSeqType4 decSeqType4;
    SeqType4 value;
    decSeqType4.setValue(value);
    DECResult decResult= decSeqType4.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_SeqType4_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    fprintf(logfile, "test_SeqType4_dec:: calculated value='{0x%x,0x%x,0x%x}'\n", value.a, (value.b.get() ? *value.b.get() : 0), (value.c.get() ? *value.c.get() : 0));
    if (value.a != 0xAA ||
        !value.b.get() ||
        *value.b.get() != 0x77 ||
        !value.c.get() ||
        *value.c.get() != 0xDD )
    {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='{0xAA,0x77,0xDD}', calculated value='{0x%x,0x%x,0x%x}'", value.a, (value.b.get() ? *value.b.get() : 0), (value.c.get() ? *value.c.get() : 0));
      fprintf(logfile, "test_SeqType1_dec:: expected value='{0xAA,0x77,0xDD}', calculated value='{0x%x,0x%x,0x%x}'", value.a, (value.b.get() ? *value.b.get() : 0), (value.c.get() ? *value.c.get() : 0));
      return false;
    }

  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqType4_dec::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
