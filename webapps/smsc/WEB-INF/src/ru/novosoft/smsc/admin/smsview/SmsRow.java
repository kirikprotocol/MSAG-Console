package ru.novosoft.smsc.admin.smsview;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;

import java.io.ByteArrayInputStream;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Hashtable;

public class SmsRow {
  private static int numStates = 5;
  public static int MSG_STATE_ENROUTE = 0;
  public static int MSG_STATE_DELIVERED = 1;
  public static int MSG_STATE_EXPIRED = 2;
  public static int MSG_STATE_UNDELIVERABLE = 3;
  public static int MSG_STATE_DELETED = 4;
  private static String states[] = {"ENROUTE", "DELIVERED", "EXPIRED", "UNDELIVERABLE", "DELETED"};

  protected long id;
  protected Date submitTime = null;
  protected  Date validTime;
  protected  Integer attempts;
  protected  Integer lastResult;
  protected  Date lastTryTime;
  protected  Date nextTryTime;
  protected  String originatingAddress = "originatingAddress";
  protected  String destinationAddress = "destinationAddress";
  protected  String dealiasedDestinationAddress = "dealiasedDestinationAddress";
  protected  Integer messageReference;
  protected  String serviceType;
  protected  boolean needArchivate;
  protected  Short deliveryReport;
  protected  Short billingRecord;
  protected  SmsDescriptor originatingDescriptor;
  protected  SmsDescriptor destinationDescriptor;
  protected  String routeId;
  protected  Integer serviceId;
  protected  Integer priority ;
  protected  String srcSmeId;
  protected  String dstSmeId;
  protected  Short concatMsgRef;
  protected  Short concatSeqNum;
  protected  Long pointer;
  protected  Integer bodyLen;
  protected  Mask originatingAddressMask;
  protected  Mask destinationAddressMask;
  protected  Mask dealiasedDestinationAddressMask;

  protected  Integer status;


  protected  String originalText = null;
  protected  String text = "";
  protected  boolean textEncoded = false;
  protected  byte arc = -1;

  private boolean marked = false;

  private Hashtable parameters;
  protected byte body[] = null;

  public void setId(long id) {
    this.id = id;
  }

  ;

  public long getId() {
    return id;
  }

  ;

  public String getIdString() {
    return Long.toHexString(getId()).toUpperCase();
  }

  ;

  public String getOriginatingAddress() {
    return originatingAddress;
  }

  public void setOriginatingAddress(String address) throws AdminException {
    originatingAddress = address;
    try {
      originatingAddressMask = new Mask(address);
    } catch (Exception ex) {
      originatingAddressMask = new Mask(".5.0.inv_addr");
    }
  }

  public void setOriginatingAddress(Mask mask) {
    originatingAddress = mask.getMask();
    originatingAddressMask = mask;
  }

  public String getDestinationAddress() {
    return destinationAddress;
  }

  ;

  public void setDestinationAddress(String address) throws AdminException {
    destinationAddress = address;
    try {
      destinationAddressMask = new Mask(address);
    } catch (Exception ex) {
      destinationAddressMask = new Mask(".5.0.invalid_addr");
    }
  }

  public void setDestinationAddress(Mask mask) {
    destinationAddress = mask.getMask();
    destinationAddressMask = mask;
  }

  public String getDealiasedDestinationAddress() {
    return dealiasedDestinationAddress;
  }

  ;

  public void setDealiasedDestinationAddress(String address) throws AdminException {
    dealiasedDestinationAddress = address;
    try {
      dealiasedDestinationAddressMask = new Mask(address);
    } catch (Exception ex) {
      dealiasedDestinationAddressMask = new Mask(".5.0.invalid_addr");
    }
  }

  public void setDealiasedDestinationAddress(Mask mask) {
    dealiasedDestinationAddress = mask.getMask();
    dealiasedDestinationAddressMask = mask;
  }

  public String getToString() {
    return ((getDealiasedDestinationAddress() == null || getDealiasedDestinationAddress().length() == 0 ||
            getDestinationAddress().equalsIgnoreCase(getDealiasedDestinationAddress())) ? getDestinationAddress() : (getDestinationAddress() + " (" + getDealiasedDestinationAddress() + ")"));
  }

