#include "util/smstext.h"
#ifndef NOLOGGERPLEASE
#include "util/debug.h"
#else
#define __trace2__(...)
#define __warning2(...)
#endif
#include <memory>
#include "core/buffers/TmpBuf.hpp"


namespace smsc{
namespace util{

using namespace std;
using std::auto_ptr;

using smsc::core::buffers::TmpBuf;

static inline bool isEscapedChar(char c)
{
  return c=='|' || c=='{' || c=='}' || c=='[' || c==']' ||
         c=='\\'|| c=='~';
}

static inline int countEscapedChars(const char* text,size_t length)
{
  int res=0;
  for(size_t i=0;i<length;i++)
  {
    if(isEscapedChar(text[i]))res++;
  }
  return res;
}

int splitSms(SMS* tmplSms,const char *text,size_t length,ConvEncodingEnum encoding,int datacoding,Array<SMS*>& dest)
{
  dest.Clean();
  size_t buflen=length*2+4;
  auto_ptr<char> buf(new char[buflen]);
  bool hb=hasHighBit(text,length);
  int dc;
  size_t datalen;
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
    dc=DataCoding::LATIN1;
  }
  __trace2__("splitSms:dc=%d",dc);
  size_t maxlen=dc==DataCoding::LATIN1?160:140;
  size_t sent=0;
  while(sent<datalen)
  {
    SMS *s=new SMS(*tmplSms);
    size_t piece=datalen-sent>maxlen?maxlen:datalen-sent;
    if(dc==DataCoding::LATIN1 && piece==maxlen)piece-=countEscapedChars(buf.get()+sent,piece);
    s->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,dc);
    s->setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,buf.get()+sent,(unsigned)piece);
    s->setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,(unsigned)piece);
    dest.Push(s);
    sent+=piece;
  }
  return dest.Count();
}

size_t fillSms(SMS* sms,const char *text,size_t length,ConvEncodingEnum encoding,int datacoding,int trimLen)
{
  size_t buflen=length*2+4;
  auto_ptr<char> buf(new char[buflen]);
  bool hb=hasHighBit(text,length);
  int dc;
  size_t datalen;
  if(hb && (datacoding&DataCoding::UCS2)==DataCoding::UCS2)
  {
    ConvertMultibyteToUCS2(text,length,(short*)buf.get(),buflen,encoding);
    dc=DataCoding::UCS2;
    if(trimLen)
    {
      if(length<=trimLen/2)
      {
        datalen=length*2;
      }else
      {
        datalen=trimLen;
        short d[1];
        ConvertMultibyteToUCS2(".",1,d,1,CONV_ENCODING_ANSI);
        short *b=(short*)buf.get();
        b[trimLen/2-1]=d[0];
        b[trimLen/2-2]=d[0];
        b[trimLen/2-3]=d[0];
      }
    }else
    {
      datalen=length*2;
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
    if((datacoding&DataCoding::LATIN1)==DataCoding::LATIN1)
    {
      dc=DataCoding::LATIN1;
    }else
    {
      char *buf2=new char[datalen*2];
      datalen=ConvertLatin1ToSMSC7Bit(buf.get(),datalen,buf2);
      buf=auto_ptr<char>(buf2);
      dc=DataCoding::SMSC7BIT;
    }
    if(trimLen)
    {
      trimLen*=8;
      trimLen/=7;
      if(datalen>trimLen)
      {
        datalen=trimLen;
        buf.get()[trimLen-1]='.';
        buf.get()[trimLen-2]='.';
        buf.get()[trimLen-3]='.';
      }
    }
  }
  sms->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,dc);
  sms->getMessageBody().dropProperty(Tag::SMPP_MESSAGE_PAYLOAD);
//  sms->getMessageBody().dropProperty(Tag::SMSC_RAW_PAYLOAD);
  sms->getMessageBody().dropProperty(Tag::SMPP_SHORT_MESSAGE);
  //sms->getMessageBody().dropProperty(Tag::SMSC_RAW_SHORTMESSAGE);
  sms->getMessageBody().dropIntProperty(Tag::SMPP_SM_LENGTH);
  if(datalen>255)
  {
    //sms->setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,"",0);
    sms->setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,0);
    sms->setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD,buf.get(),(unsigned)datalen);
  }else
  {
    sms->setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,buf.get(),(unsigned)datalen);
    sms->setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,(unsigned)datalen);
  }
  return datalen;
}


