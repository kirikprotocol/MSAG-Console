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

int fillSms(SMS* sms,const char *text,int length,ConvEncodingEnum encoding,int datacoding)
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
  sms->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,dc);
  sms->setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,buf.get(),datalen);
  sms->setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,datalen);
  return datalen;
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
      datalen=140;
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
  char udhiData[257];
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
    udhiDataLen=*data+1;
    memcpy(udhiData,data,udhiDataLen);
    msg=(short*)(data+udhiDataLen);
    len-=udhiDataLen;
  }

  buf=auto_ptr<char>(new char[len*2]);
  len=ConvertUCS2ToMultibyte(msg,len,buf.get(),len*2,CONV_ENCODING_CP1251);
  buf8=auto_ptr<char>(new char[udhiDataLen+len*3+1]);
  int newlen=Transliterate(buf.get(),len,CONV_ENCODING_CP1251,buf8.get()+udhiDataLen,len*3);
  sms->setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);
  __trace2__("SUBMIT: converting ucs2->text(%d->%d)",len,newlen);
  if(udhi)
  {
    memcpy(buf8.get(),udhiData,udhiDataLen);
    newlen+=udhiDataLen;
  }
  if(pl)
  {
    sms->setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,buf8.get(),newlen);
  }
  else
  {
    sms->setBinProperty(Tag::SMPP_SHORT_MESSAGE,buf8.get(),newlen);
    sms->setIntProperty(Tag::SMPP_SM_LENGTH,newlen);
  }
}

int partitionSms(SMS* sms,int dstdc)
{
  unsigned int len;
  const char* msg=sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  int dc=sms->getIntProperty(Tag::SMPP_DATA_CODING);
  if(len==0 && sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    msg=sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
  }
  __trace2__("partitionSms: len=%d, dc=%d, dstdc=%d",len,dc,dstdc);
  bool udhi=sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40;
  int  udhilen=udhi?(1+*((unsigned char*)msg)):0;
  if(udhilen>140)return psErrorUdhi;
  auto_ptr<char> buf8;
  auto_ptr<char> bufTr;
  len-=udhilen;
  if(dc==DataCoding::UCS2 && dstdc!=DataCoding::UCS2)
  {
    buf8=auto_ptr<char>(new char[len]);
    short *data=(short*)(msg+udhilen);
    ConvertUCS2ToMultibyte(data,len,buf8.get(),len,CONV_ENCODING_CP1251);
    len/=2;
    bufTr=auto_ptr<char>(new char[udhilen+len*3+3]);
    len=Transliterate(buf8.get(),len,CONV_ENCODING_CP1251,bufTr.get()+udhilen,len*3);
    if(udhi)memcpy(bufTr.get(),msg,udhilen);
    msg=bufTr.get();
    dc=DataCoding::DEFAULT;
    //len+=udhilen;
  }
  int maxlen=134,maxfulllen=140;
  if(dc==DataCoding::DEFAULT)
  {
    int xlen=len;
    for(int i=0;i<len;i++)
    {
      switch(msg[udhilen+i])
      {
        case '{':
        case '}':
        case '[':
        case ']':
        case '~':
        case '\\':
        case '^':
        case '|':
         xlen++;
      }
    }
    __trace2__("PARTITIONSMS: udhilen=%d, xlen=%d",udhilen,xlen);
    xlen*=7;
    if(udhilen+xlen/8+(xlen%8?1:0)<=maxfulllen)return psSingle;
  }else
  if(dc==DataCoding::SMSC7BIT)
  {
    len*=7;
    if(udhilen+len/8+(len%8?1:0)<=maxfulllen)return psSingle;
  }
  else
  {
    if(udhilen+len<=maxfulllen)return psSingle;
  }
  if(udhi)return psErrorUdhi;

  int parts=1;
  uint16_t offsets[256];
  offsets[0]=0;
  if(dc==DataCoding::UCS2)
  {
    int lastword=0;
    int lastpos=0;
    for(int i=0;i<len/2;i++)
    {
      unsigned short c;
      memcpy(&c,msg+i*2,2);
      if((i-lastpos)>=67)
      {
        __trace2__("PARTITIONSMS: part=%d, i=%d, lastpos=%d, lastword=%d",parts,i,lastpos,lastword);
        if(i-lastword<67)
        {
          offsets[parts++]=(lastword+1)*2;
          lastpos=lastword+1;
        }else
        {
          offsets[parts++]=i*2;
          lastpos=i;
        }
        __trace2__("PARTITIONSMS: part=%d, off=%d",parts,offsets[parts-1]);
        if(parts>=256)return psErrorLength;
      }
      if(c<=32)lastword=i;
    }
  }else
  if(dc==DataCoding::BINARY)
  {
    parts=len/maxlen+((len%maxlen)?1:0);
    if(parts>=256)return psErrorLength;
    for(int i=0;i<parts;i++)offsets[i]=i*maxlen;
  }
  else
  {
    int lastword=0;
    int lastpos=0;
    int l=0,wl=0;
    for(int i=0;i<len;i++)
    {
      unsigned char c;
      c=msg[i];
      if(l>=153)
      {
        __trace2__("PARTITIONSMS: part=%d, l=%d, wl=%d",parts,l,wl);
        if(wl<153)
        {
          offsets[parts++]=lastword+(wl==0?0:1);
          lastpos=offsets[parts-1];
          i=lastpos+1;
          l=0;
        }else
        {
          if(l>153)i--;
          offsets[parts++]=i;
          lastpos=i;
          l=0;
        }
        __trace2__("PARTITIONSMS: part=%d, off=%d",parts-1,offsets[parts-1]);
        if(parts>=256)return psErrorLength;
      }
      wl++;
      l++;
      if(c==32 || c==10 || c==13)
      {
        lastword=i;
        wl=0;
      }
      if(dc==DataCoding::DEFAULT)
      {
        switch(msg[i])
        {
          case '{':
          case '}':
          case '[':
          case ']':
          case '~':
          case '\\':
          case '^':
          case '|':
          {
            l++;
            wl++;
          }
        }
      }
    }
  }
  __trace2__("PARTITIONSMS: len=%d, parts=%d",len,parts);
  char bin[1+256*2];
  bin[0]=(char)parts;
  memcpy(bin+1,offsets,parts*2);
  int blen=parts*2+1;
  sms->setBinProperty(Tag::SMSC_CONCATINFO,bin,blen);
  return psMultiple;
}

