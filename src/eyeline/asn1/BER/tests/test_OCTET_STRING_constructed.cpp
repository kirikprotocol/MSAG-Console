#include <stdio.h>
#include <string.h>

#include "asn1c_gen/OctsStrValue.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeOCTSTR.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeOCTSTR.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "TestPatternsRegistry.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_OCTET_STRING_constructed_enc(char* err_msg)
{
  OctsStrValue_t value;
  char patternTrSyntax[MAX_PATTERN_LEN]={0};
  uint8_t buf4value[1001];
  memset(buf4value, 5, sizeof(buf4value));
  value.buf= buf4value;
  value.size= sizeof(buf4value);
  printf("test_OCTET_STRING_constructed_enc:\t");

  EncoderOfOCTSTR encOctStr(TransferSyntax::ruleCER);
  encOctStr.setValue((eyeline::asn1::TSLength)sizeof(buf4value), buf4value);
  try {
    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encOctStr.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_OCTET_STRING_constructed_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_OCTET_STRING_constructed_enc:: OctsStrValueConstructed=%s, trSyntax=%s\n",
            utilx::hexdmp(buf4value, sizeof(buf4value)).c_str(), trSyntaxAsStr);
    TestPatternsRegistry::getInstance().insertResultPattern("test_OCTETSTRING_constructed", "0505.....0505", patternTrSyntax);
//    if ( strcmp(trSyntaxAsStr, patternTrSyntax)) {
//      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax, trSyntaxAsStr);
//      fprintf(logfile, "test_OCTET_STRING:: expected value='%s', calculated value='%s'\n", patternTrSyntax, trSyntaxAsStr);
//      return false;
//    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_OCTET_STRING_constructed_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

bool
test_OCTET_STRING_constructed_dec(char* err_msg)
{
  printf("test_OCTET_STRING_constructed_dec:\t");
  try {
    const std::string& patternTrSyntax = TestPatternsRegistry::getInstance().getResultPattern("test_OCTETSTRING", "0505...0505");

    fprintf(logfile, "test_OCTET_STRING_constructed_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    DecoderOfOCTSTR decOctStr(TransferSyntax::ruleCER);
    //smsc::core::buffers::LWArray_T<uint8_t, uint16_t, 1024, smsc::core::buffers::LWArrayTraitsPOD_T> expectedValue;
    asn1::OCTSTR_T<1024>  expectedValue;
    decOctStr.setValue(expectedValue, 1024);

    DECResult decResult= decOctStr.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_OCTET_STRING_constructed_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    uint8_t buf4value[1001];
    memset(buf4value, 5, sizeof(buf4value));

    const std::string expectedValueAsStr = utilx::hexdmp(expectedValue.get(), expectedValue.size());
    fprintf(logfile, "test_OCTET_STRING_constructed_dec:: expectedValue=%s\n", expectedValueAsStr.c_str());

    if (memcmp(expectedValue.get(), buf4value, sizeof(buf4value)))
      return false;

  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_OCTET_STRING_constructed_dec::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
