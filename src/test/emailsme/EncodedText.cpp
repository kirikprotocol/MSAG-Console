#include "EncodedText.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace emailsme {

using namespace std;

TextTable EncodedText::t;

EncodedText::EncodedText(Charset _charset, TransferEncoding _transferEncoding)
: charset(_charset), transferEncoding(_transferEncoding) {}

const char* EncodedText::getCharset()
{
	switch (charset)
	{
		case KOI8_U:
			return "koi8-u";
		case KOI8_R:
			return "koi8-r";
		case WINDOWS_1251:
			return "windows-1251";
		case ISO_8859_5:
			return "iso-8859-5";
		default:
			__unreachable__("Invalid charset");
	}
}

const char* EncodedText::getTransferEncoding()
{
	switch (transferEncoding)
	{
		case QUOTED_PRINTABLE:
			return "quoted-printable";
		case BIT8:
			return "8bit";
		default:
			__unreachable__("Invalid transfer encoding");

	}
}

const char* EncodedText::getText()
{
	return text.str().c_str();
}

const char* EncodedText::getEncodedText()
{
	return encText.str().c_str();
}

TextTable::TextTable()
{
/*
`1234567890-=\	qwertyuiop[]asdfghjkl;'zxcvbnm,./
~!@#$%^&*()_+|	QWERTYUIOP{}ASDFGHJKL:"ZXCVBNM<>?
¸1234567890-=\	éöóêåíãøùçõúôûâàïğîëäæıÿ÷ñìèòüáş.
¨!"¹;%:?*()_+/	ÉÖÓÊÅÍÃØÙÇÕÚÔÛÂÀÏĞÎËÄÆİß×ÑÌÈÒÜÁŞ,
*/
	TextTable& t = *this;
	//windows-1251, iso-8859-5
	t["latin"] = "`1234567890-=\\\tqwertyuiop[]asdfghjkl;'zxcvbnm,./ ";
	t["latin2"] = "~!@#$%^&*()_+|\tQWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>? ";
	t["rus"] = "¸1234567890-=\\\téöóêåíãøùçõúôûâàïğîëäæıÿ÷ñìèòüáş. ";
	t["rus2"] = "¨!\"¹;%:?*()_+/\tÉÖÓÊÅÍÃØÙÇÕÚÔÛÂÀÏĞÎËÄÆİß×ÑÌÈÒÜÁŞ, ";

	t["latin.quoted-printable"] = "`1234567890-=3D\\=09qwertyuiop[]asdfghjkl;'zxcvbnm,./ ";
	t["latin.base64"] = "YDEyMzQ1Njc4OTAtPVwgcXdlcnR5dWlvcFtdYXNkZmdoamtsOyd6eGN2Ym5tLC4vIA0K";
	t["latin2.base64"] = "fiFAIyQlXiYqKClfK3wgUVdFUlRZVUlPUHt9QVNERkdISktMOiJaWENWQk5NPD4/IA0K";

	t["rus.windows-1251.8bit"] = t["rus"];
	t["rus2.windows-1251.8bit"] = t["rus2"];
	t["rus.windows-1251.quoted-printable"] = "=B81234567890-=3D\\\t=E9=F6=F3=EA=E5=ED=E3=F8=F9=E7=F5=FA=F4=FB=E2=E0=EF=F0=EE=EB=E4=E6=FD=FF=F7=F1=EC=E8=F2=FC=E1=FE. ";
	t["rus2.windows-1251.quoted-printable"] = "=A8!\"=B9;%:?*()_+/\t=C9=D6=D3=CA=C5=CD=C3=D8=D9=C7=D5=DA=D4=DB=C2=C0=CF=D0=CE=CB=C4=C6=DD=DF=D7=D1=CC=C8=D2=DC=C1=DE, ";
	t["rus.windows-1251.base64"] = "uDEyMzQ1Njc4OTAtPVwg6fbz6uXt4/j55/X69Pvi4O/w7uvk5v3/9/Hs6PL84f4uIA0K";
	t["rus2.windows-1251.base64"] = "qCEiuTslOj8qKClfKy8gydbTysXNw9jZx9Xa1NvCwM/QzsvExt3f19HMyNLcwd4sIA0K";
	
	t["rus.iso-8859-5.8bit"] = "ñ1234567890-=\\\tÙæãÚÕİÓèé×åêäëÒĞßàŞÛÔÖíïçáÜØâìÑî. ";
	t["rus2.iso-8859-5.8bit"] = "¡!\"ğ;%:?*()_+/\t¹ÆÃº÷__ÈÉ·ÅÊÄË_°¿À_>_¶ÍÏÇÁ_¸ÂÌ+Î, ";
	t["rus.iso-8859-5.quoted-printable"] = "=F11234567890-=3D\\=09=D9=E6=E3=DA=D5=DD=D3=E8=E9=D7=E5=EA=E4=EB=D2=D0=DF=E0=DE=DB=D4=D6=ED=EF=E7=E1=DC=D8=E2=EC=D1=EE. ";
	t["rus2.iso-8859-5.quoted-printable"] = "=A1!\"=F0;%:?*()_+/=09=B9=C6=C3=BA=B5=BD=B3=C8=C9=B7=C5=CA=C4=CB=B2=B0=BF=C0=BE=BB=B4=B6=CD=CF=C7=C1=BC=B8=C2=CC=B1=CE, ";
	t["rus.iso-8859-5.base64"] = "8TEyMzQ1Njc4OTAtPVwg2ebj2tXd0+jp1+Xq5OvS0N/g3tvU1u3v5+Hc2OLs0e4uIA0K";
	t["rus2.iso-8859-5.base64"] = "oSEi8DslOj8qKClfKy8gucbDurW9s8jJt8XKxMuysL/Avru0ts3Px8G8uMLMsc4sIA0K";

	//koi8
	t["rus.koi8"] = "1234567890-=\\\téöóêåíãøùçõúôûâàïğîëäæıÿ÷ñìèòüáş. ";
	t["rus2.koi8"] = "!\";%:?*()_+/\tÉÖÓÊÅÍÃØÙÇÕÚÔÛÂÀÏĞÎËÄÆİß×ÑÌÈÒÜÁŞ, ";
	
	t["rus.koi8.8bit"] = "1234567890-=\\\tÊÃÕËÅÎÇÛİÚÈßÆÙ×ÁĞÒÏÌÄÖÜÑŞÓÍÉÔØÂÀ. ";
	t["rus2.koi8.8bit"] = "!\";%:?*()_+/\têãõëåîçûıúèÿæù÷áğòïìäöüñşóíéôøâà, ";
	t["rus.koi8.quoted-printable"] = "1234567890-=3D\\\t=CA=C3=D5=CB=C5=CE=C7=DB=DD=DA=C8=DF=C6=D9=D7=C1=D0=D2=CF=CC=C4=D6=DC=D1=DE=D3=CD=C9=D4=D8=C2=C0. ";
	t["rus2.koi8.quoted-printable"] = "!\";%:?*()_+/\t=EA=E3=F5=EB=E5=EE=E7=FB=FD=FA=E8=FF=E6=F9=F7=E1=F0=F2=EF=EC=E4=F6=FC=F1=FE=F3=ED=E9=F4=F8=E2=E0, ";
}

