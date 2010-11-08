#ifndef __EYELINE_ASN1_BER_TESTS_COMMON_HPP__
# ident "@(#)$Id$"
# define __EYELINE_ASN1_BER_TESTS_COMMON_HPP__

# define MAX_PATTERN_LEN 2048
# define MAX_ENCODED_LEN 1024
# define MAX_ERR_MESSAGE 65535

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

bool test_BOOL(char* err_msg);
bool test_INT(char* err_msg);
bool test_OCTET_STRING(char* err_msg);
bool test_BIT_STRING(char* err_msg);
bool test_SeqType1(char* err_msg);
bool test_SeqType2(char* err_msg);

int
write_transfer_syntax(const void *buffer, size_t size, void *pattern_tr_syntax);

}}}}

#endif
