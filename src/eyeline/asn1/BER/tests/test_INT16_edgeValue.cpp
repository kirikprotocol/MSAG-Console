#include <stdio.h>
#include <string.h>

#include "asn1c_gen/IntValue.h"
#include "common.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "TestPatternsRegistry.hpp"

extern FILE* logfile;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

// return true on success or false on error
bool
test_INT16_edgeValue_enc(char* err_msg)
{
  IntValue_t value1= -1, value2= 0x7fff,
             value3= 0xFFFF, value4= 0x4000, value5= 0x40;
  asn_enc_rval_t retVal;
  char patternTrSyntax_1[MAX_PATTERN_LEN]={0};
  char patternTrSyntax_2[MAX_PATTERN_LEN]={0};
  char patternTrSyntax_3[MAX_PATTERN_LEN]={0};
  char patternTrSyntax_4[MAX_PATTERN_LEN]={0};
  char patternTrSyntax_5[MAX_PATTERN_LEN]={0};

  printf("test_INT16_edgeValue_enc:\t\t");
  retVal = der_encode(&asn_DEF_IntValue,
                      &value1, write_transfer_syntax, patternTrSyntax_1);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_INT16_edgeValue_enc:: IntValue=-1, trSyntax=%s, retVal=%ld\n", patternTrSyntax_1, retVal.encoded);
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT16_edgeValue", "-1", patternTrSyntax_1);

  retVal = der_encode(&asn_DEF_IntValue,
                      &value2, write_transfer_syntax, patternTrSyntax_2);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_INT16_edgeValue_enc:: IntValue=0x7fff, trSyntax=%s, retVal=%ld\n", patternTrSyntax_2, retVal.encoded);
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT16_edgeValue", "0x7fff", patternTrSyntax_2);

  retVal = der_encode(&asn_DEF_IntValue,
                      &value3, write_transfer_syntax, patternTrSyntax_3);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_INT16_edgeValue_enc:: IntValue=0xffff, trSyntax=%s, retVal=%ld\n", patternTrSyntax_3, retVal.encoded);
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT16_edgeValue", "0xffff", patternTrSyntax_3);

  retVal = der_encode(&asn_DEF_IntValue,
                      &value4, write_transfer_syntax, patternTrSyntax_4);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_INT16_edgeValue_enc:: IntValue=0x4000, trSyntax=%s, retVal=%ld\n", patternTrSyntax_4, retVal.encoded);
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT16_edgeValue", "0x4000", patternTrSyntax_4);
  retVal = der_encode(&asn_DEF_IntValue,
                      &value5, write_transfer_syntax, patternTrSyntax_5);
  if (retVal.encoded == -1)
    return false;

  fprintf(logfile, "test_INT16_edgeValue_enc:: IntValue=0x40, trSyntax=%s, retVal=%ld\n", patternTrSyntax_5, retVal.encoded);
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT16_edgeValue", "0x40", patternTrSyntax_5);

  try {
    EncoderOfINTEGER encInt;
    encInt.setValue((int16_t)-1);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encInt.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_INT16_edgeValue_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_INT16_edgeValue_enc:: IntValue=-1, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax_1)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax_1, trSyntaxAsStr);
      fprintf(logfile, "test_INT16_edgeValue_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax_1, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT16_edgeValue_enc:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    EncoderOfINTEGER encInt;
    encInt.setValue((int16_t)0x7fff);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encInt.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_INT16_edgeValue_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_INT16_edgeValue_enc:: IntValue=0x7fff, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax_2)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax_2, trSyntaxAsStr);
      fprintf(logfile, "test_INT16_edgeValue_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax_2, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT16_edgeValue_enc:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    EncoderOfINTEGER encInt;
    encInt.setValue((uint16_t)0xffff);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encInt.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_INT16_edgeValue_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_INT16_edgeValue_enc:: IntValue=0xffff, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax_3)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax_3, trSyntaxAsStr);
      fprintf(logfile, "test_INT16_edgeValue_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax_3, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT16_edgeValue_enc:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    EncoderOfINTEGER encInt;
    encInt.setValue((int16_t)0x4000);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encInt.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_INT16_edgeValue_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_INT16_edgeValue_enc:: IntValue=0x4000, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax_4)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax_4, trSyntaxAsStr);
      fprintf(logfile, "test_INT16_edgeValue_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax_4, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT16_edgeValue_enc:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    EncoderOfINTEGER encInt;
    encInt.setValue((int16_t)0x40);

    uint8_t encodedBuf[MAX_ENCODED_LEN];
    char trSyntaxAsStr[MAX_PATTERN_LEN];

    ENCResult encResult= encInt.encode(encodedBuf, MAX_ENCODED_LEN);
    fprintf(logfile, "test_INT16_edgeValue_enc:: ENCResult.status=%d\n", encResult.status);
    if (encResult.status != ENCResult::encOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "ENCResult.status=%d", encResult.status);
      return false;
    }
    utilx::hexdmp(trSyntaxAsStr, sizeof(trSyntaxAsStr), encodedBuf, encResult.nbytes);
    fprintf(logfile, "test_INT16_edgeValue_enc:: IntValue=0x40, trSyntax=%s\n", trSyntaxAsStr);
    if ( strcmp(trSyntaxAsStr, patternTrSyntax_5)) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "expected value='%s', calculated value='%s'", patternTrSyntax_5, trSyntaxAsStr);
      fprintf(logfile, "test_INT16_edgeValue_enc:: expected value='%s', calculated value='%s'\n", patternTrSyntax_5, trSyntaxAsStr);
      return false;
    }
  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT16_edgeValue_enc:: caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

