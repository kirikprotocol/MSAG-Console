package ru.novosoft.smsc.admin.smsview.operative;

import ru.novosoft.smsc.admin.smsview.SmsRow;
import ru.novosoft.smsc.admin.smsview.SmsDescriptor;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.admin.route.Mask;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.EOFException;
import java.io.InputStream;
import java.util.Date;

/**
 * User: artem
 * Date: 25.05.2009
 */

/**
 * SmsRow with lazy fields deserialization
 */
public class LazySmsRow extends SmsRow {

  private final byte[] message;
  private InputStream is;
  private final boolean haveArc;

  public LazySmsRow(byte[] message, boolean haveArc, long id, int status) {
    this.message = message;
    this.haveArc = haveArc;
    setId(id);
    setStatus(status);
  }

  private void readSubmitTime() {
    if (submitTime == null) {
      if (is == null)
        is = new ByteArrayInputStream(message);
      try {
        setSubmitTime(new Date(Message.readUInt32(is) * 1000));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readValidTime() {
    if (validTime == null) {
      readSubmitTime();
      try {
        setValidTime(new Date(Message.readUInt32(is) * 1000));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readLastTryTime() {
    if (lastTryTime == null) {
      readValidTime();
      try {
        setLastTryTime(new Date(Message.readUInt32(is) * 1000));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readNextTryTime() {
    if (nextTryTime == null) {
      readLastTryTime();
      try {
        setNextTryTime(new Date(Message.readUInt32(is) * 1000));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readAttempts() {
    if (attempts == null) {
      readNextTryTime();
      try {
        setAttempts((int) Message.readUInt32(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readLastResult() {
    if (lastResult == null) {
      readAttempts();
      try {
        setLastResult((int) Message.readUInt32(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readOriginatingAddress() {
    if (originatingAddress.equals("originatingAddress")) {
      readLastResult();
      try {
        int strLen = Message.readUInt8(is);
        Mask origMask = new Mask((byte) Message.readUInt8(is), (byte) Message.readUInt8(is), Message.readString(is, strLen));
        setOriginatingAddress(origMask);
      } catch (IOException e) {
        e.printStackTrace();
      }

    }
  }

  private void readDestinationAddress() {
    if (destinationAddress.equals("destinationAddress")) {
      readOriginatingAddress();
      try {
        int strLen = Message.readUInt8(is);
        Mask destMask = new Mask((byte) Message.readUInt8(is), (byte) Message.readUInt8(is), Message.readString(is, strLen));
        setDestinationAddress(destMask);
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readDealiasedDestinationAddress() {
    if (dealiasedDestinationAddress.equals("dealiasedDestinationAddress")) {
      readDestinationAddress();
      try {
        int strLen = Message.readUInt8(is);
        Mask dDestMask = new Mask((byte) Message.readUInt8(is), (byte) Message.readUInt8(is), Message.readString(is, strLen));
        setDealiasedDestinationAddress(dDestMask);
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readMessageReference() {
    if (messageReference == null) {
      readDealiasedDestinationAddress();
      try {
        setMessageReference(Message.readUInt16(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readServiceType() {
    if (serviceType == null) {
      try {
        readMessageReference();
        setServiceType(Message.readString8(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readDeliveryReport() {
    if (deliveryReport == null) {
      readServiceType();
      try {
        setDeliveryReport((short) Message.readUInt8(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readBillingRecord() {
    if (billingRecord == null) {
      readDeliveryReport();
      try {
        setBillingRecord((short) Message.readUInt8(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readOriginatingDescriptor() {
    if (originatingDescriptor == null) {
      readBillingRecord();
      try {
        String odMsc = Message.readString8(is);
        String odImsi = Message.readString8(is);
        int odSme = (int) Message.readUInt32(is);
        setOriginatingDescriptor(new SmsDescriptor(odImsi, odMsc, odSme));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readDestinationDescriptor() {
    if (destinationDescriptor == null) {
      readOriginatingDescriptor();
      try {
        String ddMsc = Message.readString8(is);
        String ddImsi = Message.readString8(is);
        int ddSme = (int) Message.readUInt32(is);
        setDestinationDescriptor(new SmsDescriptor(ddImsi, ddMsc, ddSme));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readRouteId() {
    if (routeId == null) {
      readDestinationDescriptor();
      try {
        setRouteId(Message.readString8(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readServiceId() {
    if (serviceId == null) {
      readRouteId();
      try {
        setServiceId((int) Message.readUInt32(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readPriority() {
    if (priority == null) {
      readServiceId();
      try {
        setPriority((int) Message.readUInt32(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readSrcSmeId() {
    if (srcSmeId == null) {
      readPriority();
      try {
        setSrcSmeId(Message.readString8(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readDstSmeId() {
    if (dstSmeId == null) {
      readSrcSmeId();
      try {
        setDstSmeId(Message.readString8(is));
        if (is.skip(3) != 3) // Skip concat msg ref and seq num
          throw new EOFException();
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readArc() {
    readDstSmeId();
    try {
      if (haveArc && arc == -1)
        setArc((byte) Message.readUInt8(is));      
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  private void readBody() {
    if (body == null) {
      readArc();
      try {
        int bodyLen = (int) Message.readUInt32(is);
        byte body[] = new byte[bodyLen];
        int pos = 0;
        while (pos < bodyLen) {
          int cnt = is.read(body, pos, bodyLen - pos);
          if (cnt == -1)
            throw new EOFException();
          pos += cnt;
        }
        setBody(body);
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  public Date getSubmitTime() {
    readSubmitTime();
    return super.getSubmitTime();
  }

  public Date getValidTime() {
    readValidTime();
    return super.getValidTime();
  }

  public Date getLastTryTime() {
    readLastTryTime();
    return super.getLastTryTime();
  }

  public Date getNextTryTime() {
    readNextTryTime();
    return super.getNextTryTime();
  }

  public int getAttempts() {
    readAttempts();
    return super.getAttempts();
  }

  public int getLastResult() {
    readLastResult();
    return super.getLastResult();
  }

  public String getOriginatingAddress() {
    readOriginatingAddress();
    return super.getOriginatingAddress();
  }

  public Mask getOriginatingAddressMask() {
    readOriginatingAddress();
    return super.getOriginatingAddressMask();
  }

  public String getDestinationAddress() {
    readDestinationAddress();
    return super.getDestinationAddress();
  }

  public Mask getDestinationAddressMask() {
    readDestinationAddress();
    return super.getDestinationAddressMask();
  }

  public String getDealiasedDestinationAddress() {
    readDealiasedDestinationAddress();
    return super.getDealiasedDestinationAddress();
  }

  public Mask getDealiasedDestinationAddressMask() {
    readDealiasedDestinationAddress();
    return super.getDealiasedDestinationAddressMask();
  }

  public int getMessageReference() {
    readMessageReference();
    return super.getMessageReference();
  }

  public String getServiceType() {
    readServiceType();
    return super.getServiceType();
  }

  public short getDeliveryReport() {
    readDeliveryReport();
    return super.getDeliveryReport();
  }

  public short getBillingRecord() {
    readBillingRecord();
    return super.getBillingRecord();
  }

  public SmsDescriptor getOriginatingDescriptor() {
    readOriginatingDescriptor();
    return super.getOriginatingDescriptor();
  }

  public SmsDescriptor getDestinationDescriptor() {
    readDestinationDescriptor();
    return super.getDestinationDescriptor();
  }

  public String getRouteId() {
    readRouteId();
    return super.getRouteId();
  }

  public int getServiceId() {
    readServiceId();
    return super.getServiceId();
  }

  public int getPriority() {
    readPriority();
    return super.getPriority();
  }

  public String getSrcSmeId() {
    readSrcSmeId();
    return super.getSrcSmeId();
  }

  public String getDstSmeId() {
    readDstSmeId();
    return super.getDstSmeId();
  }

  public byte getArc() {
    readArc();
    return super.getArc();
  }

  public byte[] getBody() {
    readBody();
    return super.getBody();
  }

}
