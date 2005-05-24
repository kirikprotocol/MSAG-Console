package ru.novosoft.smsc.admin.smsexport;

import ru.novosoft.smsc.admin.smsview.*;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.admin.route.Mask;

import java.io.*;
import java.sql.Date;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 11.02.2005
 * Time: 18:23:01
 * To change this template use File | Settings | File Templates.
 */
public class SmsFileImport  extends Message
{
 // private SmsRow sms = null;
  private SqlSms sqlsms = null;
 // private SmsId smsId =null;
 // private boolean BodyRecived=false;
 // private boolean allReaded=false;
  public SmsFileImport()
  {
    super(Message.SMSC_BYTE_RSSMS_TYPE);

  }

/*  public SmsFileImport(SmsRow sms)
  {
    super(Message.SMSC_BYTE_RSSMS_TYPE);
    this.sms = sms;
  }

  public SmsRow getSms()
  {
    return sms;
  }
*/
  public SqlSms getSqlSms()
   {
     return sqlsms;
   }
/*
  public SmsId getSmsId()
  {
    return smsId;
  }
   */
  public static int readUInt8(BufferedInputStream is) throws IOException
   {
     int b = is.read();
     if (b == -1) throw new EOFException();
     return b;
   }

   public static int readUInt16(BufferedInputStream is) throws IOException
   {
     return (readUInt8(is) << 8 | readUInt8(is));
   }

   public static long readUInt32(BufferedInputStream is) throws IOException
   {
     return ((long) readUInt8(is) << 24) | ((long) readUInt8(is) << 16) |
             ((long) readUInt8(is) << 8) | ((long) readUInt8(is));
   }

   public static long readInt64(BufferedInputStream is) throws IOException
   {
     return (readUInt32(is) << 32) | readUInt32(is);
   }

  public static String readString(BufferedInputStream is, int size) throws IOException
   {
     if (size <= 0) return "";
     byte buff[] = new byte[size];
     int pos = 0;
     int cnt = 0;
     while (pos < size) {
       cnt = is.read(buff, pos, size - pos);
       if (cnt == -1) throw new EOFException();
       pos += cnt;
     }
     return new String(buff);
   }

   public static String readString8(BufferedInputStream is) throws IOException
   {
     return readString(is, readUInt8(is));
   }

   public static String readString16(BufferedInputStream is) throws IOException
   {
     return readString(is, readUInt16(is));
   }

