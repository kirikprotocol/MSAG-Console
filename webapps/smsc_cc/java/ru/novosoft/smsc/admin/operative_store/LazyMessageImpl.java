package ru.novosoft.smsc.admin.operative_store;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.GSM7CharSet;
import ru.novosoft.smsc.util.IOUtils;

import java.io.*;
import java.util.Date;

/**
 * @author Artem Snopkov
 */
class LazyMessageImpl implements Message {

  private final static byte INT_TAG_TYPE = 0;
  private final static byte STR_TAG_TYPE = 1;
  private final static byte BIN_TAG_TYPE = 2;

  private long id;
  private Date submitTime;
  private Date validTime;
  private Integer attempts;
  private Integer lastResult;
  private Date lastTryTime;
  private Date nextTryTime;
  private Address originatingAddress;
  private Address destinationAddress;
  private Address dealiasedDestinationAddress;
  private Integer messageReference;
  private String serviceType;
  private Integer deliveryReport;
  private Integer billingRecord;
  private RoutingInfo originatingDescriptor;
  private RoutingInfo destinationDescriptor;
  private String routeId;
  private Integer serviceId;
  private Integer priority;
  private String srcSmeId;
  private String dstSmeId;

  private String originalText;
  private String text;
  private boolean textEncoded;
  private byte arc = -1;
  private BodyParameters parameters;
  private byte body[];

  private byte[] message;
  private InputStream is;
  private boolean haveArc;

  LazyMessageImpl(byte[] message, boolean haveArc, long id) {
    reset(message, haveArc, id);
  }

  void reset(byte[] message, boolean haveArc, long id) {
    this.id = id;
    this.message = message;
    this.haveArc = haveArc;
    submitTime = null;
    validTime = null;
    attempts = null;
    lastResult = null;
    lastTryTime = null;
    nextTryTime = null;
    originatingAddress = null;
    destinationAddress = null;
    dealiasedDestinationAddress = null;
    messageReference = null;
    serviceType = null;
    deliveryReport = null;
    billingRecord = null;
    originatingDescriptor = null;
    destinationDescriptor = null;
    routeId = null;
    serviceId = null;
    priority = null;
    srcSmeId = null;
    dstSmeId = null;
    originalText = null;
    text = null;
    textEncoded = false;
    arc = (byte) -1;
    parameters = null;
    body = null;
    is = null;
  }

