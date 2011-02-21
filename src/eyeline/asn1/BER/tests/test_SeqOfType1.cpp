#include <stdio.h>
#include <string.h>

#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "enc/MESeqOfType1.hpp"
#include "dec/MDSeqOfType1.hpp"
#include "SeqOfType1.hpp"
#include "TestPatternsRegistry.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_SeqOfType1_enc(char* err_msg)
{
  printf("test_SeqOfType1_enc:\t\t\t");

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_SeqOfType1", "{{a=0xEE,b=0x11,c=NULL},{a=0x33,b=0xCC,c=NULL},{a=0x55,b=0xFF,c=NULL},{a=0xAA,b=0x77,c=NULL}}");
    SeqOfType1 value_2;

    value_2.push_back(SeqType1(0xEE, 0x11));
    value_2.push_back(SeqType1(0x33, 0xCC));
    value_2.push_back(SeqType1(0x55, 0xFF));
    value_2.push_back(SeqType1(0xAA, 0x77));

    enc::MESeqOfType1 encSeqOfType1;
    encSeqOfType1.setValue(value_2);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encSeqOfType1.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_SeqOfType1_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_SeqOfType1_enc:: SeqOfType1Value={{a=0xEE,b=0x11,c=NULL},{a=0x33,b=0xCC,c=NULL},{a=0x55,b=0xFF,c=NULL},{a=0xAA,b=0x77,c=NULL}}, trSyntax=%s\n",
            trSyntaxAsStr);
    if (strcmp(trSyntaxAsStr, patternTrSyntax.c_str())) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax.c_str(), trSyntaxAsStr);
      fprintf(logfile, "test_SeqOfType1_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax.c_str(), trSyntaxAsStr);
      return false;
    }

  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqOfType1_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

// return true on success or false on error
bool
test_SeqOfType1_dec(char* err_msg)
{
  printf("test_SeqOfType1_dec:\t\t\t");

  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_SeqOfType1", "{{a=0xEE,b=0x11,c=NULL},{a=0x33,b=0xCC,c=NULL},{a=0x55,b=0xFF,c=NULL},{a=0xAA,b=0x77,c=NULL}}");
    fprintf(logfile, "test_SeqType1_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    SeqOfType1 value;

    dec::MDSeqOfType1 decSeqOfType1(TransferSyntax::ruleDER);
    decSeqOfType1.setValue(value);

    DECResult decResult= decSeqOfType1.decode(patternTrSyntaxBin, patternLen);;
    fprintf(logfile, "test_SeqOfType1_dec:: ENCResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    if (value.size() != 4) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected size of value=4, calculated size of value=%ld", value.size());
      fprintf(logfile, "test_SeqOfType1_dec:: expected size of value=4, calculated size of value=%ld", value.size());
      return false;
    }

    SeqOfType1::const_iterator iter = value.begin();
    if (iter->a != 0xEE || iter->b  != 0x11) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "first element is wrong: expected value={0xEE,0x11,NULL}, calculated value={0x%x,0x%x,NULL}", iter->a, iter->b);
      fprintf(logfile, "test_SeqOfType1_dec:: first element is wrong: expected value={0xEE,0x11,NULL}, calculated value={0x%x,0x%x,NULL}", iter->a, iter->b);
      return false;
    }

    ++iter;
    if (iter->a != 0x33 || iter->b  != 0xCC) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "second element is wrong: expected value={0xEE,0x11,NULL}, calculated value={0x%x,0x%x,NULL}", iter->a, iter->b);
      fprintf(logfile, "test_SeqOfType1_dec:: second element is wrong: expected value={0x33,0xCC,NULL}, calculated value={0x%x,0x%x,NULL}", iter->a, iter->b);
      return false;
    }

    ++iter;
    if (iter->a != 0x55 || iter->b  != 0xFF) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "second element is wrong: expected value={0xEE,0x11,NULL}, calculated value={0x%x,0x%x,NULL}", iter->a, iter->b);
      fprintf(logfile, "test_SeqOfType1_dec:: second element is wrong: expected value={0x33,0xCC,NULL}, calculated value={0x%x,0x%x,NULL}", iter->a, iter->b);
      return false;
    }

    ++iter;
    if (iter->a != 0xAA || iter->b  != 0x77) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "second element is wrong: expected value={0xEE,0x11,NULL}, calculated value={0x%x,0x%x,NULL}", iter->a, iter->b);
      fprintf(logfile, "test_SeqOfType1_dec:: second element is wrong: expected value={0x33,0xCC,NULL}, calculated value={0x%x,0x%x,NULL}", iter->a, iter->b);
      return false;
    }
  } catch(std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_SeqOfType1_dec::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
