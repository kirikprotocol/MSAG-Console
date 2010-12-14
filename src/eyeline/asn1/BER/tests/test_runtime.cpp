#include <stdio.h>

#include "common.hpp"
#include "TestPatternsRegistry.hpp"

FILE* logfile;

//using eyeline::asn1::ber::tests;

extern "C"
void test_runtime()
{
  char errMsg[MAX_ERR_MESSAGE];

  eyeline::asn1::ber::tests::TestPatternsRegistry::init();

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
