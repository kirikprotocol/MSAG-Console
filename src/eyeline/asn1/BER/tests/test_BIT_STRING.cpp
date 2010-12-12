#include <stdio.h>
#include <string.h>

#include "asn1c_gen/BitStrValue.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeBITSTR.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "TestPatternsRegistry.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_BIT_STRING_enc(char* err_msg)
{
  BitStrValue_t value;
  asn_enc_rval_t retVal;
  char patternTrSyntax[MAX_PATTERN_LEN]={0};
  uint8_t buf4value[] = { 0xF0, 0x0F, 0xAA };

  value.buf= buf4value;
  value.size= sizeof(buf4value);
  value.bits_unused= 2;
  printf("test_BIT_STRING_enc:\t\t\t");
  retVal = der_encode(&asn_DEF_BitStrValue,
                      &value, write_transfer_syntax, patternTrSyntax);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_BIT_STRING_enc:: BitStrValue=%s, trSyntax=%s, retVal=%ld\n",
          utilx::hexdmp(buf4value, sizeof(buf4value)).c_str(), patternTrSyntax, (long int)retVal.encoded);
  TestPatternsRegistry::getInstance().insertResultPattern("test_BITSTRING", "F00FAA", patternTrSyntax);

  try {
    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];
    EncoderOfBITSTR encBitStr;
    encBitStr.setValue((TSLength)sizeof(buf4value)*8 - 2, buf4value);

    ENCResult encResult= encBitStr.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_BIT_STRING_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_BIT_STRING_enc:: BitStrValue, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax, trSyntaxAsStr);
      fprintf(logfile, "test_BIT_STRING_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_BIT_STRING_enc::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
