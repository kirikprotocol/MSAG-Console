#include "test/util/TextUtil.hpp"
#include "util/debug.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace smsc::test::util;

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "Usage: test <dataCoding> <bytes>" << endl;
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
	return 0;
}