void transLiterateSms(SMS* sms,int datacoding)
{
  char udhiData[257];
  int udhiDataLen=0;
  bool udhi=sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40;

  TmpBuf<char,256> buf(0);
  TmpBuf<char,256> buf8(0);

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

  buf.setSize(len*2);

  int newlen;

  int dc;

  if(sms->getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
  {
    len=ConvertUCS2ToMultibyte(msg,len,buf.get(),len*2,CONV_ENCODING_CP1251);
    buf8.setSize(udhiDataLen+len*4+1);
    newlen=Transliterate(buf.get(),len,CONV_ENCODING_CP1251,buf8.get()+udhiDataLen,len*3+1);
    dc=DataCoding::LATIN1;
    if(datacoding==DataCoding::SMSC7BIT)
    {
      buf.setSize(newlen*2+1);
      newlen=ConvertLatin1ToSMSC7Bit(buf8.get()+udhiDataLen,newlen,buf.get());
      memcpy(buf8.get()+udhiDataLen,buf.get(),newlen);
      dc=DataCoding::SMSC7BIT;
    }
  }else if(sms->getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::LATIN1)
  {
    buf8.setSize(udhiDataLen+len*2+1);
    newlen=ConvertLatin1ToSMSC7Bit((char*)msg,len,buf8.get()+udhiDataLen);
    dc=DataCoding::SMSC7BIT;
  }else
  {
    return;
  }

  sms->setIntProperty(Tag::SMPP_DATA_CODING,dc);
  __trace2__("SUBMIT: converting %d->%d (%d->%d)",sms->getIntProperty(Tag::SMPP_DATA_CODING),dc,len,newlen);
  if(udhi)
  {
    memcpy(buf8.get(),udhiData,udhiDataLen);
    newlen+=udhiDataLen;
  }
  if(!pl && newlen>254)
  {
    sms->dropProperty(Tag::SMPP_SHORT_MESSAGE);
    sms->getMessageBody().dropIntProperty(Tag::SMPP_SM_LENGTH);
    pl=true;
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

static int CalcExUserDataLen(SMS* sms)
{
  if(sms->hasIntProperty(Tag::SMPP_SOURCE_PORT) && sms->hasIntProperty(Tag::SMPP_DESTINATION_PORT))
  {
    int rv=1+1;//tag+len
    if(sms->getIntProperty(Tag::SMPP_SOURCE_PORT)<256 &&
       sms->getIntProperty(Tag::SMPP_DESTINATION_PORT)<256)
    {
      rv+=2; //8 bit ports
    }else
    {
      rv+=4; //16 bit ports
    }
    return rv;
  }
  return 0;
}

static bool FillUd(SMS* sms)
{
  unsigned char userdata[256];
  unsigned int len;
  unsigned int udhlen=0;
  const char* msg=sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  if(len==0 && sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    msg=sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
  }
  unsigned int off=1;
  unsigned int oldUdh = 0;
  if(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
  {
    udhlen=*((unsigned char*)msg);
    oldUdh = 1;
  }
  bool processed=false;
  if(sms->hasIntProperty(Tag::SMPP_SOURCE_PORT))
  {
    if(sms->getIntProperty(Tag::SMPP_SOURCE_PORT)<256 &&
       sms->getIntProperty(Tag::SMPP_DESTINATION_PORT)<256)
    {
      if(len+5-oldUdh > 255)
      {
        __trace__("fillUd: estimated length>255");
        return false;
      }
      userdata[off++]=4;
      userdata[off++]=2;
      userdata[off++]=sms->getIntProperty(Tag::SMPP_DESTINATION_PORT);
      userdata[off++]=sms->getIntProperty(Tag::SMPP_SOURCE_PORT);
    }else
    {
      if(len+7-oldUdh>255)
      {
        __trace__("fillUd: estimated length>255");
        return false;
      }
      userdata[off++]=5;
      userdata[off++]=4;
      unsigned short tmp=htons(sms->getIntProperty(Tag::SMPP_DESTINATION_PORT));
      memcpy(userdata+off,&tmp,2);
      off+=2;
      tmp=htons(sms->getIntProperty(Tag::SMPP_SOURCE_PORT));
      memcpy(userdata+off,&tmp,2);
      off+=2;
    }
    processed=true;
  }
  if(processed)
  {
    if(len==0)return false;
    userdata[0]=udhlen+off-1;
    char buf[256];
    memcpy(buf,userdata,off);
    memcpy(buf+off,msg+oldUdh,len-oldUdh);
    len+=off-oldUdh;
    sms->setIntProperty(Tag::SMPP_ESM_CLASS,sms->getIntProperty(Tag::SMPP_ESM_CLASS)|0x40);
    if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      sms->setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,buf,len);
      sms->setIntProperty(Tag::SMPP_SM_LENGTH,0);
    }else
    {
      sms->setBinProperty(Tag::SMPP_SHORT_MESSAGE,buf,len);
      sms->setIntProperty(Tag::SMPP_SM_LENGTH,len);
    }
  }
  return true;
}

int partitionSms(SMS* sms)
{
  int dstdc=sms->getIntProperty(Tag::SMSC_DSTCODEPAGE);
  unsigned int len;
  const char* msg=sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  int dc=sms->getIntProperty(Tag::SMPP_DATA_CODING);
  if(len==0 && sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    msg=sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
  }
  __trace2__("partitionSms: len=%ld, dc=%d, dstdc=%d",len,dc,dstdc);
  bool udhi=sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40;
  int  udhilen=udhi?(1+*((unsigned char*)msg)):0;
  int maxfulllen=140;
  if(sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    maxfulllen=160;
  }

  if(udhilen>maxfulllen)return psErrorUdhi;

  auto_ptr<char> buf8;
  auto_ptr<char> bufTr;
  len-=udhilen;
  if(dc==DataCoding::UCS2 && (dstdc&DataCoding::UCS2)!=DataCoding::UCS2)
  {
    buf8=auto_ptr<char>(new char[len]);
    short *data=(short*)(msg+udhilen);
    ConvertUCS2ToMultibyte(data,len,buf8.get(),len,CONV_ENCODING_CP1251);
    len/=2;
    bufTr=auto_ptr<char>(new char[udhilen+len*3+3]);
    len=Transliterate(buf8.get(),len,CONV_ENCODING_CP1251,bufTr.get()+udhilen,len*3+1);
    if(udhi)memcpy(bufTr.get(),msg,udhilen);
    msg=bufTr.get();
    if(dstdc==DataCoding::SMSC7BIT)
    {
      auto_ptr<char> buf7(new char[len*2+udhilen+1]);
      len=ConvertLatin1ToSMSC7Bit(bufTr.get()+udhilen,len,buf7.get());
      if(udhi)memcpy(buf7.get(),bufTr.get(),udhilen);
      bufTr=buf7;
      msg=bufTr.get();
      dc=DataCoding::SMSC7BIT;
    }else
    {
      dc=DataCoding::LATIN1;
    }
    //len+=udhilen;
  }
  if(!(len>=0 && len<=65535))
  {
    __warning2__("udhlen=%d, data len=%d",udhilen,len);
    return psErrorLength;
  }
  int maxlen=134;
  int exudhilen=CalcExUserDataLen(sms);
  //if(udhi && exudhilen)return psErrorUdhi;

  if(exudhilen>0)
  {
    if(udhilen>0)udhilen+=exudhilen;
    else udhilen=exudhilen+1;
  }
  int rv=psErrorUdhi;
  if(dc==DataCoding::LATIN1)
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
    if(udhilen+xlen/8+(xlen%8?1:0)<=maxfulllen)
    {
      rv=psSingle;
    }
  }else
  if(dc==DataCoding::SMSC7BIT)
  {
    int xlen=len*7;
    if(udhilen+xlen/8+(xlen%8?1:0)<=maxfulllen)
    {
      rv=psSingle;
    }
  }
  else
  {
    if(udhilen+len<=maxfulllen)
    {
      rv=psSingle;
    }
  }
  if(rv==psSingle)
  {
    if(!FillUd(sms))return psErrorUdhi;
    return rv;
  }
  if(udhi)return psErrorUdhi;
  if(sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP))return psErrorUssd;

  int parts=1;
  uint16_t offsets[256];
  offsets[0]=0;
  if(sms->getIntProperty(Tag::SMSC_UDH_CONCAT) || dc==DataCoding::BINARY)
  {
    maxlen=140-6-exudhilen;
  }else
  {
    int hdrlen=0;
    if(dc==DataCoding::UCS2)
    {
      int estimate=len/(140-exudhilen-6);
      if(estimate>10)
      {
        hdrlen=7*2;
      }else
      {
        hdrlen=5*2;
      }
    }else
    if(dc!=DataCoding::BINARY)
    {
      hdrlen=6;
    }
    maxlen=140-exudhilen-hdrlen;
  }

  if(dc==DataCoding::UCS2)
  {
    int lastword=0;
    int lastpos=0;
    //maxlen&=~1;
    maxlen/=2;
    for(int i=0;i<len/2;i++)
    {
      unsigned short c;
      memcpy(&c,msg+i*2,2);
      if((i-lastpos)>=maxlen)
      {
        __trace2__("PARTITIONSMS: part=%d, i=%d, lastpos=%d, lastword=%d",parts,i,lastpos,lastword);
        if(c<=32 || sms->getIntProperty(Tag::SMSC_UDH_CONCAT))
        {
          offsets[parts++]=i*2;
          lastpos=i;
        }else
        if(i-lastword<maxlen)
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
  else //LATIN1&SMSC7BIT
  {
    int lastword=0;
    int lastpos=0;
    int lastEsc=-2;
    int l=0,wl=0;
    maxlen*=8;
    maxlen/=7;
    for(int i=0;i<len;i++)
    {
      unsigned char c;
      c=msg[i];
      if(l>=maxlen)
      {
        __trace2__("PARTITIONSMS: part=%d, l=%d, wl=%d",parts,l,wl);
        if(wl<maxlen || sms->getIntProperty(Tag::SMSC_UDH_CONCAT))
        {
          if(i<len-1 && (c==32 || c==10 || c==13 || sms->getIntProperty(Tag::SMSC_UDH_CONCAT)))
          {
            if(i==lastEsc-1)
            {
              i--;
            }
            offsets[parts++]=i;
            lastword=i;
            wl=0;
            lastpos=i;
          }else
          {
            offsets[parts++]=lastword+(wl==0?0:1);
            lastpos=offsets[parts-1];
            i=lastpos;
            wl=0;
          }
          l=0;
        }else
        {
          if(l>maxlen)i--;
          __trace2__("PARTITIONSMS: cut at %d (lastEsc=%d)",i,lastEsc);
          if(i==lastEsc+1)i--;
          offsets[parts++]=i;
          lastpos=i;
          l=0;
          wl=0;
        }
        __trace2__("PARTITIONSMS: part=%d, off=%d",parts-1,offsets[parts-1]);
        if(parts>=256)return psErrorLength;
      }
      c=msg[i];
      wl++;
      l++;
      if(c==32 || c==10 || c==13)
      {
        lastword=i;
        wl=0;
      }
      if(dc==DataCoding::LATIN1 && dstdc==DataCoding::SMSC7BIT)
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
      }else //SMSC7BIT
      {
        if(msg[i]==0x1b && lastEsc!=i-1)
        {
          lastEsc=i;
          __trace2__("PARTITIONSMS: lastEsc=%d",i);
        }
      }
    }
  }
  __trace2__("PARTITIONSMS: len=%d, parts=%d",len,parts);
  char bin[1+256*2];
  ConcatInfo *ci=(ConcatInfo*)bin;
  ci->num=parts;
  //bin[0]=(char)parts;
  //memcpy(bin+1,offsets,parts*2);
  for(int i=0;i<parts;i++)ci->setOff(i,offsets[i]);
  int blen=parts*2+1;
  sms->setBinProperty(Tag::SMSC_CONCATINFO,bin,blen);
  return psMultiple;
}

bool extractSmsPart(SMS* sms,int partnum)
{
  if(!sms->hasBinProperty(Tag::SMSC_CONCATINFO))return false;
  int dstdc=sms->getIntProperty(Tag::SMSC_DSTCODEPAGE);
  int dc=sms->getIntProperty(Tag::SMPP_DATA_CODING);
  if(sms->hasBinProperty(Tag::SMSC_DC_LIST))
  {
    unsigned len;
    unsigned char* dcList=(unsigned char*)sms->getBinProperty(Tag::SMSC_DC_LIST,&len);
    if(partnum<len)
    {
      dc=dcList[partnum];
    }
  }
  unsigned int len;
  const char* msg=sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  if(len==0 && sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    msg=sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
  }
  if(!sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
  {
    __trace2__("extractSmsPart: len=%d, dc=%d, dstdc=%d",len,dc,dstdc);
    auto_ptr<char> buf8;
    auto_ptr<char> bufTr;
    if(dc==DataCoding::UCS2 && (dstdc&DataCoding::UCS2)!=DataCoding::UCS2)
    {
      buf8=auto_ptr<char>(new char[len]);
      ConvertUCS2ToMultibyte((short*)msg,len,buf8.get(),len,CONV_ENCODING_CP1251);
      len/=2;
      bufTr=auto_ptr<char>(new char[len*3+1]);
      len=Transliterate(buf8.get(),len,CONV_ENCODING_CP1251,bufTr.get(),len*3+1);
      msg=bufTr.get();
      if(dstdc==DataCoding::SMSC7BIT)
      {
        auto_ptr<char> buf7(new char[len*2+1]);
        len=ConvertLatin1ToSMSC7Bit(bufTr.get(),len,buf7.get());
        bufTr=buf7;
        msg=bufTr.get();
      }
    }
    int maxlen=134;
    if(dstdc==DataCoding::LATIN1 || dstdc==DataCoding::SMSC7BIT)
    {
      maxlen=152;
    }
    unsigned int cilen;
    ConcatInfo *ci=(ConcatInfo *)sms->getBinProperty(Tag::SMSC_CONCATINFO,&cilen);
    if(!(partnum<ci->num))return false;
    int off=ci->getOff(partnum);
    int newlen=ci->num==partnum+1?len-off:ci->getOff(partnum+1)-off;
    __trace2__("extractSmsPart: newlen=%d, part=%d/%d, maxlen=%d, off=%d, partlen=%d",len,partnum,(int)ci->num,maxlen,off,newlen);
    if(!(newlen>0 && newlen<=160))
    {
      __warning2__("INVALID PART LEN=%d",newlen);
      return false;
    }
    if(dc==DataCoding::UCS2 && (dstdc&DataCoding::UCS2)!=DataCoding::UCS2)
    {
      //if(dstdc==DataCoding::UCS2|DataCoding::LATIN1)dstdc=DataCoding::LATIN1;
      sms->setIntProperty(Tag::SMPP_DATA_CODING,dstdc);
      if(sms->hasIntProperty(Tag::SMSC_ORIGINAL_DC))
      {
        int orgdc=sms->getIntProperty(Tag::SMSC_ORIGINAL_DC);
        int olddc=orgdc;
        if((orgdc&0xc0)==0 || (orgdc&0xf0)==0xf0) //groups 00xx and 1111
        {
          orgdc&=0xf3; //11110011 - clear 2-3 bits (set alphabet to default).

        }else if((orgdc&0xf0)==0xe0)
        {
          orgdc=0xd0 | (orgdc&0x0f);
        }
        sms->setIntProperty(Tag::SMSC_ORIGINAL_DC,orgdc);
        __trace2__("extractSmsPart: transliterate olddc(%x)->dc(%x)",olddc,orgdc);
      }
    }
    uint8_t buf[256];

    bool haveudh=true;
    if(!sms->getIntProperty(Tag::SMSC_UDH_CONCAT) && dc!=DataCoding::BINARY)
    {
      char hdrbuf[32];
      int pfx;
      if(ci->num<100)
        pfx=sprintf(hdrbuf,"(%d/%d)",partnum+1,(int)ci->num);
      else
        pfx=sprintf(hdrbuf,"%d:",partnum+1);
      if(dc==DataCoding::UCS2 && (dstdc&DataCoding::UCS2)==DataCoding::UCS2)
      {
        ConvertMultibyteToUCS2(hdrbuf,pfx,(short*)buf,256,CONV_ENCODING_ANSI);
        pfx*=2;
      }else
      /*
      if(dc==DataCoding::SMSC7BIT)
      {
        pfx=ConvertLatin1ToSMSC7Bit(hdrbuf,pfx,(char*)buf);
      }else*/
      {
        memcpy(buf,hdrbuf,pfx);
      }
      memcpy(buf+pfx,msg+off,newlen);
      newlen+=pfx;
      haveudh=false;
    }else
    {
      //if(sms->getConcatMsgRef()<256)
      {
        buf[0]=5;
        buf[1]=0;
        buf[2]=3;
        buf[3]=sms->getConcatMsgRef();
        buf[4]=ci->num;
        buf[5]=partnum+1;
        memcpy(buf+6,msg+off,newlen);
        newlen+=6;
      }
      /*else
      {
        buf[0]=6;
        buf[1]=8;
        buf[2]=4;
        buf[3]=sms->getConcatMsgRef()>>8;
        buf[4]=sms->getConcatMsgRef()&0xff;
        buf[5]=ci->num;
        buf[6]=partnum+1;
        memcpy(buf+7,msg+off,newlen);
        newlen+=7;
      }
      */
    }

    if(haveudh)
      sms->setIntProperty(Tag::SMPP_ESM_CLASS,sms->getIntProperty(Tag::SMPP_ESM_CLASS)|0x40);
    sms->getMessageBody().dropProperty(Tag::SMSC_CONCATINFO);
    sms->setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)buf,newlen);
    sms->setIntProperty(Tag::SMPP_SM_LENGTH,newlen);
    sms->getMessageBody().dropProperty(Tag::SMPP_MESSAGE_PAYLOAD);
    //sms->getMessageBody().dropProperty(Tag::SMSC_RAW_PAYLOAD);
    sms->setIntProperty(Tag::SMPP_DATA_SM,0);
    FillUd(sms);
  }else
  {
    __trace2__("extractSmsPart: (unmerge) len=%d, dc=%d, dstdc=%d",len,dc,dstdc);
    unsigned int cilen;
    ConcatInfo *ci=(ConcatInfo *)sms->getBinProperty(Tag::SMSC_CONCATINFO,&cilen);
    int off=ci->getOff(partnum);
    int newlen=ci->num==partnum+1?len-off:ci->getOff(partnum+1)-off;
    __trace2__("extractSmsPart: (unmerge) newlen=%d, part=%d/%d, off=%d, partlen=%d",len,partnum,(int)ci->num,off,newlen);
    if(sms->hasStrProperty(Tag::SMSC_FORWARD_MO_TO))
    {
      sms->setBinProperty(Tag::SMSC_MO_PDU,msg+off,newlen);
    }else
    {
      msg+=off;
      int udhlen=*((unsigned char*)msg)+1;
      len=newlen;
      auto_ptr<char> buf8;
      auto_ptr<char> bufTr;
      if(dc==DataCoding::UCS2 && (dstdc&DataCoding::UCS2)!=DataCoding::UCS2)
      {
        buf8=auto_ptr<char>(new char[len]);
        ConvertUCS2ToMultibyte((short*)(msg+udhlen),len-udhlen,buf8.get(),len,CONV_ENCODING_CP1251);
        len=(len-udhlen)/2;
        bufTr=auto_ptr<char>(new char[udhlen+len*3+1]);
        len=Transliterate(buf8.get(),len,CONV_ENCODING_CP1251,bufTr.get()+udhlen,len*3+1);
        memcpy(bufTr.get(),msg,udhlen);
        len+=udhlen;
        msg=bufTr.get();
        dc=DataCoding::LATIN1;
        if(dstdc==DataCoding::SMSC7BIT)
        {
          auto_ptr<char> buf7(new char[len*2+1+udhlen]);
          len=ConvertLatin1ToSMSC7Bit(bufTr.get()+udhlen,len-udhlen,buf7.get()+udhlen);
          memcpy(buf7.get(),bufTr.get(),udhlen);
          bufTr=buf7;
          len+=udhlen;
          msg=bufTr.get();
          if(len>udhlen+(len-udhlen)*8/7)len=udhlen+(len-udhlen)*8/7;
          dc=DataCoding::SMSC7BIT;
        }
        if(len>140)len=140;
        if(sms->hasIntProperty(Tag::SMSC_ORIGINAL_DC))
        {
          int orgdc=sms->getIntProperty(Tag::SMSC_ORIGINAL_DC);
          int olddc=orgdc;
          if((orgdc&0xc0)==0 || (orgdc&0xf0)==0xf0) //groups 00xx and 1111
          {
            orgdc&=0xf3; //11110011 - clear 2-3 bits (set alphabet to default).

          }else if((orgdc&0xf0)==0xe0)
          {
            orgdc=0xd0 | (orgdc&0x0f);
          }
          sms->setIntProperty(Tag::SMSC_ORIGINAL_DC,orgdc);
          __trace2__("extractSmsPart: transliterate olddc(%x)->orgdc(%x)",olddc,orgdc);
        }
      }
      sms->setIntProperty(Tag::SMPP_DATA_CODING,dc);
      sms->setIntProperty(Tag::SMPP_ESM_CLASS,sms->getIntProperty(Tag::SMPP_ESM_CLASS)|0x40);
      sms->getMessageBody().dropProperty(Tag::SMSC_CONCATINFO);
      sms->setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)msg,len);
      sms->setIntProperty(Tag::SMPP_SM_LENGTH,len);
      sms->getMessageBody().dropProperty(Tag::SMPP_MESSAGE_PAYLOAD);
      //sms->getMessageBody().dropProperty(Tag::SMSC_RAW_PAYLOAD);
      sms->setIntProperty(Tag::SMPP_DATA_SM,0);
      FillUd(sms);
    }
  }
  return true;
}

}//util
}//smsc
