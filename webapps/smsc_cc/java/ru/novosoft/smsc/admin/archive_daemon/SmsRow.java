package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;

import java.io.ByteArrayInputStream;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

@SuppressWarnings({"UnusedDeclaration"})
/**
 * Строка-запись в статистике ArchiveDemon
 * @author Aleksandr Khalitov
 */
public class SmsRow {

  protected long id;
  protected Date submitTime = null;
  protected Date validTime;
  protected Integer attempts;
  protected Integer lastResult;
  protected Date lastTryTime;
  protected Date nextTryTime;
  protected String originatingAddress = "originatingAddress";
  protected String destinationAddress = "destinationAddress";
  protected String dealiasedDestinationAddress = "dealiasedDestinationAddress";
  protected Integer messageReference;
  protected String serviceType;
  protected boolean needArchivate;
  protected Short deliveryReport;
  protected Short billingRecord;
  protected SmsDescriptor originatingDescriptor;
  protected SmsDescriptor destinationDescriptor;
  protected String routeId;
  protected Integer serviceId;
  protected Integer priority;
  protected String srcSmeId;
  protected String dstSmeId;
  protected Short concatMsgRef;
  protected Short concatSeqNum;
  protected Long pointer;
  protected Integer bodyLen;
  protected Address originatingAddressMask;
  protected Address destinationAddressMask;
  protected Address dealiasedDestinationAddressMask;

  protected Status status;

  protected String originalText = null;
  protected String text = "";
  protected boolean textEncoded = false;
  protected byte arc = -1;

  private boolean marked = false;

  private Map<String, Object> parameters = new ConcurrentHashMap<String, Object>();

  protected byte body[] = null;

  private final SimpleDateFormat formatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

  public void setId(long id) {
    this.id = id;
  }

  public long getId() {
    return id;
  }

  public String getIdString() {
    return Long.toHexString(getId()).toUpperCase();
  }

  public String getOriginatingAddress() {
    return originatingAddress;
  }

  public void setOriginatingAddress(String address) throws AdminException {
    originatingAddress = address;
    try {
      originatingAddressMask = new Address(address);
    } catch (Exception ex) {
      originatingAddressMask = new Address(".5.0.inv_addr");
    }
  }

  public void setOriginatingAddress(Address mask) {
    originatingAddress = mask.getMask();
    originatingAddressMask = mask;
  }

  public String getDestinationAddress() {
    return destinationAddress;
  }

  public void setDestinationAddress(String address) throws AdminException {
    destinationAddress = address;
    try {
      destinationAddressMask = new Address(address);
    } catch (Exception ex) {
      destinationAddressMask = new Address(".5.0.invalid_addr");
    }
  }

  public void setDestinationAddress(Address mask) {
    destinationAddress = mask.getMask();
    destinationAddressMask = mask;
  }

  public String getDealiasedDestinationAddress() {
    return dealiasedDestinationAddress;
  }

  public void setDealiasedDestinationAddress(String address) throws AdminException {
    dealiasedDestinationAddress = address;
    try {
      dealiasedDestinationAddressMask = new Address(address);
    } catch (Exception ex) {
      dealiasedDestinationAddressMask = new Address(".5.0.invalid_addr");
    }
  }

  public void setDealiasedDestinationAddress(Address mask) {
    dealiasedDestinationAddress = mask.getMask();
    dealiasedDestinationAddressMask = mask;
  }

  public String getToString() {
    return ((getDealiasedDestinationAddress() == null || getDealiasedDestinationAddress().length() == 0 ||
        getDestinationAddress().equalsIgnoreCase(getDealiasedDestinationAddress())) ? getDestinationAddress() : (getDestinationAddress() + " (" + getDealiasedDestinationAddress() + ")"));
  }

  public String getDateString() {
    return formatter.format(getSubmitTime());
  }

  public Date getSubmitTime() {
    return submitTime == null ? new Date() : submitTime;
  }

  public void setSubmitTime(Date submitTime) {
    this.submitTime = submitTime;
  }

  public static String getStatusString(int status) {
    Status s;
    return (s = Status.get(status)) != null ? s.toString() : "UNKNOWN";
  }

  public String getStatus() {
    return getStatusString(getStatusInt());
  }

  public int getStatusInt() {
    return status == null ? 0 : status.getCode();
  }

  public void setStatus(int status) {
    this.status = Status.get(status);
  }

  public int getLastResult() {
    return lastResult == null ? 0 : lastResult;
  }

  public void setLastResult(int lastResult) {
    this.lastResult = lastResult;
  }

  public String getText() {
    return text;
  }

  public void setText(String text) {
    this.text = text;
  }

  public boolean isMarked() {
    return marked;
  }

  public void mark() {
    marked = true;
  }

  public void unmark() {
    marked = false;
  }

  public int getAttempts() {
    return attempts == null ? 0 : attempts;
  }

  public short getBillingRecord() {
    return billingRecord == null ? 0 : billingRecord;
  }

  public short getConcatMsgRef() {
    return concatMsgRef == null ? 0 : concatMsgRef;
  }

  public short getConcatSeqNum() {
    return concatSeqNum == null ? 0 : concatSeqNum;
  }

