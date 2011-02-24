#include <stdio.h>
#include <string.h>

#include "asn1c_gen/OIDType.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeOID.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "TestPatternsRegistry.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_OID_enc(char* err_msg)
{
  OIDType_t value={0};
  asn_enc_rval_t retVal;
  char patternTrSyntax[MAX_PATTERN_LEN]={0};
  unsigned int oidValue[] = { 1, 2, 3, 10000 };

  int st= OBJECT_IDENTIFIER_set_arcs(&value,
                                     oidValue,
                                     sizeof(oidValue[0]),
                                     sizeof(oidValue)/sizeof(oidValue[0]));
  printf("test_OID_enc:\t\t\t\t");
  retVal = der_encode(&asn_DEF_OIDType,
                      &value, write_transfer_syntax, patternTrSyntax);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_OID_enc:: OIDValue={1,2,3,10000}, trSyntax=%s, retVal=%ld\n",
          patternTrSyntax, retVal.encoded);
  TestPatternsRegistry::getInstance().insertResultPattern("test_OID", "{1,2,3,10000}", patternTrSyntax);

  try {
    EncoderOfObjectID encObjId;
    ObjectID objId;
    uint16_t oidValues[] = { 1, 2, 3, 10000 };
    objId.append(oidValues, sizeof(oidValues)/sizeof(oidValues[0]));
    encObjId.setValue(objId);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encObjId.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_OID_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_OID_enc:: OIDValue={1,2,3,10000}, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax, trSyntaxAsStr);
      fprintf(logfile, "test_OID_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax, trSyntaxAsStr);
      return false;
    }
  } catch(std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_OID_enc:: caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
