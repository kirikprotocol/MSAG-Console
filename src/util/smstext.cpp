#include "util/smstext.h"
#include "util/debug.h"
#include <memory>

namespace smsc{
namespace util{

using namespace std;
using std::auto_ptr;

static inline bool isEscapedChar(char c)
{
  return c=='|' || c=='{' || c=='}' || c=='[' || c==']' ||
         c=='\\'|| c=='~';
}

static inline int countEscapedChars(const char* text,int length)
{
  int res=0;
  for(int i=0;i<length;i++)
  {
    if(isEscapedChar(text[i]))res++;
  }
  return res;
}

int splitSms(SMS* tmplSms,const char *text,int length,ConvEncodingEnum encoding,int datacoding,Array<SMS*>& dest)
{
  dest.Clean();
  int buflen=length*2+4;
  auto_ptr<char> buf(new char[buflen]);
  bool hb=hasHighBit(text,length);
  int dc;
  int datalen;
  if(hb && datacoding==DataCoding::UCS2)
  {
    ConvertMultibyteToUCS2(text,length,(short*)buf.get(),buflen,encoding);
    dc=DataCoding::UCS2;
    datalen=length*2;
  }else
  {
    do{
      datalen=Transliterate(text,length,encoding,buf.get(),buflen);
      if(datalen==-1)
      {
        buflen*=2;
        buf=auto_ptr<char>(new char[buflen]);
      }
    }while(datalen==-1);
    dc=DataCoding::DEFAULT;
  }
  __trace2__("splitSms:dc=%d",dc);
  int maxlen=dc==DataCoding::DEFAULT?160:140;
  int sent=0;
  while(sent<datalen)
  {
    SMS *s=new SMS(*tmplSms);
    int piece=datalen-sent>maxlen?maxlen:datalen-sent;
    if(dc==DataCoding::DEFAULT && piece==maxlen)piece-=countEscapedChars(buf.get()+sent,piece);
    s->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,dc);
    s->setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,buf.get()+sent,piece);
    s->setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,piece);
    dest.Push(s);
    sent+=piece;
  }
  return dest.Count();
}

int trimSms(SMS* sms,const char *text,int length,ConvEncodingEnum encoding,int datacoding)
{
  int buflen=length*2+4;
  auto_ptr<char> buf(new char[buflen]);
  bool hb=hasHighBit(text,length);
  int dc;
  int datalen;
  if(hb && datacoding==DataCoding::UCS2)
  {
    ConvertMultibyteToUCS2(text,length,(short*)buf.get(),buflen,encoding);
    dc=DataCoding::UCS2;
    if(length<=70)
    {
      datalen=length*2;
    }else
    {
      datalen=70;
      short d[1];
      ConvertMultibyteToUCS2(".",1,d,1,CONV_ENCODING_ANSI);
      short *b=(short*)buf.get();
      b[69]=d[0];
      b[68]=d[0];
      b[67]=d[0];
    }
  }else
  {
    do{
      datalen=Transliterate(text,length,encoding,buf.get(),buflen);
      if(datalen==-1)
      {
        buflen*=2;
        buf=auto_ptr<char>(new char[buflen]);
      }
    }while(datalen==-1);
    if(datalen>160)
    {
      datalen=160;
      buf.get()[159]='.';
      buf.get()[158]='.';
      buf.get()[157]='.';
    }
    dc=DataCoding::DEFAULT;
  }
  sms->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,dc);
  sms->setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,buf.get(),datalen);
  sms->setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,datalen);
  return datalen;
}

void transLiterateSms(SMS* sms)
{
  char udhiData[256];
  int udhiDataLen=0;
  bool udhi=sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40;

  auto_ptr<char> buf;
  auto_ptr<char> buf8;

  unsigned len;
  const short*msg=(const short*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  bool pl=false;
  if(len==0)
  {
    if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      msg=(const short*)sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
      pl=true;
    }
  }
  if(udhi)
  {
    unsigned char* data=(unsigned char*)msg;
    udhiDataLen=*data;
    memcpy(udhiData,data,udhiDataLen);
    msg=(short*)data+udhiDataLen;
    len-=udhiDataLen;
  }

  buf=auto_ptr<char>(new char[len*2]);
  len=ConvertUCS2ToMultibyte(msg,len,buf.get(),len*2,CONV_ENCODING_CP1251);
  buf8=auto_ptr<char>(new char[udhiDataLen+len*3+1]);
  int newlen=Transliterate(buf.get(),len,CONV_ENCODING_CP1251,buf8.get()+udhiDataLen,len*3);
  sms->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);
  __trace2__("SUBMIT: converting ucs2->text(%d->%d)",len,newlen);
  if(udhi)
  {
    memcpy(buf8.get(),udhiData,udhiDataLen);
    newlen+=udhiDataLen;
  }
  if(pl)
  {
    sms->setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD,buf8.get(),newlen);
  }
  else
  {
    sms->setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,buf8.get(),newlen);
    sms->setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,newlen);
  }
}


};//util
};//smsc