int EncodedText::appendText(TextType type)
{
	string textKey;
	string encTextKey;
	switch (type)
	{
		case LATIN:
			textKey = "latin";
			switch (transferEncoding)
			{
				case QUOTED_PRINTABLE:
					encTextKey = "latin.quoted-printable";
					break;
				case BIT8:
					encTextKey = "latin";
					break;
				default:
					__unreachable__("Invalid transfer encoding");
			}
			break;
		case LATIN2:
			textKey = encTextKey = "latin2";
			break;
		case RUS:
		case RUS2:
			textKey = encTextKey = (type == RUS ? "rus" : "rus2");
			switch (charset)
			{
				case KOI8_U:
				case KOI8_R:
					textKey += ".koi8";
					encTextKey += ".koi8";
					break;
				case WINDOWS_1251:
					encTextKey += ".windows-1251";
					break;
				case ISO_8859_5:
					encTextKey += ".iso-8859-5";
					break;
				default:
					__unreachable__("Invalid charset");
			}
			switch (transferEncoding)
			{
				case QUOTED_PRINTABLE:
					encTextKey += ".quoted-printable";
					break;
				case BIT8:
					encTextKey += ".8bit";
					break;
				default:
					__unreachable__("Invalid transfer encoding");
			}
			break;
		default:
			__unreachable__("Invalid text type");
	}
	text << t[textKey] << endl;
	encText << t[encTextKey] << endl;
	return t[textKey].length();
}

}
}
}