  public short getDeliveryReport() {
    return deliveryReport == null ? 0 : deliveryReport;
  }

  public SmsDescriptor getDestinationDescriptor() {
    return destinationDescriptor;
  }

  public String getDstSmeId() {
    return dstSmeId;
  }

  public Date getLastTryTime() {
    return lastTryTime;
  }

  public int getMessageReference() {
    return messageReference == null ? 0 : messageReference;
  }

  public boolean isNeedArchivate() {
    return needArchivate;
  }

  public Date getNextTryTime() {
    return nextTryTime;
  }

  public SmsDescriptor getOriginatingDescriptor() {
    return originatingDescriptor;
  }

  public int getPriority() {
    return priority == null ? 0 : priority;
  }

  public String getRouteId() {
    return routeId;
  }

  public int getServiceId() {
    return serviceId == null ? 0 : serviceId;
  }

  public String getServiceType() {
    return serviceType;
  }

  public String getSrcSmeId() {
    return srcSmeId;
  }

  public Date getValidTime() {
    return validTime;
  }

  public String getValidString() {
    return formatter.format(getValidTime());
  }

  public void setAttempts(int attempts) {
    this.attempts = attempts;
  }

  public void setBillingRecord(short billingRecord) {
    this.billingRecord = billingRecord;
  }

  public void setConcatMsgRef(short concatMsgRef) {
    this.concatMsgRef = concatMsgRef;
  }

  public void setConcatSeqNum(short concatSeqNum) {
    this.concatSeqNum = concatSeqNum;
  }

  public void setDeliveryReport(short deliveryReport) {
    this.deliveryReport = deliveryReport;
  }

  public void setDestinationDescriptor(SmsDescriptor destinationDescriptor) {
    this.destinationDescriptor = destinationDescriptor;
  }

  public void setDstSmeId(String dstSmeId) {
    this.dstSmeId = dstSmeId;
  }

  public void setLastTryTime(Date lastTryTime) {
    this.lastTryTime = lastTryTime;
  }

  public void setMessageReference(int messageReference) {
    this.messageReference = messageReference;
  }

  public void setNeedArchivate(boolean needArchivate) {
    this.needArchivate = needArchivate;
  }

  public void setNextTryTime(Date nextTryTime) {
    this.nextTryTime = nextTryTime;
  }

  public void setOriginatingDescriptor(SmsDescriptor originatingDescriptor) {
    this.originatingDescriptor = originatingDescriptor;
  }

  public void setPriority(int priority) {
    this.priority = priority;
  }

  public void setRouteId(String routeId) {
    this.routeId = routeId;
  }

  public void setServiceId(int serviceId) {
    this.serviceId = serviceId;
  }

  public void setServiceType(String serviceType) {
    this.serviceType = serviceType;
  }

  public void setSrcSmeId(String srcSmeId) {
    this.srcSmeId = srcSmeId;
  }

  public void setValidTime(Date validTime) {
    this.validTime = validTime;
  }

  public Address getDealiasedDestinationAddressMask() {
    return dealiasedDestinationAddressMask;
  }

  public Address getDestinationAddressMask() {
    return destinationAddressMask;
  }

  public Address getOriginatingAddressMask() {
    return originatingAddressMask;
  }

  public boolean isTextEncoded() {
    return textEncoded;
  }

  public void setTextEncoded(boolean textEncoded) {
    this.textEncoded = textEncoded;
  }

  public void addBodyParameter(short tag, Object value) {
    parameters.put(String.valueOf(tag), value);
  }

  public Map getBodyParameters() {
    byte[] body = getBody();
    if (body != null)
      SmsRowParser.parse(new ByteArrayInputStream(body, 0, body.length), this);
    return parameters;
  }

  public boolean equals(Object obj) {
    if (obj instanceof SmsRow) {
      return (((SmsRow) obj).getId() == id);
    } else
      return super.equals(obj);
  }

  public long getPointer() {
    return pointer == null ? 0 : pointer;
  }

  public void setPointer(long pointer) {
    this.pointer = pointer;
  }

  public byte[] getBody() {
    return body;
  }

  public void setBody(byte[] body) {
    this.body = body;
  }

  public int getBodyLen() {
    return bodyLen == null ? 0 : bodyLen;
  }

  public void setBodyLen(int bodyLen) {
    this.bodyLen = bodyLen;
  }

  public byte getArc() {
    return arc;
  }

  public void setArc(byte arc) {
    this.arc = arc;
  }

  public String getOriginalText() {
    return originalText;
  }

  public void setOriginalText(String originalText) {
    this.originalText = originalText;
  }


  private static enum Status {
    MSG_STATE_ENROUTE(0),
    MSG_STATE_DELIVERED(1),
    MSG_STATE_EXPIRED(2),
    MSG_STATE_UNDELIVERABLE(3),
    MSG_STATE_DELETED(4);
    private static final Map<Integer, Status> map = new HashMap<Integer, Status>(Status.values().length + 1) {
      {
        for (Status s : Status.values()) {
          put(s.getCode(), s);
        }
      }
    };
    private final int code;

    private Status(int code) {
      this.code = code;
    }

    public int getCode() {
      return code;
    }

    public static Status get(int code) {
      return map.get(code);
    }
  }


}