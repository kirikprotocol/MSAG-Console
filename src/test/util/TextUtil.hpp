#ifndef TEST_UTIL_TEXT_UTIL
#define TEST_UTIL_TEXT_UTIL

#include "Util.hpp"
#include <string>
#include <vector>

namespace smsc {
namespace test {
namespace util {

using std::string;
using std::vector;

//data_coding
const uint8_t DATA_CODING_SMSC_DEFAULT = 0x0;
const uint8_t DATA_CODING_UCS2 = 0x8;

std::auto_ptr<char> rand_text(int length, uint8_t dataCoding = DATA_CODING_SMSC_DEFAULT);
void rand_text(int length, char* buf, uint8_t dataCoding = DATA_CODING_SMSC_DEFAULT);

const string encode(const char* text, uint8_t dataCoding);
const string decode(const char* text, int len, uint8_t dataCoding);

vector<int> compare(uint8_t dc1, const char* str1, int len1,
	uint8_t dc2, const char* str2, int len2);


}
}
}

#endif /* TEST_UTIL_TEXT_UTIL */
