#ifndef TEST_UTIL_TEXT_UTIL
#define TEST_UTIL_TEXT_UTIL

#include "Util.hpp"
#include "util/recoder/recode_dll.h"
#include <string>
#include <vector>
#include <memory>

namespace smsc {
namespace test {
namespace util {

using std::string;
using std::vector;
using std::auto_ptr;
using std::pair;

//data_coding
const uint8_t DATA_CODING_SMSC_DEFAULT = 0x0;
const uint8_t DATA_CODING_UCS2 = 0x8;

auto_ptr<char> rand_text(int& length, uint8_t dataCoding = DATA_CODING_SMSC_DEFAULT);
void rand_text(int& length, char* buf, uint8_t dataCoding = DATA_CODING_SMSC_DEFAULT);

auto_ptr<char> encode(const string& text, uint8_t dataCoding, int& msgLen);
const string decode(const char* text, int len, uint8_t dataCoding);
const pair<string, uint8_t> convert(const string& text, int profileCodePage);

vector<int> compare(uint8_t dc1, const char* str1, int len1,
	uint8_t dc2, const char* str2, int len2);

uint8_t getDataCoding(int num);
int getMaxChars(uint8_t dataCoding);
int findPos(const string& text, const string& expected, int segmentSize);

}
}
}

#endif /* TEST_UTIL_TEXT_UTIL */