// return true on success or false on error
bool
test_INT16_edgeValue_dec(char* err_msg)
{
  printf("test_INT16_edgeValue_dec:\t\t");
  try {
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_INT16_edgeValue", "-1");
    fprintf(logfile, "test_INT16_edgeValue_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    DecoderOfINTEGER decInt;
    int16_t expectedValue=0;
    decInt.setValue(expectedValue);

    DECResult decResult= decInt.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_INT16_edgeValue_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    fprintf(logfile, "test_INT16_edgeValue_dec:: expectedValue=%d\n", expectedValue);
    if ( expectedValue!= -1 )
      return false;

  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT16_edgeValue_dec:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_INT16_edgeValue", "0x7fff");
    fprintf(logfile, "test_INT16_edgeValue_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    DecoderOfINTEGER decInt;
    int16_t expectedValue=0;
    decInt.setValue(expectedValue);

    DECResult decResult= decInt.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_INT16_edgeValue_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    fprintf(logfile, "test_INT16_edgeValue_dec:: expectedValue=%d\n", expectedValue);
    if ( expectedValue!= 0x7fff )
      return false;

  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT16_edgeValue_dec:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_INT16_edgeValue", "0xffff");
    fprintf(logfile, "test_INT16_edgeValue_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    DecoderOfINTEGER decInt;
    uint16_t expectedValue=0;
    decInt.setValue(expectedValue);

    DECResult decResult= decInt.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_INT16_edgeValue_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    fprintf(logfile, "test_INT16_edgeValue_dec:: expectedValue=%d\n", expectedValue);
    if ( expectedValue!= 0xffff )
      return false;

  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT16_edgeValue_dec:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_INT16_edgeValue", "0x4000");
    fprintf(logfile, "test_INT16_edgeValue_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    DecoderOfINTEGER decInt;
    int16_t expectedValue=0;
    decInt.setValue(expectedValue);

    DECResult decResult= decInt.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_INT16_edgeValue_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    fprintf(logfile, "test_INT16_edgeValue_dec:: expectedValue=%d\n", expectedValue);
    if ( expectedValue!= 0x4000 )
      return false;

  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT16_edgeValue_dec:: caught exception [%s]\n", ex.what());
    return false;
  }

  try {
    const std::string& patternTrSyntax= TestPatternsRegistry::getInstance().getResultPattern("test_INT16_edgeValue", "0x40");
    fprintf(logfile, "test_INT16_edgeValue_dec:: patternTrSyntax=%s\n", patternTrSyntax.c_str());
    uint8_t patternTrSyntaxBin[MAX_PATTERN_LEN];
    size_t patternLen= utilx::hexbuf2bin(patternTrSyntax.c_str(), patternTrSyntaxBin, sizeof(patternTrSyntaxBin));

    DecoderOfINTEGER decInt;
    int16_t expectedValue=0;
    decInt.setValue(expectedValue);

    DECResult decResult= decInt.decode(patternTrSyntaxBin, patternLen);
    fprintf(logfile, "test_INT16_edgeValue_dec:: DECResult.status=%d\n", decResult.status);
    if (decResult.status != DECResult::decOk) {
      snprintf(err_msg, MAX_ERR_MESSAGE, "DECResult.status=%d", decResult.status);
      return false;
    }

    fprintf(logfile, "test_INT16_edgeValue_dec:: expectedValue=%d\n", expectedValue);
    if ( expectedValue!= 0x40 )
      return false;

  } catch (std::exception& ex) {
    snprintf(err_msg, MAX_ERR_MESSAGE, "caught exception [%s]", ex.what());
    fprintf(logfile, "test_INT16_edgeValue_dec:: caught exception [%s]\n", ex.what());
    return false;
  }

  return true;
}

}}}}