   public static String readString32(BufferedInputStream is) throws IOException
   {
     return readString(is, (int) readUInt32(is));
   }
 /*
  public long receiveId(BufferedInputStream bis, long offset ) throws IOException
  {
    //sms = new SmsRow();
    smsId = new SmsId();
    int msgSize1 = (int)readUInt32(bis);
    offset += 4;
    //sms.setPointer(offset);
    smsId.setPointer(offset);
    long msgId = readInt64(bis);
    offset += 8;
    //sms.setId(msgId);
    smsId.setId(msgId);
    long nsz=msgSize1-8;
  //  long bufferCount=offset/bufferSize+1;
   // long temp1=bufferSize*bufferCount;
   // long avail=temp1-offset-nsz;
    //long avail=temp1-off-nsz;
   // if (avail<=0) {
      for (int i = 0; i < nsz; i++) {
         bis.read();
      }
   // }
    //else bis.skip(nsz);
    offset +=nsz;
   int msgSize2 = (int) readUInt32(bis);
    offset += 4;
    if (msgSize1 != msgSize2) {
      System.out.println("error sz1 sz2");
      throw new IOException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);
    }
    return offset;
  }
    public void skipSms(BufferedInputStream bis ) throws IOException
   {
     int msgSize1 = (int)readUInt32(bis);
     for (int i = 0; i < msgSize1; i++) {
         bis.read();
     }
     int msgSize2 = (int) Message.readUInt32(bis);
   }
   */
  public boolean receive(InputStream is,long msgId,boolean haveArc) throws IOException
    {
      int seq=(int)Message.readUInt32(is);
      String finall=Message.readString(is,1);

      int smsStatus = Message.readUInt8(is);
      Date submitTime = new Date(Message.readUInt32(is)*1000);

      long validTime = Message.readUInt32(is);
      long lastTryTime = Message.readUInt32(is);
      long nextTryTime = Message.readUInt32(is);
      int attempts = (int)Message.readUInt32(is);

      int lastResult = (int)Message.readUInt32(is);

      int strLen = Message.readUInt8(is);
      Mask origMask = new Mask((byte)Message.readUInt8(is), (byte)Message.readUInt8(is), Message.readString(is,strLen));

      strLen = Message.readUInt8(is);
      Mask destMask = new Mask((byte)Message.readUInt8(is), (byte)Message.readUInt8(is), Message.readString(is,strLen));

      strLen = Message.readUInt8(is);
      Mask dDestMask = new Mask((byte)Message.readUInt8(is), (byte)Message.readUInt8(is), Message.readString(is,strLen));

      int mr = Message.readUInt16(is);
      String sctype = Message.readString8(is);
      short dreport = (short) Message.readUInt8(is);
      short billr = (short) Message.readUInt8(is);
      String odMsc = Message.readString8(is);
      String odImsi = Message.readString8(is);
      int odSme = (int) Message.readUInt32(is);
      String ddMsc = Message.readString8(is);
      String ddImsi = Message.readString8(is);
      int ddSme = (int) Message.readUInt32(is);

      String routeId = Message.readString8(is);

      int serviceId = (int) Message.readUInt32(is);
      int priority = (int) Message.readUInt32(is);

      String srcSmeId = Message.readString8(is);
      String dstSmeId = Message.readString8(is);

      // parse rest and fill SmsRow
  /*    sms = new SmsRow();

      sms.setId(msgId);
     // sms.setSeq(seq);
      sms.setStatus(smsStatus);
      sms.setSubmitTime(submitTime);
      sms.setValidTime( new Date(validTime*1000));
      sms.setLastTryTime(new Date(lastTryTime*1000));
      sms.setNextTryTime(new Date(nextTryTime*1000));
      sms.setAttempts(attempts);
      sms.setLastResult(lastResult);
      sms.setOriginatingAddress(origMask);
      sms.setDestinationAddress(destMask);
      sms.setDealiasedDestinationAddress(dDestMask);
      sms.setMessageReference(mr);
      sms.setServiceType(sctype);
      sms.setDeliveryReport(dreport);
      sms.setBillingRecord(billr);
      sms.setOriginatingDescriptor(new SmsDescriptor(odImsi, odMsc, odSme));
      sms.setDestinationDescriptor(new SmsDescriptor(ddImsi, ddMsc, ddSme));
      sms.setRouteId(routeId);
      sms.setServiceId(serviceId);
      sms.setPriority(priority);
      sms.setSrcSmeId(srcSmeId);
      sms.setDstSmeId(dstSmeId);   */
      short concatMsgRef=(short)Message.readUInt16(is);
     // sms.setConcatMsgRef(concatMsgRef);
      short concatSeqNum=(short)Message.readUInt8(is);
    //  sms.setConcatSeqNum(concatSeqNum);
      byte arc=-1;
      if (haveArc)  {
        arc=((byte)Message.readUInt8(is));
      }
      int bodyLen = (int) Message.readUInt32(is);
    //  sms.setBodyLen(bodyLen);
      sqlsms = new SqlSms("SMS",msgId,smsStatus,submitTime,new Date(validTime*1000),attempts,
                lastResult,new Date(lastTryTime*1000),new Date(nextTryTime*1000),origMask.getMask(),destMask.getMask(),
                dDestMask.getMask(),mr,sctype,dreport,
                billr,odMsc,odImsi,odSme,ddMsc,
                ddImsi,ddSme,routeId,serviceId,priority,
               srcSmeId,dstSmeId,concatMsgRef,concatSeqNum,bodyLen);
      sqlsms.setArc(arc);
      if (bodyLen > 0) {
       // byte body[] = new byte[bodyLen];
        int pos = 0;
        int cnt = 0;
        while (pos < bodyLen) {
          cnt = is.read();//cnt = is.read(body, pos, bodyLen - pos);
          if (cnt == -1) { /*BodyRecived=true;*/throw new EOFException();
          };
          pos += cnt;
        }
       // sms.setBody(body);
      }
      return true;
    }

}
