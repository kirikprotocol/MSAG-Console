package ru.novosoft.smsc.admin.archive_daemon.messages;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.SmsDescriptor;
import ru.novosoft.smsc.admin.archive_daemon.SmsRow;
import ru.novosoft.smsc.util.IOUtils;

import java.io.ByteArrayInputStream;
import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;

/**
 * Служебный класс для работы с ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */
public class RsSmsMessage extends Message {
  private SmsRow sms = null;

  public RsSmsMessage() {
    super(SMSC_BYTE_RSSMS_TYPE);
  }

  public RsSmsMessage(SmsRow sms) {
    super(SMSC_BYTE_RSSMS_TYPE);
    this.sms = sms;
  }

  public SmsRow getSms() {
    return sms;
  }

  public void receive(InputStream bis) throws IOException {
    sms = new SmsRow();

    int msgSize1 = (int) IOUtils.readUInt32(bis);
    byte message[] = new byte[msgSize1];
    int read = 0;
    while (read < msgSize1) {
      int result = bis.read(message, read, msgSize1 - read);
      if (result < 0)
        throw new IOException("Protocol error. Failed to read " + msgSize1 + " bytes");
      read += result;
    }
    InputStream is = new ByteArrayInputStream(message);
    int msgSize2 = (int) IOUtils.readUInt32(bis);
    if (msgSize1 != msgSize2)
      throw new IOException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);

    sms.setId(IOUtils.readInt64(is));
    sms.setStatus(IOUtils.readUInt8(is));
    sms.setSubmitTime(convertDateFromString(IOUtils.readString8(is)));
    sms.setValidTime(convertDateFromString(IOUtils.readString8(is)));
    sms.setLastTryTime(convertDateFromString(IOUtils.readString8(is)));
    sms.setNextTryTime(convertDateFromString(IOUtils.readString8(is)));
    sms.setAttempts((int) IOUtils.readUInt32(is));
    sms.setLastResult((int) IOUtils.readUInt32(is));
    try {
      sms.setOriginatingAddress(IOUtils.readString8(is));
    } catch (AdminException e) {
      throw new IOException(e.getMessage());
    }
    try {
      sms.setDestinationAddress(IOUtils.readString8(is));
    } catch (AdminException e) {
      throw new IOException(e.getMessage());
    }
    try {
      sms.setDealiasedDestinationAddress(IOUtils.readString8(is));
    } catch (AdminException e) {
      throw new IOException(e.getMessage());
    }
    sms.setMessageReference(IOUtils.readUInt16(is));
    sms.setServiceType(IOUtils.readString8(is));
    sms.setDeliveryReport((short) IOUtils.readUInt8(is));
    sms.setBillingRecord((short) IOUtils.readUInt8(is));
    String odMsc = IOUtils.readString8(is);
    String odImsi = IOUtils.readString8(is);
    sms.setOriginatingDescriptor(new SmsDescriptor(odImsi, odMsc, (int) IOUtils.readUInt32(is)));
    String ddMsc = IOUtils.readString8(is);
    String ddImsi = IOUtils.readString8(is);
    sms.setDestinationDescriptor(new SmsDescriptor(ddImsi, ddMsc, (int) IOUtils.readUInt32(is)));
    sms.setRouteId(IOUtils.readString8(is));
    sms.setServiceId((int) IOUtils.readUInt32(is));
    sms.setPriority((int) IOUtils.readUInt32(is));
    sms.setSrcSmeId(IOUtils.readString8(is));
    sms.setDstSmeId(IOUtils.readString8(is));
    int bodyLen = (int) IOUtils.readUInt32(is);
    if (bodyLen > 0) {
      byte body[] = new byte[bodyLen];
      int pos = 0;
      int cnt = 0;
      while (pos < bodyLen) {
        cnt = is.read(body, pos, bodyLen - pos);
        if (cnt == -1) throw new EOFException();
        pos += cnt;
      }
//      SmsSource.parseBody(new ByteArrayInputStream(body, 0, bodyLen), sms);
      sms.setBody(body);
    }
  }
}
