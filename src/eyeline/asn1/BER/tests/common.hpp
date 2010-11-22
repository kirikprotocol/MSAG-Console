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

bool test_BOOL(char* err_msg);
bool test_INT(char* err_msg);
bool test_INT_edgeValue(char* err_msg);
bool test_OCTET_STRING(char* err_msg);
bool test_BIT_STRING(char* err_msg);
bool test_SeqType1(char* err_msg);
bool test_SeqTypeExplicit1(char* err_msg);
bool test_SeqType2(char* err_msg);
bool test_SeqType3(char* err_msg);
bool test_SeqType4(char* err_msg);
bool test_SeqType4_minPrealloc(char* err_msg);
bool test_SeqOfType1(char* err_msg);

int
write_transfer_syntax(const void *buffer, size_t size, void *pattern_tr_syntax);

}}}}

#endif
