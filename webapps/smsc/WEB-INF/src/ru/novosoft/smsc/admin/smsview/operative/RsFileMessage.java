package ru.novosoft.smsc.admin.smsview.operative;

import ru.novosoft.smsc.admin.smsview.SmsRow;
import ru.novosoft.smsc.admin.smsview.SmsDescriptor;
import ru.novosoft.smsc.admin.smsview.SmsSource;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.admin.AdminException;

import java.io.InputStream;
import java.io.IOException;
import java.io.ByteArrayInputStream;
import java.io.EOFException;
import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 11.02.2005
 * Time: 18:23:01
 * To change this template use File | Settings | File Templates.
 */
public class RsFileMessage  extends Message
{
  private SmsRow sms = null;
  private boolean BodyRecived=false;
  private boolean allReaded=false;
  public RsFileMessage()
  {
    super(Message.SMSC_BYTE_RSSMS_TYPE);

  }

  public RsFileMessage(SmsRow sms)
  {
    super(Message.SMSC_BYTE_RSSMS_TYPE);
    this.sms = sms;
  }

  public SmsRow getSms()
  {
    return sms;
  }

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
    long msgId=Message.readInt64(is);
    sms.setId(msgId);
    int seq=(int)Message.readUInt32(is);
    String finall=Message.readString(is,1);
    sms.setStatus(Message.readUInt8(is));
    sms.setSubmitTime(new Date(Message.readUInt32(is)*1000));
    sms.setValidTime(new Date(Message.readUInt32(is)*1000));
    sms.setLastTryTime(new Date(Message.readUInt32(is)*1000));
    sms.setNextTryTime(new Date(Message.readUInt32(is)*1000));
    sms.setAttempts((int) Message.readUInt32(is));
    sms.setLastResult((int) Message.readUInt32(is));
    try {
      int len=Message.readUInt8(is);
      String type=Message.readString(is,1);
      String plan=Message.readString(is,1);
      String address=Message.readString(is,len);
      sms.setOriginatingAddress(address);
    } catch (AdminException e) {
      throw new IOException(e.getMessage());
    }
    try {
      int len=Message.readUInt8(is);
      String type=Message.readString(is,1);
      String plan=Message.readString(is,1);
      String address=Message.readString(is,len);
      sms.setDestinationAddress(address);
    } catch (AdminException e) {
      throw new IOException(e.getMessage());
    }
    try {
      int len=Message.readUInt8(is);
      String type=Message.readString(is,1);
      String plan=Message.readString(is,1);
      String address=Message.readString(is,len);
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
    int concatMsgRef=Message.readUInt16(is);
    String concatSeqNum=Message.readString(is,1);
    int bodyLen = (int) Message.readUInt32(is);
    if (bodyLen > 0) {
      byte body[] = new byte[bodyLen];
      int pos = 0;
      int cnt = 0;
      while (pos < bodyLen) {
        cnt = is.read(body, pos, bodyLen - pos);
        if (cnt == -1) { BodyRecived=true;throw new EOFException();
        };
        pos += cnt;
      }
      SmsSource.parseBody(new ByteArrayInputStream(body, 0, bodyLen), sms);
    }
  }

  public boolean isBodyRecived()
  {
    return BodyRecived;
  }

  public boolean isAllReaded()
  {
    return allReaded;
  }

  public void setAllReaded(boolean allReaded)
  {
    this.allReaded = allReaded;
  }
}
