#include "util/recoder/recode_dll.h"
#include "util/debug.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		cout << "recoder <cp1251 file>" << endl;
		exit(-1);
	}
	ifstream in(argv[1]); //cp1251
	ofstream ucs2test("ucs2test.txt");
	ofstream ucs2backTest("ucs2backTest.txt");
	ofstream koi8test("koi8test.txt");
	ofstream koi8backTest("koi8backTest.txt");
	ofstream bit7backTest("bit7backTest.txt");
	string s;
	s.reserve(1024);
	while (getline(in, s))
	{
		s += "\n";
		int textLen;
		char text[s.length() * 2];
		//ucs2
		short ucs2Buf[s.length() + 10];
		int ucs2Len = ConvertMultibyteToUCS2(s.c_str(), s.length(), ucs2Buf,
			sizeof(ucs2Buf), CONV_ENCODING_CP1251);
		ucs2test.write((char*) ucs2Buf, ucs2Len);
		//ucs2back
		textLen = ConvertUCS2ToMultibyte(ucs2Buf, ucs2Len,
			text, sizeof(text), CONV_ENCODING_CP1251);
		ucs2backTest.write(text, textLen);
		//koi8
		char koi8Buf[s.length() + 10];
		int koi8Len = ConvertUCS2ToMultibyte(ucs2Buf, ucs2Len,
			koi8Buf, sizeof(koi8Buf), CONV_ENCODING_KOI8R);
		koi8test.write(koi8Buf, koi8Len);
		//koi8back
		ucs2Len = ConvertMultibyteToUCS2(koi8Buf, koi8Len, ucs2Buf,
			sizeof(ucs2Buf), CONV_ENCODING_KOI8R);
		textLen = ConvertUCS2ToMultibyte(ucs2Buf, ucs2Len,
			text, sizeof(text), CONV_ENCODING_CP1251);
		koi8backTest.write(text, textLen);
		//bit7
		char bit7buf[s.length() * 2];
		int bit7len = ConvertTextTo7Bit(s.c_str(), s.length(),
			bit7buf, sizeof(bit7buf), CONV_ENCODING_CP1251);
		textLen = Convert7BitToText(bit7buf, bit7len, text, sizeof(text));
		bit7backTest.write(text, textLen);
	}
	ucs2test.flush();
	ucs2backTest.flush();
	koi8test.flush();
	koi8backTest.flush();
	bit7backTest.flush();
}

