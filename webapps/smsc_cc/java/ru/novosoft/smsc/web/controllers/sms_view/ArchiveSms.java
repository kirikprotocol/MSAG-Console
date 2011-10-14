package ru.novosoft.smsc.web.controllers.sms_view;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.SmsDescriptor;
import ru.novosoft.smsc.admin.archive_daemon.SmsRow;
import ru.novosoft.smsc.util.Address;

import java.util.Date;
import java.util.Map;
import java.util.ResourceBundle;

/**
* author: Aleksandr Khalitov
*/
public class ArchiveSms extends Sms {

  private final SmsRow r;

  private boolean showText;

  public ArchiveSms(SmsRow r, boolean showText, ResourceBundle bundle) {
    super(bundle);
    this.r = r;
    this.showText = showText;
  }

  public long getId() throws AdminException {
    return r.getId();
  }

  public Date getSubmitTime() throws AdminException {
    return r.getSubmitTime();
  }

  public Date getValidTime() throws AdminException {
    return r.getValidTime();
  }

  public Integer getAttempts() throws AdminException {
    return r.getAttempts();
  }

  public Integer getLastResult() throws AdminException {
    return r.getLastResult();
  }

  public Date getLastTryTime() throws AdminException {
    return r.getLastTryTime();
  }

  public Date getNextTryTime() throws AdminException {
    return r.getNextTryTime();
  }

  public Address getOriginatingAddress() throws AdminException {
    return new Address(r.getOriginatingAddress());
  }

  public Address getDestinationAddress() throws AdminException {
    return new Address(r.getDestinationAddress());
  }

  public Address getDealiasedDestinationAddress() throws AdminException {
    return new Address(r.getDealiasedDestinationAddress());
  }

  public Integer getMessageReference() throws AdminException {
    return r.getMessageReference();
  }

  public String getServiceType() throws AdminException {
    return r.getServiceType();
  }

  public Integer getDeliveryReport() throws AdminException {
    return (int) r.getDeliveryReport();
  }

  public Integer getBillingRecord() throws AdminException {
    return (int) r.getBillingRecord();
  }

  public RoutingInfo getOriginatingDescriptor() throws AdminException {
    SmsDescriptor d = r.getOriginatingDescriptor();
    return new RoutingInfo(d.getImsi(), d.getMsc(), d.getSme());
  }

  public RoutingInfo getDestinationDescriptor() throws AdminException {
    SmsDescriptor d = r.getDestinationDescriptor();
    return new RoutingInfo(d.getImsi(), d.getMsc(), d.getSme());
  }

  public String getRouteId() throws AdminException {
    return r.getRouteId();
  }

  public Integer getServiceId() throws AdminException {
    return r.getServiceId();
  }

  public Integer getPriority() throws AdminException {
    return r.getPriority();
  }

  public String getSrcSmeId() throws AdminException {
    return r.getSrcSmeId();
  }

  public String getDstSmeId() throws AdminException {
    return r.getDstSmeId();
  }

  public byte getArc() throws AdminException {
    return r.getArc();
  }

  public byte[] getBody() throws AdminException {
    return r.getBody();
  }

  public String getOriginalText() throws AdminException {
    return r.getOriginalText();
  }

  public String getText() throws AdminException {
    return r.getText();
  }

  public boolean isTextEncoded() throws AdminException {
    return r.isTextEncoded();
  }

  public SmsStatus getStatus() throws AdminException {
    return SmsStatus.valueOf(r.getStatus().toString());
  }

  @Override
  protected Map<Integer, Object> loadBodyParameters() throws AdminException {
    return r.getBodyParameters();
  }

  public boolean isShowText() {
    return showText;
  }
}
