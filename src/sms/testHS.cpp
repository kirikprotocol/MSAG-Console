#include <iostream>
#include <iomanip>
#include <cstdlib>

#include "sms.h"
using namespace std;
using namespace smsc::sms;

void printBytes(const char* bytes,unsigned len)
{
  const unsigned char* b = (const unsigned char*)bytes;
  for ( unsigned i = 0; i < len; ++i )
  {
    cerr << setw(2) << hex << (unsigned)b[i] << " "; 
  }
  cerr << endl;
}

int main()
{
	const char* text = (const char*)L"text sample 1234567890";
  unsigned textLen = 26;
  SMS sms;
  cerr << "before" << endl;
  printBytes(text,textLen);
  sms.setIntProperty(Tag::SMPP_DATA_CODING,0x8);
  sms.setIntProperty(Tag::SMPP_ESM_CLASS,0);
  sms.setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,text,textLen);
  unsigned len;
  const char* text2 = sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  printBytes(text2,13);
  cerr << "after" << endl;
	return 0;
}

