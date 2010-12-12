#ifndef __EYELINE_ASN1_BER_TESTS_COMMON_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_ASN1_BER_TESTS_COMMON_HPP__

#include <stddef.h>

# define MAX_PATTERN_LEN 2048
# define MAX_ENCODED_LEN 1024
# define MAX_ERR_MESSAGE 65535


namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

bool test_BOOL_enc(char* err_msg);
bool test_INT_enc(char* err_msg);
bool test_INT8_edgeValue_enc(char* err_msg);
bool test_INT16_edgeValue_enc(char* err_msg);
bool test_INT32_edgeValue_enc(char* err_msg);
bool test_OCTET_STRING_enc(char* err_msg);
bool test_OCTET_STRING_constructed_enc(char* err_msg);
bool test_BIT_STRING_enc(char* err_msg);
bool test_SeqType1_enc(char* err_msg);
bool test_SeqTypeExplicit1_enc(char* err_msg);
bool test_SeqType2_enc(char* err_msg);
bool test_SeqType3_enc(char* err_msg);
bool test_SeqType4_enc(char* err_msg);
bool test_SeqType4_minPrealloc(char* err_msg);
bool test_SeqOfType1_enc(char* err_msg);

bool test_BOOL_dec(char* err_msg);
bool test_INT_dec(char* err_msg);
bool test_INT8_edgeValue_dec(char* err_msg);
bool test_INT16_edgeValue_dec(char* err_msg);
bool test_INT32_edgeValue_dec(char* err_msg);
bool test_OCTET_STRING_dec(char* err_msg);
bool test_OCTET_STRING_constructed_dec(char* err_msg);
bool test_BIT_STRING_dec(char* err_msg);
bool test_SeqType1_dec(char* err_msg);
bool test_SeqTypeExplicit1_dec(char* err_msg);
bool test_SeqType2_dec(char* err_msg);
bool test_SeqType3_dec(char* err_msg);
bool test_SeqType4_dec(char* err_msg);
bool test_SeqOfType1_dec(char* err_msg);

int
write_transfer_syntax(const void *buffer, size_t size, void *pattern_tr_syntax);

}}}}

#endif