  private void readSubmitTime() {
    if (submitTime == null) {
      if (is == null)
        is = new ByteArrayInputStream(message);
      try {
        submitTime = (new Date(IOUtils.readUInt32(is) * 1000));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readValidTime() {
    if (validTime == null) {
      readSubmitTime();
      try {
        validTime = (new Date(IOUtils.readUInt32(is) * 1000));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readLastTryTime() {
    if (lastTryTime == null) {
      readValidTime();
      try {
        lastTryTime = (new Date(IOUtils.readUInt32(is) * 1000));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readNextTryTime() {
    if (nextTryTime == null) {
      readLastTryTime();
      try {
        nextTryTime = (new Date(IOUtils.readUInt32(is) * 1000));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readAttempts() {
    if (attempts == null) {
      readNextTryTime();
      try {
        attempts = ((int) IOUtils.readUInt32(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readLastResult() {
    if (lastResult == null) {
      readAttempts();
      try {
        lastResult = ((int) IOUtils.readUInt32(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readOriginatingAddress() {
    if (originatingAddress == null) {
      readLastResult();
      try {
        int strLen = IOUtils.readUInt8(is);
        originatingAddress = new Address((byte) IOUtils.readUInt8(is), (byte) IOUtils.readUInt8(is), IOUtils.readString(is, strLen));
      } catch (IOException e) {
        e.printStackTrace();
      }

    }
  }

  private void readDestinationAddress() {
    if (destinationAddress == null) {
      readOriginatingAddress();
      try {
        int strLen = IOUtils.readUInt8(is);
        destinationAddress = new Address((byte) IOUtils.readUInt8(is), (byte) IOUtils.readUInt8(is), IOUtils.readString(is, strLen));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readDealiasedDestinationAddress() {
    if (dealiasedDestinationAddress == null) {
      readDestinationAddress();
      try {
        int strLen = IOUtils.readUInt8(is);
        dealiasedDestinationAddress = new Address((byte) IOUtils.readUInt8(is), (byte) IOUtils.readUInt8(is), IOUtils.readString(is, strLen));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readMessageReference() {
    if (messageReference == null) {
      readDealiasedDestinationAddress();
      try {
        messageReference = (IOUtils.readUInt16(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readServiceType() {
    if (serviceType == null) {
      try {
        readMessageReference();
        serviceType = (IOUtils.readString8(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readDeliveryReport() {
    if (deliveryReport == null) {
      readServiceType();
      try {
        deliveryReport = (IOUtils.readUInt8(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readBillingRecord() {
    if (billingRecord == null) {
      readDeliveryReport();
      try {
        billingRecord = (IOUtils.readUInt8(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readOriginatingDescriptor() {
    if (originatingDescriptor == null) {
      readBillingRecord();
      try {
        String odMsc = IOUtils.readString8(is);
        String odImsi = IOUtils.readString8(is);
        int odSme = (int) IOUtils.readUInt32(is);
        originatingDescriptor = (new RoutingInfo(odImsi, odMsc, odSme));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readDestinationDescriptor() {
    if (destinationDescriptor == null) {
      readOriginatingDescriptor();
      try {
        String ddMsc = IOUtils.readString8(is);
        String ddImsi = IOUtils.readString8(is);
        int ddSme = (int) IOUtils.readUInt32(is);
        destinationDescriptor = (new RoutingInfo(ddImsi, ddMsc, ddSme));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readRouteId() {
    if (routeId == null) {
      readDestinationDescriptor();
      try {
        routeId = (IOUtils.readString8(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readServiceId() {
    if (serviceId == null) {
      readRouteId();
      try {
        serviceId = ((int) IOUtils.readUInt32(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readPriority() {
    if (priority == null) {
      readServiceId();
      try {
        priority = ((int) IOUtils.readUInt32(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readSrcSmeId() {
    if (srcSmeId == null) {
      readPriority();
      try {
        srcSmeId = (IOUtils.readString8(is));
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void readDstSmeId() {
    if (dstSmeId == null) {
      readSrcSmeId();
      try {
        dstSmeId = (IOUtils.readString8(is));
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
        arc = ((byte) IOUtils.readUInt8(is));
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  private void readBody() {
    if (body == null) {
      readArc();
      try {
        int bodyLen = (int) IOUtils.readUInt32(is);
        body = new byte[bodyLen];
        int pos = 0;
        while (pos < bodyLen) {
          int cnt = is.read(body, pos, bodyLen - pos);
          if (cnt == -1)
            throw new EOFException();
          pos += cnt;
        }
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  public long getId() throws AdminException {
    return id;
  }

  public Date getSubmitTime() throws AdminException {
    readSubmitTime();
    return submitTime;
  }

  public Date getValidTime() throws AdminException {
    readValidTime();
    return validTime;
  }

  public Date getLastTryTime() throws AdminException {
    readLastTryTime();
    return lastTryTime;
  }

  public Date getNextTryTime() throws AdminException {
    readNextTryTime();
    return nextTryTime;
  }

  public Integer getAttempts() throws AdminException {
    readAttempts();
    return attempts;
  }

  public Integer getLastResult() throws AdminException {
    readLastResult();
    return lastResult;
  }

  public Address getOriginatingAddress() throws AdminException {
    readOriginatingAddress();
    return originatingAddress;
  }

  public Address getDestinationAddress() throws AdminException {
    readDestinationAddress();
    return destinationAddress;
  }

  public Address getDealiasedDestinationAddress() throws AdminException {
    readDealiasedDestinationAddress();
    return dealiasedDestinationAddress;
  }

  public Integer getMessageReference() throws AdminException {
    readMessageReference();
    return messageReference;
  }

  public String getServiceType() throws AdminException {
    readServiceType();
    return serviceType;
  }

  public Integer getDeliveryReport() throws AdminException {
    readDeliveryReport();
    return deliveryReport;
  }

  public Integer getBillingRecord() throws AdminException {
    readBillingRecord();
    return billingRecord;
  }

  public RoutingInfo getOriginatingDescriptor() throws AdminException {
    readOriginatingDescriptor();
    return originatingDescriptor;
  }

  public RoutingInfo getDestinationDescriptor() throws AdminException {
    readDestinationDescriptor();
    return destinationDescriptor;
  }

  public String getRouteId() throws AdminException {
    readRouteId();
    return routeId;
  }

  public Integer getServiceId() throws AdminException {
    readServiceId();
    return serviceId;
  }

  public Integer getPriority() throws AdminException {
    readPriority();
    return priority;
  }

  public String getSrcSmeId() throws AdminException {
    readSrcSmeId();
    return srcSmeId;
  }

  public String getDstSmeId() throws AdminException {
    readDstSmeId();
    return dstSmeId;
  }

  public byte getArc() throws AdminException {
    readArc();
    return arc;
  }

  public byte[] getBody() throws AdminException {
    readBody();
    return body;
  }

  public String getOriginalText() throws AdminException {
    parseBody();
    return originalText;
  }

  public String getText() throws AdminException {
    parseBody();
    return text;
  }

  public boolean isTextEncoded() throws AdminException {
    parseBody();
    return textEncoded;
  }

  public BodyParameters getBodyParameters() throws AdminException {
    parseBody();
    return parameters;
  }

  private void parseBody() throws AdminException {
    readBody();
    try {
      parameters = parseBodyParameters(body);

      byte[] text = parameters.getShortMessage();
      if (text == null)
        text = parameters.getMessagePayload();

      if (text == null || text.length == 0)
        return;


      DataCoding textEncoding = parameters.getDataCoding();

      StringBuilder textBuffer = new StringBuilder(text.length);
      StringBuilder originalTextBuffer = new StringBuilder(text.length);
      if (parameters.getMergeConcat() != null) {
        byte[] concatInfo = parameters.getConcatInfo();
        if (concatInfo != null) {
//          int partsCount = concatInfo[0];
          int partsCount = parameters.getOriginalPartsNum();

          StringBuilder sb = new StringBuilder();
          for (byte aConcatInfo : concatInfo)
            sb.append(Integer.toHexString(((int) aConcatInfo) & 0xFF)).append(' ');

          DataCoding[] partsEncoding = parameters.getDcList();

          if (partsEncoding != null && partsCount != partsEncoding.length) {
            IOException exc = new IOException("Invalid partsEncoding count dc:" + partsEncoding.length + " ci:" + partsCount);
            this.text = exc.getMessage();
            throw exc;
          }

          if (partsCount == concatInfo[0]) {
            for (int i = 0; i < partsCount; i++) {
              int offset = ((((int) concatInfo[i * 2 + 1]) & 0xFF) << 8) | (((int) concatInfo[i * 2 + 2]) & 0xFF);
              int len = text.length - offset;
              if (i < partsCount - 1) {
                int offset_next = ((((int) concatInfo[(i + 1) * 2 + 1]) & 0xFF) << 8) | (((int) concatInfo[(i + 1) * 2 + 2]) & 0xFF);
                len = offset_next - offset;
              }
              convertMessage(textBuffer, originalTextBuffer, text, offset, len, true, (partsEncoding != null) ? partsEncoding[i] : textEncoding);
            }
          }
        } else
          convertMessage(textBuffer, originalTextBuffer, text, 0, text.length, true, textEncoding);
      } else {
        Integer esmClass = parameters.getEsmClass();
        if (esmClass == null)
          esmClass = 0;
        convertMessage(textBuffer, originalTextBuffer, text, 0, text.length, ((esmClass & 0x40) == 0x40), textEncoding);
      }

      this.textEncoded = textEncoding == DataCoding.UCS2;
      this.text = textBuffer.toString();
      this.originalText = originalTextBuffer.toString();

    } catch (IOException exc) {
      throw new OperativeStoreException("message_read_error", exc);
    }
  }

  private static Object parseBodyParamValue(DataInputStream stream, int tag, int type, int len) throws IOException {
    switch (tag) {
      case BodyParameters.SMSC_BACKUP_SME_TAG:
      case BodyParameters.SMPP_SHORT_MESSAGE_TAG:
      case BodyParameters.SMPP_MESSAGE_PAYLOAD_TAG:
        byte msgText[] = new byte[len];
        stream.read(msgText, 0, len);
        return msgText;
      case BodyParameters.SMSC_MERGE_CONCAT_TAG:
        return stream.readInt();
      case BodyParameters.SMSC_DC_LIST_TAG:
        byte[] partsEncodingInt = new byte[len];
        stream.read(partsEncodingInt, 0, len);
        DataCoding[] partsEncoding = new DataCoding[len];
        for (int i = 0; i < partsEncodingInt.length; i++)
          partsEncoding[i] = parseDataCoding(partsEncodingInt[i]);
        return partsEncoding;
      case BodyParameters.SMPP_CONCAT_INFO_TAG:
        byte[] concatInfo = new byte[len];
        stream.read(concatInfo, 0, len);
        return concatInfo;
      case BodyParameters.SMPP_DATA_CODING_TAG:
        return parseDataCoding(stream.readInt());
      case BodyParameters.SMPP_ESM_CLASS_TAG:
        return stream.readInt();
      case BodyParameters.SMSC_ORIG_PARTS_NUM_TAG:
        return stream.readInt();
      default:
        switch (type) {
          case INT_TAG_TYPE:
            return stream.readInt();
          case STR_TAG_TYPE:
            byte val[] = new byte[len];
            stream.read(val, 0, len);
            return new String(val);
          case BIN_TAG_TYPE:
            byte bval[] = new byte[len];
            stream.read(bval, 0, len);
            return bval;
          default:
            byte dval[] = new byte[len];
            stream.read(dval, 0, len);
            return "invalid tag type: " + getHexString(dval);
        }
    }
  }

  private static BodyParameters parseBodyParameters(byte[] body) throws IOException {
    BodyParameters p = new BodyParameters();
    DataInputStream stream = null;
    try {
      stream = new DataInputStream(new ByteArrayInputStream(body));
      while (stream.available() > 0) {
        int tag = stream.readShort();
        byte type = (byte) ((tag & (short) 0xff00) >> 8);
        type &= (short) 0x00ff;
        tag &= (short) 0x00ff;

        int len = (type == INT_TAG_TYPE) ? 4 : stream.readInt();

        Object value = parseBodyParamValue(stream, tag, type, len);
        p.addParameter(tag, value);
      } // while (stream.available() > 0)
    } finally {
      if (stream != null)
        stream.close();
    }
    return p;
  }

  private static void convertMessage(StringBuilder sb, StringBuilder orgsb, byte text[], int start, int len, boolean udh, DataCoding encoding) throws UnsupportedEncodingException {
    if (udh) {
      int headerLen = ((int) text[start]) & 0xff;  // convert negative byte to int
      if (headerLen > len - 1) {
        sb.append("<< UDH len greater then message len " + headerLen + "/" + (len - 1) + ">>");
        orgsb.append("<< UDH len greater then message len " + headerLen + "/" + (len - 1) + ">>");
      } else {
        sb.append("<< UDH " + headerLen + " bytes >> ");
        orgsb.append("<< UDH " + headerLen + " bytes >> ");
        int textLen = len - headerLen - 1;
        if (textLen > 0) {
          byte msgText[] = new byte[textLen];
          System.arraycopy(text, start + headerLen + 1, msgText, 0, textLen);
          sb.append(decodeMessage(msgText, textLen, encoding));
          orgsb.append(decodeOriginalMessage(msgText, textLen, encoding));
        }
      }
    } else {
      byte msgText[] = new byte[len];
      System.arraycopy(text, start, msgText, 0, len);
      sb.append(decodeMessage(msgText, len, encoding));
      orgsb.append(decodeOriginalMessage(msgText, len, encoding));
    }
  }

  private static DataCoding parseDataCoding(int dataCoding) {
    switch (dataCoding) {
      case 3:
        return DataCoding.LATIN1;
      case 4:
        return DataCoding.BINARY;
      case 8:
        return DataCoding.UCS2;
      default:
        return DataCoding.DEFAULT;
    }
  }

  private static String decodeMessage(byte text[], int len, DataCoding encoding) throws UnsupportedEncodingException {
    switch (encoding) {
      case LATIN1:
        return new String(text, 0, len, "ISO-8859-1");
      case DEFAULT:
        return GSM7CharSet.decode(text);
      case UCS2:
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < len / 2; ++i) {
          int x = ((((int) text[i * 2]) & 0x0ff) << 8) | ((((int) text[i * 2 + 1]) & 0x0ff));
          sb.append("&#").append(x).append(';');
        }
        return sb.toString();
      case BINARY:
        return "<< Binary data >>";
      default:
        return "<< Unsupported encoding (" + encoding + ") ! >>";
    }
  }

  private static String decodeOriginalMessage(byte text[], int len, DataCoding encoding) throws UnsupportedEncodingException {
    switch (encoding) {
      case LATIN1:
        return new String(text, 0, len, "ISO-8859-1");
      case DEFAULT:
        return GSM7CharSet.decode(text);
      case UCS2:
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < len / 2; ++i) {
          int x = ((((int) text[i * 2]) & 0x0ff) << 8) | ((((int) text[i * 2 + 1]) & 0x0ff));
          sb.append((char) x);
        }
        return sb.toString();
      case BINARY:
        return "<< Binary data >>";
      default:
        return "<< Unsupported encoding (" + encoding + ") ! >>";
    }
  }

  private static String getHexString(byte val[]) {
    StringBuilder sb = new StringBuilder();
    for (byte aVal : val) {
      sb.append(' ');
      int k = (((int) aVal) >> 4) & 0xf;
      sb.append((char) (k < 10 ? k + 48 : k + 55));
      k = ((int) aVal) & 0xf;
      sb.append((char) (k < 10 ? k + 48 : k + 55));
    }
    return sb.toString().toUpperCase();
  }


}
