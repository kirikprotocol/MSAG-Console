#include <stdio.h>

#include "common.hpp"

FILE* logfile;

//using eyeline::asn1::ber::tests;

extern "C"
void test_runtime()
{
  char errMsg[MAX_ERR_MESSAGE];
  logfile= fopen("test_runtime.log", "w");

  if (!eyeline::asn1::ber::tests::test_BOOL(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_INT(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_OCTET_STRING(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_BIT_STRING(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqType1(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqType2(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqType3(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqType4(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");

  if (!eyeline::asn1::ber::tests::test_SeqOfType1(errMsg)) {
    printf ("failed: %s\n", errMsg);
  } else
    printf("ok\n");
}

int main()
{
  test_runtime();
  return 0;
}