  public String getDateString() {
    SimpleDateFormat formatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
    return formatter.format(getSubmitTime());
  }

  public Date getSubmitTime() {
    return submitTime == null ? new Date() : submitTime;
  }

  public void setSubmitTime(Date submitTime) {
    this.submitTime = submitTime;
  }

  public static String getStatusString(int status) {
    return (status >= 0 && status < numStates) ? states[status] : "UNKNOWN";
  }

  public String getStatus() {
    return getStatusString(getStatusInt());
  }

  public int getStatusInt() {
    return status == null ? 0 : status.intValue();
  }

  public void setStatus(int status) {
    this.status = new Integer(status);
  }

  public int getLastResult() {
    return lastResult == null ? 0 : lastResult.intValue();
  }

  public void setLastResult(int lastResult) {
    this.lastResult = new Integer(lastResult);
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
    return attempts == null ? 0 : attempts.intValue();
  }

  public short getBillingRecord() {
    return billingRecord == null ? 0 : billingRecord.shortValue();
  }

  public short getConcatMsgRef() {
    return concatMsgRef == null ? 0 : concatMsgRef.shortValue();
  }

  public short getConcatSeqNum() {
    return concatSeqNum == null ? 0 : concatSeqNum.shortValue();
  }

  public short getDeliveryReport() {
    return deliveryReport == null ? 0 : deliveryReport.shortValue();
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
    return messageReference == null ? 0 : messageReference.intValue();
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
    return priority == null ? 0 : priority.intValue();
  }

  public String getRouteId() {
    return routeId;
  }

  public int getServiceId() {
    return serviceId == null ? 0 : serviceId.intValue();
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
    SimpleDateFormat formatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
    return formatter.format(getValidTime());
  }

  public void setAttempts(int attempts) {
    this.attempts = new Integer(attempts);
  }

  public void setBillingRecord(short billingRecord) {
    this.billingRecord = new Short(billingRecord);
  }

  public void setConcatMsgRef(short concatMsgRef) {
    this.concatMsgRef = new Short(concatMsgRef);
  }

  public void setConcatSeqNum(short concatSeqNum) {
    this.concatSeqNum = new Short(concatSeqNum);
  }

  public void setDeliveryReport(short deliveryReport) {
    this.deliveryReport = new Short(deliveryReport);
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
    this.messageReference = new Integer(messageReference);
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
    this.priority = new Integer(priority);
  }

  public void setRouteId(String routeId) {
    this.routeId = routeId;
  }

  public void setServiceId(int serviceId) {
    this.serviceId = new Integer(serviceId);
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

  public Mask getDealiasedDestinationAddressMask() {
    return dealiasedDestinationAddressMask;
  }

  public Mask getDestinationAddressMask() {
    return destinationAddressMask;
  }

  public Mask getOriginatingAddressMask() {
    return originatingAddressMask;
  }

  public boolean isTextEncoded() {
    return textEncoded;
  }

  public void setTextEncoded(boolean textEncoded) {
    this.textEncoded = textEncoded;
  }

  public void addBodyParameter(short tag, Object value) {
    if (parameters == null)
      parameters = new Hashtable();
    parameters.put(String.valueOf(tag), value);
  }

  public Hashtable getBodyParameters() {
    byte[] body = getBody();
    if (body != null)
      SmsSource.parseBody(new ByteArrayInputStream(body, 0, body.length), this);
    return parameters == null ? new Hashtable() : parameters;
  }

  public boolean equals(Object obj) {
    if (obj instanceof SmsRow) {
      return (((SmsRow) obj).getId() == id);
    } else
      return super.equals(obj);
  }

  public long getPointer() {
    return pointer == null ? 0 : pointer.longValue();
  }

  public void setPointer(long pointer) {
    this.pointer = new Long(pointer);
  }

  public byte[] getBody() {
    return body;
  }

  public void setBody(byte[] body) {
    this.body = body;
  }

  public int getBodyLen() {
    return bodyLen == null ? 0 : bodyLen.intValue();
  }

  public void setBodyLen(int bodyLen) {
    this.bodyLen = new Integer(bodyLen);
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
}
