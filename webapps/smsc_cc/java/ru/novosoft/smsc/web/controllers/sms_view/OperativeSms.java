package ru.novosoft.smsc.web.controllers.sms_view;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.operative_store.Message;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.util.Date;

/**
 * author: Aleksandr Khalitov
 */
public class OperativeSms implements Sms {

  private final Message m;

  private boolean showText;

  public OperativeSms(Message m, boolean showText) {
    this.m = m;
    this.showText = showText;
  }

  public long getId() throws AdminException {
    return m.getId();
  }

  public Date getSubmitTime() throws AdminException {
    return m.getSubmitTime();
  }

  public Date getValidTime() throws AdminException {
    return m.getValidTime();
  }

  public Integer getAttempts() throws AdminException {
    return m.getAttempts();
  }

  public Integer getLastResult() throws AdminException {
    return m.getLastResult();
  }

  public Date getLastTryTime() throws AdminException {
    return m.getLastTryTime();
  }

  public Date getNextTryTime() throws AdminException {
    return m.getNextTryTime();
  }

  public Address getOriginatingAddress() throws AdminException {
    return m.getOriginatingAddress();
  }

  public Address getDestinationAddress() throws AdminException {
    return m.getDestinationAddress();
  }

  public Address getDealiasedDestinationAddress() throws AdminException {
    return m.getDealiasedDestinationAddress();
  }

  public Integer getMessageReference() throws AdminException {
    return m.getMessageReference();
  }

  public String getServiceType() throws AdminException {
    return m.getServiceType();
  }

  public Integer getDeliveryReport() throws AdminException {
    return m.getDeliveryReport();
  }

  public Integer getBillingRecord() throws AdminException {
    return m.getBillingRecord();
  }

  public RoutingInfo getOriginatingDescriptor() throws AdminException {
    ru.novosoft.smsc.admin.operative_store.RoutingInfo i = m.getOriginatingDescriptor();
    return new RoutingInfo(i.getImsi(), i.getMsc(), i.getSme());
  }

  public RoutingInfo getDestinationDescriptor() throws AdminException {
    ru.novosoft.smsc.admin.operative_store.RoutingInfo i = m.getDestinationDescriptor();
    return new RoutingInfo(i.getImsi(), i.getMsc(), i.getSme());
  }

  public String getRouteId() throws AdminException {
    return m.getRouteId();
  }

  public Integer getServiceId() throws AdminException {
    return m.getServiceId();
  }

  public Integer getPriority() throws AdminException {
    return m.getPriority();
  }

  public String getSrcSmeId() throws AdminException {
    return m.getSrcSmeId();
  }

  public String getDstSmeId() throws AdminException {
    return m.getDstSmeId();
  }

  public byte getArc() throws AdminException {
    return m.getArc();
  }

  public byte[] getBody() throws AdminException {
    return m.getBody();
  }

  public String getOriginalText() throws AdminException {
    return m.getOriginalText();
  }

  public String getText() throws AdminException {
    String t = m.getText();
    if(t != null && isTextEncoded()) {
      t = StringEncoderDecoder.encode(t);
    }
    return t;
  }

  public boolean isTextEncoded() throws AdminException {
    return m.isTextEncoded();
  }

  public SmsStatus getStatus() throws AdminException {
    return SmsStatus.valueOf(m.getStatus().toString());
  }

  public boolean isShowText() {
    return showText;
  }
}

