#include <stdio.h>

#include "common.hpp"
#include "TestPatternsRegistry.hpp"

FILE* logfile;

//using eyeline::asn1::ber::tests;
namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

void
test_runtime_framework_init()
{
  TestPatternsRegistry::getInstance().insertResultPattern("test_BOOL", "false", "010100");
  TestPatternsRegistry::getInstance().insertResultPattern("test_BOOL", "true", "0101FF");
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT", "100", "020164");

  TestPatternsRegistry::getInstance().insertResultPattern("test_INT32_edgeValue", "-1", "0201FF");
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT32_edgeValue", "7FFFFFFF", "02047FFFFFFF");
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT32_edgeValue", "FFFFFFFF", "020500FFFFFFFF");
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT32_edgeValue", "0x400000", "0203400000");
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT32_edgeValue", "0x4000", "02024000");

  TestPatternsRegistry::getInstance().insertResultPattern("test_INT16_edgeValue", "-1", "0201FF");
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT16_edgeValue", "7fff", "02027FFF");
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT16_edgeValue", "ffff", "020300FFFF");
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT16_edgeValue", "0x4000", "02024000");
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT16_edgeValue", "0x40", "020140");

  TestPatternsRegistry::getInstance().insertResultPattern("test_INT8_edgeValue", "-1", "0201FF");
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT8_edgeValue", "7f", "02017F");
  TestPatternsRegistry::getInstance().insertResultPattern("test_INT8_edgeValue", "ff", "020200FF");

  TestPatternsRegistry::getInstance().insertResultPattern("test_BITSTRING", "F00FAA", "030402F00FA8"); // bits string F00FAA with 2 unused bits at the end
  TestPatternsRegistry::getInstance().insertResultPattern("test_OCTETSTRING", "01020304050504030201", "040A01020304050504030201");
  TestPatternsRegistry::getInstance().insertResultPattern("test_OCTETSTRING", "0505...0505",
                                                          "2480048203E805050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "05050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505050505"
                                                          "0505050505050505050505050505050505050401050000");
}

}}}}

extern "C"
void test_runtime()
{
  char errMsg[MAX_ERR_MESSAGE];

  eyeline::asn1::ber::tests::TestPatternsRegistry::init();

  eyeline::asn1::ber::tests::test_runtime_framework_init();

  if (!eyeline::asn1::ber::tests::test_BOOL_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_INT_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_INT32_edgeValue_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_INT16_edgeValue_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_INT8_edgeValue_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_OCTET_STRING_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_OCTET_STRING_constructed_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_BIT_STRING_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqType1_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqTypeExplicit1_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqType2_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqType3_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqType4_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqType4_minPrealloc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqOfType1_enc(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_BOOL_dec(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_INT_dec(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_INT32_edgeValue_dec(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_INT16_edgeValue_dec(errMsg)) {
     printf ("failed: %s\n", errMsg);
   } else
     printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_INT8_edgeValue_dec(errMsg)) {
     printf ("failed: %s\n", errMsg);
   } else
     printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_BIT_STRING_dec(errMsg)) {
     printf ("failed: %s\n", errMsg);
   } else
     printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_OCTET_STRING_dec(errMsg)) {
     printf ("failed: %s\n", errMsg);
   } else
     printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_OCTET_STRING_constructed_dec(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqType1_dec(errMsg)) {
     printf ("failed: %s\n", errMsg);
   } else
     printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqType2_dec(errMsg)) {
     printf ("failed: %s\n", errMsg);
   } else
     printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqType3_dec(errMsg)) {
     printf ("failed: %s\n", errMsg);
   } else
     printf("ok\n");

}

int main()
{
  logfile = fopen("test_runtime.log", "w");
  setvbuf(stdout, NULL, _IOLBF, 0);
  setvbuf(logfile, NULL, _IOLBF, 0);

  test_runtime();
  return 0;
}
