#include <iostream>
#include <iomanip>
#include <cstdlib>

#include "sms.h"
using namespace std;
using namespace smsc::sms;

void printBytes(const char* bytes,unsigned len)
{
  const unsigned char* b = (const unsigned char*)bytes;
  for ( unsigned i = 0; i < len/2; i+=2 )
  {
    cerr  << hex << setw(2) << setfill('0') << (unsigned)b[i*2] << "x" << setw(2) << setfill('0') << (unsigned)b[i*2+1] << " "; 
  }
  cerr << endl;
}

int main()
{
	char* text = "text sample 1234567890";
  wchar_t text2[64];
  unsigned textLen = mbstowcs(text2,text,strlen(text))*2;
  cerr << "sizeof wchar_t " << sizeof(wchar_t) << endl;
  SMS sms;
  printBytes((char*)text,textLen/2);
  cerr << "before" << endl;
  printBytes((char*)text2,textLen);
  sms.setIntProperty(Tag::SMPP_DATA_CODING,0x8);
  sms.setIntProperty(Tag::SMPP_ESM_CLASS,0);
  sms.setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,(char*)text2,textLen);
  unsigned len;
  const char* text3 = sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  printBytes(text3,len);
  SMS sms2;
  sms2.setIntProperty(Tag::SMPP_DATA_CODING,0x8);
  sms2.setIntProperty(Tag::SMPP_ESM_CLASS,0);
  sms2.setBinProperty(Tag::SMPP_SHORT_MESSAGE,text3,len);
  const char* text4 = sms2.getBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,&len);
  printBytes(text4,len);
  cerr << "after" << endl;
	return 0;
}

