package ru.novosoft.smsc.admin.smsview.operative;

import ru.novosoft.smsc.admin.smsview.SmsRow;
import ru.novosoft.smsc.admin.smsview.SmsDescriptor;
import ru.novosoft.smsc.admin.smsview.SmsSource;
import ru.novosoft.smsc.admin.smsview.SmsId;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsview.operative.Message;

import java.io.*;
import java.util.Date;
import java.util.Set;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 11.02.2005
 * Time: 18:23:01
 * To change this template use File | Settings | File Templates.
 */
public class RsFileMessage extends Message
{
  private SmsRow sms = null;
  private SmsId smsId =null;
  private boolean BodyRecived = false;
  private boolean allReaded = false;
  public RsFileMessage()
  {
    super(Message.SMSC_BYTE_RSSMS_TYPE);

  }

  public RsFileMessage(SmsRow sms,SmsId smsId )
  {
    super(Message.SMSC_BYTE_RSSMS_TYPE);
    this.sms = sms;
    this.smsId = smsId;
  }

  public SmsRow getSms()
  {
    return sms;
  }

  public SmsId getSmsId()
  {
    return smsId;
  }

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
    public  void receiveSms(BufferedInputStream bis ) throws IOException
   {
      sms = new SmsRow();
      int msgSize1 = (int)readUInt32(bis);
      long msgId=Message.readInt64(bis);
      sms.setId(msgId);
      int seq=(int)readUInt32(bis);
      String finall=readString(bis,1);
      sms.setStatus(readUInt8(bis));
      sms.setSubmitTime(new Date(readUInt32(bis)*1000));
      sms.setValidTime(new Date(readUInt32(bis)*1000));
      sms.setLastTryTime(new Date(readUInt32(bis)*1000));
      sms.setNextTryTime(new Date(readUInt32(bis)*1000));
      sms.setAttempts((int) readUInt32(bis));
      sms.setLastResult((int) readUInt32(bis));
      try {
        int len=readUInt8(bis);
        String type=readString(bis,1);
        String plan=readString(bis,1);
        String address=readString(bis,len);
        sms.setOriginatingAddress(address);
      } catch (AdminException e) {
        throw new IOException(e.getMessage());
      }
      try {
        int len=readUInt8(bis);
        String type=readString(bis,1);
        String plan=readString(bis,1);
        String address=readString(bis,len);
        sms.setDestinationAddress(address);
      } catch (AdminException e) {
        throw new IOException(e.getMessage());
      }
      try {
        int len=readUInt8(bis);
        String type=readString(bis,1);
        String plan=readString(bis,1);
        String address=readString(bis,len);
        sms.setDealiasedDestinationAddress(address);
      } catch (AdminException e) {
        throw new IOException(e.getMessage());
      }
      sms.setMessageReference(readUInt16(bis));
      sms.setServiceType(readString8(bis));
      sms.setDeliveryReport((short) readUInt8(bis));
      sms.setBillingRecord((short) readUInt8(bis));
      String odMsc = readString8(bis);
      String odImsi = readString8(bis);
      sms.setOriginatingDescriptor(new SmsDescriptor(odImsi, odMsc, (int) readUInt32(bis)));
      String ddMsc = readString8(bis);
      String ddImsi = readString8(bis);
      sms.setDestinationDescriptor(new SmsDescriptor(ddImsi, ddMsc, (int) readUInt32(bis)));
      sms.setRouteId(readString8(bis));
      sms.setServiceId((int) readUInt32(bis));
      sms.setPriority((int) readUInt32(bis));
      sms.setSrcSmeId(readString8(bis));
      sms.setDstSmeId(readString8(bis));
      int concatMsgRef=readUInt16(bis);
      String concatSeqNum=readString(bis,1);
      int bodyLen = (int) readUInt32(bis);
      if (bodyLen > 0) {
        byte body[] = new byte[bodyLen];
        int pos = 0;
        int cnt = 0;
        while (pos < bodyLen) {
          cnt = bis.read(body, pos, bodyLen - pos);
          if (cnt == -1) { BodyRecived=true;throw new EOFException();
          };
          pos += cnt;
        }
        SmsSource.parseBody(new ByteArrayInputStream(body, 0, bodyLen), sms);
      }
     int msgSize2 = (int) Message.readUInt32(bis);

    }
  /*
   public void receiveId(RandomAccessFile bis) throws IOException
    {
    sms = new SmsRow();
    long pointer=bis.getFilePointer();
    sms.setPointer(pointer);
    int msgSize1 = (int) Message.readUInt32(bis);
    pointer=bis.getFilePointer();
    long msgId=Message.readInt64(bis);
    sms.setId(msgId);
    bis.seek(msgSize1+pointer);
    int msgSize2 = (int) Message.readUInt32(bis);
    if (msgSize1 != msgSize2)
       throw new IOException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);
    }

   public void receiveSms(RandomAccessFile bis , long pointer) throws IOException
   {  bis.seek(pointer);
      sms = new SmsRow();
      int msgSize1 = (int) Message.readUInt32(bis);

      long msgId=Message.readInt64(bis);
      sms.setId(msgId);
      int seq=(int)Message.readUInt32(bis);
      String finall=Message.readString(bis,1);
      sms.setStatus(Message.readUInt8(bis));
      sms.setSubmitTime(new Date(Message.readUInt32(bis)*1000));
      sms.setValidTime(new Date(Message.readUInt32(bis)*1000));
      sms.setLastTryTime(new Date(Message.readUInt32(bis)*1000));
      sms.setNextTryTime(new Date(Message.readUInt32(bis)*1000));
      sms.setAttempts((int) Message.readUInt32(bis));
      sms.setLastResult((int) Message.readUInt32(bis));
      try {
        int len=Message.readUInt8(bis);
        String type=Message.readString(bis,1);
        String plan=Message.readString(bis,1);
        String address=Message.readString(bis,len);
        sms.setOriginatingAddress(address);
      } catch (AdminException e) {
        throw new IOException(e.getMessage());
      }
      try {
        int len=Message.readUInt8(bis);
        String type=Message.readString(bis,1);
        String plan=Message.readString(bis,1);
        String address=Message.readString(bis,len);
        sms.setDestinationAddress(address);
      } catch (AdminException e) {
        throw new IOException(e.getMessage());
      }
      try {
        int len=Message.readUInt8(bis);
        String type=Message.readString(bis,1);
        String plan=Message.readString(bis,1);
        String address=Message.readString(bis,len);
        sms.setDealiasedDestinationAddress(address);
      } catch (AdminException e) {
        throw new IOException(e.getMessage());
      }
      sms.setMessageReference(Message.readUInt16(bis));
      sms.setServiceType(Message.readString8(bis));
      sms.setDeliveryReport((short) Message.readUInt8(bis));
      sms.setBillingRecord((short) Message.readUInt8(bis));
      String odMsc = Message.readString8(bis);
      String odImsi = Message.readString8(bis);
      sms.setOriginatingDescriptor(new SmsDescriptor(odImsi, odMsc, (int) Message.readUInt32(bis)));
      String ddMsc = Message.readString8(bis);
      String ddImsi = Message.readString8(bis);
      sms.setDestinationDescriptor(new SmsDescriptor(ddImsi, ddMsc, (int) Message.readUInt32(bis)));
      sms.setRouteId(Message.readString8(bis));
      sms.setServiceId((int) Message.readUInt32(bis));
      sms.setPriority((int) Message.readUInt32(bis));
      sms.setSrcSmeId(Message.readString8(bis));
      sms.setDstSmeId(Message.readString8(bis));
      int concatMsgRef=Message.readUInt16(bis);
      String concatSeqNum=Message.readString(bis,1);
      int bodyLen = (int) Message.readUInt32(bis);
      if (bodyLen > 0) {
        byte body[] = new byte[bodyLen];
        int pos = 0;
        int cnt = 0;
        while (pos < bodyLen) {
          cnt = bis.read(body, pos, bodyLen - pos);
          if (cnt == -1) { BodyRecived=true;throw new EOFException();
          };
          pos += cnt;
        }
        SmsSource.parseBody(new ByteArrayInputStream(body, 0, bodyLen), sms);
      }
    }
  */
 /*
  public void receive(InputStream bis) throws IOException
  {
    sms = new SmsRow();
    int msgSize1 = (int) Message.readUInt32(bis);
    byte message[] = new byte[msgSize1];
    int read = 0;
    while (read < msgSize1) {
      int result = bis.read(message, read, msgSize1 - read);
      if (result < 0)
        throw new IOException("Protocol error. Failed to read " + msgSize1 + " bytes");
      read += result;
    }
    InputStream is = new ByteArrayInputStream(message);
    int msgSize2 = (int) Message.readUInt32(bis);
    if (msgSize1 != msgSize2)
      throw new IOException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);
    long msgId = Message.readInt64(is);
    sms.setId(msgId);
    int seq = (int) Message.readUInt32(is);
    String finall = Message.readString(is, 1);
    sms.setStatus(Message.readUInt8(is));
    sms.setSubmitTime(new Date(Message.readUInt32(is) * 1000));
    sms.setValidTime(new Date(Message.readUInt32(is) * 1000));
    sms.setLastTryTime(new Date(Message.readUInt32(is) * 1000));
    sms.setNextTryTime(new Date(Message.readUInt32(is) * 1000));
    sms.setAttempts((int) Message.readUInt32(is));
    sms.setLastResult((int) Message.readUInt32(is));
    try {
      int len = Message.readUInt8(is);
      String type = Message.readString(is, 1);
      String plan = Message.readString(is, 1);
      String address = Message.readString(is, len);
      sms.setOriginatingAddress(address);
    } catch (AdminException e) {
      throw new IOException(e.getMessage());
    }
    try {
      int len = Message.readUInt8(is);
      String type = Message.readString(is, 1);
      String plan = Message.readString(is, 1);
      String address = Message.readString(is, len);
      sms.setDestinationAddress(address);
    } catch (AdminException e) {
      throw new IOException(e.getMessage());
    }
    try {
      int len = Message.readUInt8(is);
      String type = Message.readString(is, 1);
      String plan = Message.readString(is, 1);
      String address = Message.readString(is, len);
      sms.setDealiasedDestinationAddress(address);
    } catch (AdminException e) {
      throw new IOException(e.getMessage());
    }
    sms.setMessageReference(Message.readUInt16(is));
    sms.setServiceType(Message.readString8(is));
    sms.setDeliveryReport((short) Message.readUInt8(is));
    sms.setBillingRecord((short) Message.readUInt8(is));
    String odMsc = Message.readString8(is);
    String odImsi = Message.readString8(is);
    sms.setOriginatingDescriptor(new SmsDescriptor(odImsi, odMsc, (int) Message.readUInt32(is)));
    String ddMsc = Message.readString8(is);
    String ddImsi = Message.readString8(is);
    sms.setDestinationDescriptor(new SmsDescriptor(ddImsi, ddMsc, (int) Message.readUInt32(is)));
    sms.setRouteId(Message.readString8(is));
    sms.setServiceId((int) Message.readUInt32(is));
    sms.setPriority((int) Message.readUInt32(is));
    sms.setSrcSmeId(Message.readString8(is));
    sms.setDstSmeId(Message.readString8(is));
    int concatMsgRef = Message.readUInt16(is);
    String concatSeqNum = Message.readString(is, 1);
    int bodyLen = (int) Message.readUInt32(is);
    if (bodyLen > 0) {
      byte body[] = new byte[bodyLen];
      int pos = 0;
      int cnt = 0;
      while (pos < bodyLen) {
        cnt = is.read(body, pos, bodyLen - pos);
        if (cnt == -1) {
          BodyRecived = true;
          throw new EOFException();
        }
        ;
        pos += cnt;
      }
      SmsSource.parseBody(new ByteArrayInputStream(body, 0, bodyLen), sms);
    }
  }
   */
  public boolean isBodyRecived()
  {
    return BodyRecived;
  }

  public boolean isAllReaded()
  {
    return allReaded;
  }

  public  void setAllReaded(boolean allReaded)
  {
    this.allReaded = allReaded;
  }
}
