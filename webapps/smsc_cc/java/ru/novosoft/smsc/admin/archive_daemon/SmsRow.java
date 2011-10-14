package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.util.Address;

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

  private long id;
  private Date submitTime = null;
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
  private boolean needArchivate;
  private Short deliveryReport;
  private Short billingRecord;
  private SmsDescriptor originatingDescriptor;
  private SmsDescriptor destinationDescriptor;
  private String routeId;
  private Integer serviceId;
  private Integer priority;
  private String srcSmeId;
  private String dstSmeId;
  private Short concatMsgRef;
  private Short concatSeqNum;
  private Long pointer;
  private Integer bodyLen;

  private Status status;

  private String originalText = null;
  private String text = "";
  private boolean textEncoded = false;
  private byte arc = -1;

  private boolean marked = false;

  private Map<Integer, Object> parameters = new ConcurrentHashMap<Integer, Object>();

  private byte body[] = null;

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

  public Address getOriginatingAddress() {
    return originatingAddress;
  }

  public void setOriginatingAddress(Address address) {
    originatingAddress = address;
  }

  public Address getDestinationAddress() {
    return destinationAddress;
  }

  public void setDestinationAddress(Address address) {
    destinationAddress = address;
  }


  public Address getDealiasedDestinationAddress() {
    return dealiasedDestinationAddress;
  }

  public void setDealiasedDestinationAddress(Address address) {
    dealiasedDestinationAddress = address;
  }


  public Date getSubmitTime() {
    return submitTime == null ? null : submitTime;
  }

  public void setSubmitTime(Date submitTime) {
    this.submitTime = submitTime;
  }

  private String getStatusString(int status) {
    Status s;
    return (s = Status.get(status)) != null ? s.toString() : "UNKNOWN";
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

  public boolean isTextEncoded() {
    return textEncoded;
  }

  public void setTextEncoded(boolean textEncoded) {
    this.textEncoded = textEncoded;
  }

  public void addBodyParameter(short tag, Object value) {
    parameters.put((int)tag, value);
  }

  public Map<Integer, Object> getBodyParameters() {
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

  public Status getStatus() {
    return status;
  }

  public void setStatus(Status status) {
    this.status = status;
  }

  public static enum Status {
    ENROUTE(0),
    DELIVERED(1),
    EXPIRED(2),
    UNDELIVERABLE(3),
    DELETED(4);
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
