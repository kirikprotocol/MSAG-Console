package ru.novosoft.smsc.admin.smsview.archive;

import ru.novosoft.smsc.admin.smsview.SmsRow;
import ru.novosoft.smsc.admin.smsview.SmsDescriptor;
import ru.novosoft.smsc.admin.smsview.SmsSource;
import ru.novosoft.smsc.admin.AdminException;

import java.io.*;
import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.01.2004
 * Time: 21:10:22
 * To change this template use Options | File Templates.
 */
public class RsSmsMessage extends Message
{
  private SmsRow sms = null;

  public RsSmsMessage() {
    super(Message.SMSC_BYTE_RSSMS_TYPE);
  }
  public RsSmsMessage(SmsRow sms) {
    super(Message.SMSC_BYTE_RSSMS_TYPE);
    this.sms = sms;
  }
  public SmsRow getSms() {
    return sms;
  }

  public void receive(InputStream bis) throws IOException
  {
    sms = new SmsRow();

    int msgSize1 = (int)Message.readUInt32(bis);
    byte message[] = new byte[msgSize1];
    int read = 0;
    while (read < msgSize1) {
      int result = bis.read(message, read, msgSize1-read);
      if (result < 0)
        throw new IOException("Protocol error. Failed to read "+msgSize1+" bytes");
      read += result;
    }
    InputStream is = new ByteArrayInputStream(message);
    int msgSize2 = (int)Message.readUInt32(bis);
    if (msgSize1 != msgSize2)
      throw new IOException("Protocol error sz1="+msgSize1+" sz2="+msgSize2);

    sms.setId(Message.readInt64(is));
    sms.setStatus(Message.readUInt8(is));
    sms.setSubmitTime (Message.convertDateFromString(Message.readString8(is)));
    sms.setValidTime  (Message.convertDateFromString(Message.readString8(is)));
    sms.setLastTryTime(Message.convertDateFromString(Message.readString8(is)));
    sms.setNextTryTime(Message.convertDateFromString(Message.readString8(is)));
    sms.setAttempts((int)Message.readUInt32(is));
    sms.setLastResult((int)Message.readUInt32(is));
    try { sms.setOriginatingAddress(Message.readString8(is)); }
    catch (AdminException e) { throw new IOException(e.getMessage()); }
    try { sms.setDestinationAddress(Message.readString8(is)); }
    catch (AdminException e) { throw new IOException(e.getMessage()); }
    try { sms.setDealiasedDestinationAddress(Message.readString8(is)); }
    catch (AdminException e) { throw new IOException(e.getMessage()); }
    sms.setMessageReference(Message.readUInt16(is));
    sms.setServiceType(Message.readString8(is));
    sms.setDeliveryReport((short)Message.readUInt8(is));
    sms.setBillingRecord((short)Message.readUInt8(is));
    String odMsc = Message.readString8(is); String odImsi = Message.readString8(is);
    sms.setOriginatingDescriptor(new SmsDescriptor(odImsi, odMsc, (int)Message.readUInt32(is)));
    String ddMsc = Message.readString8(is); String ddImsi = Message.readString8(is);
    sms.setDestinationDescriptor(new SmsDescriptor(ddImsi, ddMsc, (int)Message.readUInt32(is)));
    sms.setRouteId(Message.readString8(is));
    sms.setServiceId((int)Message.readUInt32(is));
    sms.setPriority((int)Message.readUInt32(is));
    sms.setSrcSmeId(Message.readString8(is));
    sms.setDstSmeId(Message.readString8(is));
    int bodyLen = (int)Message.readUInt32(is);
    if (bodyLen > 0) {
      byte body[] = new byte[bodyLen];
      int pos = 0; int cnt = 0;
      while (pos < bodyLen) {
        cnt = is.read(body, pos, bodyLen-pos);
        if( cnt == -1 ) throw new EOFException();
        pos += cnt;
      }
      SmsSource.parseBody(new ByteArrayInputStream(body, 0, bodyLen), sms);
    }
  }
}
