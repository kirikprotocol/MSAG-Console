package ru.novosoft.smsc.web.controllers.sms_view;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.StringEncoderDecoder;

/**
 * author: Aleksandr Khalitov
 */
public class SmsController extends SmsViewController{

  private Sms sms;

  private String comeBackAction;

  public static final String SMS_PARAM_NAME = "sms";
  public static final String COME_BACK_PARAM_NAME = "sms";

  public static final String ACTION = "SMS_VIEW";

  public String getSmsParamName() {
    return SMS_PARAM_NAME;
  }

  public String getComeBackParamName() {
    return COME_BACK_PARAM_NAME;
  }

  public String getComeBackAction() {
    return comeBackAction;
  }

  public Long getId() {
    try {
      return sms == null ? null : sms.getId();
    } catch (AdminException e) {
      addError(e);
      return null;
    }
  }

  public boolean isShow() {
    return sms != null;
  }

  public Sms getSms() {
    return sms;
  }

  public String getOrigDescriptor() {
    if(sms == null) {
      return null;
    }
    try {
      return getDescriptor(sms.getOriginatingDescriptor());
    } catch (AdminException e) {
      addError(e);
      return null;
    }
  }

  public String getDestDescriptor() {
    if(sms == null) {
      return null;
    }
    try {
      return getDescriptor(sms.getDestinationDescriptor());
    } catch (AdminException e) {
      addError(e);
      return null;
    }
  }

  public String getText() {
    if(sms == null) {
      return null;
    }
    try {
      return (sms.getText() != null && sms.isTextEncoded()) ? sms.getText() : StringEncoderDecoder.encode(sms.getText());
    } catch (AdminException e) {
      addError(e);
      return null;
    }
  }

  private String getDescriptor(RoutingInfo info) throws AdminException {
    String oimsi = info.getImsi();
    if (oimsi == null || oimsi.length() == 0)
      oimsi = "-";
    String omsc = info.getMsc();
    if (omsc == null || omsc.length() == 0)
      omsc = "-";
    String odesc = oimsi + " / " + omsc;
    return (sms.getText() != null && sms.isTextEncoded()) ? odesc : StringEncoderDecoder.encode(odesc);
  }

  public String setSms() {
    sms = (Sms)getRequest().get(SMS_PARAM_NAME);
    comeBackAction = getRequestParameter(COME_BACK_PARAM_NAME);
    return ACTION;
  }

}
