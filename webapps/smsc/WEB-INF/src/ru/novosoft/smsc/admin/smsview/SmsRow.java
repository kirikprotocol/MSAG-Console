package ru.novosoft.smsc.admin.smsview;

/**
 * Title:        SMSC Java Administration
 * Description:
 * Copyright:    Copyright (c) 2002
 * Company:      Novosoft
 * @author
 * @version 1.0
 */

import java.util.Date;
import java.text.SimpleDateFormat;

public class SmsRow
{
  private static int numStates = 5;
  private static String states[] = {
    "ENROUTE", "DELIVERED", "EXPIRED", "UNDELIVERABLE", "DELETED"
  };

  private byte    id[];
  private Date    submitTime = new Date();
  private Date    validTime;
  private int     attempts;
  private int     lastResult = 0;
  private Date    lastTryTime;
  private Date    nextTryTime;
  private String  originatingAddress = "originatingAddress";
  private String  destinationAddress   = "destinationAddress";
  private String  dealiasedDestinationAddress   = "dealiasedDestinationAddress";
  private int     messageReference;
  private String  serviceType;
  private boolean needArchivate;
  private short   deliveryReport;
  private short   billingRecord;
  private SmsDescriptor originatingDescriptor;
  private SmsDescriptor destinationDescriptor;
  private String  routeId;
  private int     serviceId;
  private int     priority;
  private String  srcSmeId;
  private String  dstSmeId;
  private short   concatMsgRef;
  private short   concatSeqNum;


  private int     status  = 0;
  private String  text = "Message text ";


  private boolean marked = false;

  public void setId(byte _id[]) { id = _id; };
  public byte[] getId() { return id; };

  public long getIdLong() {
    return
            (((long)id[0])<<7*8)
            ||(((long)id[1])<<6*8)
            ||(((long)id[2])<<5*8)
            ||(((long)id[3])<<4*8)
            ||(((long)id[4])<<3*8)
            ||(((long)id[5])<<2*8)
            ||(((long)id[6])<<1*8)
            ||(((long)id[7]));
  }

  public String getIdString() {
    StringBuffer str = new StringBuffer();
    for (int i=0; i<id.length; i++) {
      int k = (((int)id[i])>>4)&0xf;
      str.append( (char)(k<10?k+48:k+55) );
      k = ((int)id[i])&0xf;
      str.append( (char)(k<10?k+48:k+55) );
    }
    return str.toString();
  };

  public String getOriginatingAddress() { return originatingAddress; }
  public void setOriginatingAddress(String address) { originatingAddress = address; }
  public String getDestinationAddress(){ return destinationAddress; };
  public void setDestinationAddress(String address) { destinationAddress = address; }
  public String getDealiasedDestinationAddress(){ return dealiasedDestinationAddress; };
  public void setDealiasedDestinationAddress(String address) { dealiasedDestinationAddress = address; }
  public String getToString() {
      return ((dealiasedDestinationAddress == null || dealiasedDestinationAddress.length() == 0 ||
               destinationAddress.equalsIgnoreCase(dealiasedDestinationAddress)) ? destinationAddress:(destinationAddress+" ("+dealiasedDestinationAddress+")"));
  }
  public String getDateString()
  {
      SimpleDateFormat formatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
      return formatter.format(submitTime);
  }
	public Date getSubmitTime()
	{
		return submitTime;
	}
  public void setSubmitTime(Date submitTime){ this.submitTime = submitTime; }
  public String getStatus() {
    String state = (status >= 0 && status < numStates) ? states[status]:"UNKNOUN";
    return state;
  }
  public void setStatus(int status){ this.status = status; }
  public int getLastResult() { return lastResult; }
  public void setLastResult(int lastResult) { this.lastResult = lastResult; }

  public String getText(){ return text; }
  public void setText(String text){ this.text = text; }

  public boolean isMarked() { return marked; }
  public void mark() { marked = true; }
  public void unmark() { marked = false; }

  public int getAttempts() {
    return attempts;
  }

  public short getBillingRecord() {
    return billingRecord;
  }

  public short getConcatMsgRef() {
    return concatMsgRef;
  }

  public short getConcatSeqNum() {
    return concatSeqNum;
  }

  public short getDeliveryReport() {
    return deliveryReport;
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
    return messageReference;
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
    return priority;
  }

  public String getRouteId() {
    return routeId;
  }

  public int getServiceId() {
    return serviceId;
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

  public String getValidString()
  {
      SimpleDateFormat formatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
      return formatter.format(validTime);
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
};