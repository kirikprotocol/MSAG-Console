#include "util/recoder/recode_dll.h"
#include "util/debug.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

class TextObject
{
	istream* is;
	int i;
	static string text[];
public:
	TextObject() : is(NULL), i(0) {}
	TextObject(istream* _is) : is(_is) {}
	~TextObject();
	bool getLine(string& s);
};

string TextObject::text[] =
{
	"`1234567890-=\\][poiuytrewq	asdfghjkl;'/.,mnbvcx z",
	"~!@#$%^&*()_+|}{POIUYTREWQ	ASDFGHJKL:\"?><MNBVCX Z",
	"¸1234567890-=\\úõçùøãíåêóöé	ôûâàïðîëäæý.þáüòèìñ÷ ÿ",
	"¨!\"¹;%:?*()_+/ÚÕÇÙØÃÍÅÊÓÖÉ	ÔÛÂÀÏÐÎËÄÆÝ,ÞÁÜÒÈÌÑ× ß"
};

TextObject::~TextObject()
{
	if (is)
	{
		delete is;
	}
}

bool TextObject::getLine(string& s)
{
	if (is)
	{
		return getline(*is, s);
	}
	else if (i < sizeof(text)/sizeof(*text))
	{
		s = text[i++];
		return true;
	}
	return false;
}

int main(int argc, char* argv[])
{
	TextObject* text;
	cout << "Usage: recoder [cp1251 file]" << endl;
	if (argc == 2)
	{
		text = new TextObject(new ifstream(argv[1])); //cp1251
	}
	else
	{
		text = new TextObject();
	}
	ofstream original("original.txt");
	ofstream ucs2test("ucs2test.txt");
	ofstream ucs2backTest("ucs2backTest.txt");
	ofstream koi8test("koi8test.txt");
	ofstream koi8backTest("koi8backTest.txt");
	ofstream bit7backTest("bit7backTest.txt");
	string s;
	s.reserve(1024);
	while (text->getLine(s))
	{
		s += "\n";
		int textLen;
		char text[s.length() * 2];
		//original
		original.write(s.c_str(), s.length());
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
	delete text;
	cout << "Conversion complete" << endl;
}

