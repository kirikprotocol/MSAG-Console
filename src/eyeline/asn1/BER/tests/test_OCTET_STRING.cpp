#include <stdio.h>
#include <string.h>

#include "asn1c_gen/OctsStrValue.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeOCTSTR.hpp"
#include "eyeline/utilx/hexdmp.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_OCTET_STRING(char* err_msg)
{
  OctsStrValue_t value;
  asn_enc_rval_t retVal;
  char patternTrSyntax[MAX_PATTERN_LEN]={0};
  uint8_t buf4value[] = { 1, 2, 3, 4, 5, 5, 4, 3, 2, 1 };

  value.buf= buf4value;
  value.size= sizeof(buf4value);
  printf("test_OCTET_STRING:\t\t");
  retVal = der_encode(&asn_DEF_OctsStrValue,
                      &value, write_transfer_syntax, patternTrSyntax);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_OCTET_STRING:: OctsStrValue=%s, trSyntax=%s, retVal=%ld\n",
          utilx::hexdmp(buf4value, sizeof(buf4value)).c_str(), patternTrSyntax, retVal.encoded);

  EncoderOfOCTSTR encOctStr;
  encOctStr.setValue((eyeline::asn1::TSLength)sizeof(buf4value), buf4value);
  try {
    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encOctStr.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_OCTET_STRING:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_OCTET_STRING:: OctsStrValue=%s, trSyntax=%s\n",
            utilx::hexdmp(buf4value, sizeof(buf4value)).c_str(), trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax, trSyntaxAsStr);
      fprintf(logfile, "test_OCTET_STRING:: expected value='%s', calculated value='%s'\n", patternTrSyntax, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_OCTET_STRING::caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
