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
	"abcdefghijklmnopqrstuvwxyz",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	"àáâãäå¸æçèéêëìíîïðñòóôõö÷øùúûüýþÿ",
	"ÀÁÂÃÄÅ¨ÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß",
	"1234567890",
	"æàæàæ æÀæàæ Æàæàæ ÆÀæàæ",
	"æàæàæ æÀæàæ æàæÀæ æÀæÀæ æàÆàæ æÀÆàæ æàÆÀæ æÀÆÀæ",
	"æàæàæ æàæÀæ æàæàÆ æàæÀÆ",
	"`-=\\][	;'/., ~!@#$%^&*()_+|}{:\"?><¹"
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
	cout << "Usage: translit [cp1251 file]" << endl;
	if (argc == 2)
	{
		text = new TextObject(new ifstream(argv[1])); //cp1251
	}
	else
	{
		text = new TextObject();
	}
	ofstream original("original.txt");
	ofstream translit("translit.txt");
	string s;
	s.reserve(1024);
	while (text->getLine(s))
	{
		s += "\n";
		int textLen;
		char text[s.length() * 2];
		//original
		original.write(s.c_str(), s.length());
		//translit
		char translitBuf[s.length() * 2];
		int translitLen = Transliterate(s.c_str(), s.length(),
                    CONV_ENCODING_CP1251, translitBuf, sizeof(translitBuf));
		translit.write(translitBuf, translitLen);
	}
	delete text;
	cout << "Conversion complete" << endl;
}