void extractSmsPart(SMS* sms,int partnum)
{
  if(!sms->hasBinProperty(Tag::SMSC_CONCATINFO))return;
  int dstdc=sms->getIntProperty(Tag::SMSC_DSTCODEPAGE);
  unsigned int len;
  int dc=sms->getIntProperty(Tag::SMPP_DATA_CODING);
  const char* msg=sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  if(len==0 && sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    msg=sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
  }
  __trace2__("extractSmsPart: len=%d, dc=%d, dstdc=%d",len,dc,dstdc);
  auto_ptr<char> buf8;
  auto_ptr<char> bufTr;
  if(dc==DataCoding::UCS2 && dstdc!=DataCoding::UCS2)
  {
    buf8=auto_ptr<char>(new char[len]);
    ConvertUCS2ToMultibyte((short*)msg,len,buf8.get(),len,CONV_ENCODING_CP1251);
    len/=2;
    bufTr=auto_ptr<char>(new char[len*3]);
    len=Transliterate(buf8.get(),len,CONV_ENCODING_CP1251,bufTr.get(),len*3);
    msg=bufTr.get();
  }
  int maxlen=134;
  if(dstdc==DataCoding::DEFAULT)
  {
    maxlen=153;
  }
  unsigned int cilen;
  ConcatInfo *ci=(ConcatInfo *)sms->getBinProperty(Tag::SMSC_CONCATINFO,&cilen);
  __require__(partnum<ci->num);
  int off=ci->off[partnum];
  int newlen=ci->num==partnum+1?len-off:ci->off[partnum+1]-off;
  __trace2__("extractSmsPart: newlen=%d, part=%d/%d, maxlen=%d, off=%d, partlen=%d",len,partnum,(int)ci->num,maxlen,off,newlen);
  __require__(newlen<=160);
  if(dc==DataCoding::UCS2 && dstdc!=DataCoding::UCS2)
  {
    sms->setIntProperty(Tag::SMPP_DATA_CODING,dstdc);
  }
  uint8_t buf[256];
  buf[0]=5;
  buf[1]=0;
  buf[2]=3;
  buf[3]=sms->getConcatMsgRef();
  buf[4]=ci->num;
  buf[5]=partnum+1;
  memcpy(buf+6,msg+off,newlen);
  newlen+=6;

  sms->setIntProperty(Tag::SMPP_ESM_CLASS,sms->getIntProperty(Tag::SMPP_ESM_CLASS)|0x40);
  sms->setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)buf,newlen);
  sms->setIntProperty(Tag::SMPP_SM_LENGTH,newlen);
  sms->getMessageBody().dropProperty(Tag::SMPP_MESSAGE_PAYLOAD);
  sms->getMessageBody().dropProperty(Tag::SMSC_RAW_PAYLOAD);
  sms->setIntProperty(Tag::SMPP_DATA_SM,0);
}

};//util
};//smsc
