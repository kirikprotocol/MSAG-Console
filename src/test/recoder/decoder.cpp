#include "test/util/TextUtil.hpp"
#include "util/debug.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace smsc::smpp::DataCoding;
using namespace smsc::test::util;

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "Usage: test <dataCoding> <bytes> [-t]" << endl;
		return 0;
	}
	uint8_t dataCoding = atoi(argv[1]);
	const char* input = argv[2];
	cout << "dataCoding = " << dataCoding << ", input = " << input << endl;
	string tmp;
	istringstream is(input);
	ofstream out("out");
	is >> hex;
	while (!is.eof())
	{
		int ch;
		is >> ch;
		tmp += (unsigned char) ch;
		out.put(ch);
		//__trace2__("ch = %d", ch);
	}
	//__trace2__("tmp len = %d", tmp.length());
	const string str = decode(tmp.c_str(), tmp.length(), dataCoding, false);
	cout << "output = \"" << str << "\"" << endl;
	for (int i = 3; i < argc; i++)
	{
		if (string(argv[i]) == "-t" && dataCoding == 8)
		{
			char buf[(int) (1.5 * str.length())];
			int len = Transliterate(str.c_str(), str.length(),
				CONV_ENCODING_CP1251, buf, sizeof(buf));
			buf[len] = 0;
			cout << "translit = \"" << buf << "\"" << endl;
		}
		if (string(argv[i]) == "-c")
		{
			if (dataCoding == DEFAULT || dataCoding == SMSC7BIT)
			{
				int specialSymb = 0;
				for (int j = 0; j < str.length(); j++)
				{
					switch (str[j])
					{
						case '|':
						case '^':
						case '{':
						case '}':
						case '[':
						case ']':
						case '~':
						case '\\':
							specialSymb++;
							break;
					}
				}
				cout << "Number of special symbols = " << specialSymb << endl;
			}
		}
	}
	return 0;
}
