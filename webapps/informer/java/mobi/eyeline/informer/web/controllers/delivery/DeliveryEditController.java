package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryMode;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryEditController extends DeliveryController{


  private static final Logger logger = Logger.getLogger(DeliveryEditController.class);

  private Integer id;

  private Delivery delivery;

  private String comeBackParam;

  public DeliveryEditController() {
    super();

    String p = getRequestParameter(DELIVERY_PARAM);
    id = p==null || (p = p.trim()).length() == 0 ? null : Integer.parseInt(p);

    p = getRequestParameter(DELIVERY_COMEBACK_PARAM);
    if(p != null && p.length() != 0) {
      comeBackParam = p;
    }

    try{
      reload();
    }catch (AdminException e){
      addError(e);
    }
  }

  public String getComeBackParam() {
    return comeBackParam;
  }

  public void setComeBackParam(String comeBackParam) {
    this.comeBackParam = comeBackParam;
  }

  private void reload() throws AdminException{
    User u = config.getUser(getUserName());
    if(id == null || (delivery = config.getDelivery(u.getLogin(), u.getPassword(), id)) == null) {
      logger.error("Delivery is not found with id="+id);
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "delivery.not.found",id);
      return;
    }
    String p = delivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
    if(p != null) {
      smsNotificationAddress = new Address(p);
    }
    emailNotificationAddress = delivery.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
    secret = Boolean.valueOf(delivery.getProperty(UserDataConsts.SECRET));
    flashSecret = Boolean.valueOf(delivery.getProperty(UserDataConsts.SECRET_FLASH));
    secretMessage = delivery.getProperty(UserDataConsts.SECRET_TEXT);
  }

  public List<SelectItem> getUniqueDeliveryModes() {
    List<SelectItem> sIs = new LinkedList<SelectItem>();
    for(DeliveryMode m : DeliveryMode.values()) {
      sIs.add(new SelectItem(m, m.toString()));
    }
    return sIs;
  }

  public List<SelectItem> getAllDays() {
    List<SelectItem> result = new ArrayList<SelectItem>(7);
    int i=1;
    for(Delivery.Day d : Delivery.Day.values()) {
      result.add(new SelectItem(d, getLocalizedString("weekday."+i%7)));
      i++;
    }
    return result;
  }

  private String emailNotificationAddress;

  public String getEmailNotificationAddress() {
    return emailNotificationAddress;
  }

  public void setEmailNotificationAddress(String emailNotificationAddress) {
    this.emailNotificationAddress = emailNotificationAddress;
  }

  private Address smsNotificationAddress;

  public Address getSmsNotificationAddress() {
    return smsNotificationAddress;
  }

  public void setSmsNotificationAddress(Address smsNotificationAddress) {
    this.smsNotificationAddress = smsNotificationAddress;
  }

  public boolean secret;

  public String secretMessage;

  public boolean flashSecret;

  public boolean isSecret() {
    return secret;
  }

  public void setSecret(boolean secret) {
    this.secret = secret;
  }

  public String getSecretMessage() {
    return secretMessage;
  }

  public void setSecretMessage(String secretMessage) {
    this.secretMessage = secretMessage;
  }

  public boolean isFlashSecret() {
    return flashSecret;
  }

  public void setFlashSecret(boolean flashSecret) {
    this.flashSecret = flashSecret;
  }

  public String save() {
    if(!delivery.isRetryOnFail()) {
      delivery.setRetryPolicy(null);
    }else if(delivery.getRetryPolicy() == null || !getRetryPoliciesPattern().matcher(delivery.getRetryPolicy()).matches()) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "deliver.illegal_retry_policy", delivery.getRetryPolicy() == null ? "" : delivery.getRetryPolicy());
      return null;
    }
    if(delivery.getType() == Delivery.Type.SingleText && (delivery.getSingleText() == null || delivery.getSingleText().length() == 0)) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "deliver.illegal_single_text");
      return null;
    }

    if(emailNotificationAddress != null && (emailNotificationAddress = emailNotificationAddress.trim()).length() != 0) {
      delivery.setProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS, emailNotificationAddress);
    }else {
      delivery.removeProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
    }
    if(smsNotificationAddress != null) {
      delivery.setProperty(UserDataConsts.SMS_NOTIF_ADDRESS, smsNotificationAddress.getSimpleAddress());
    }else {
      delivery.removeProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
    }

    try{
      User u = config.getUser(getUserName());
      config.modifyDelivery(u.getLogin(), u.getPassword(), delivery);
    }catch (AdminException e){
      addError(e);
      return null;
    }
    return comeBackParam != null && comeBackParam.length() != 0 ? comeBackParam : "DELIVERIES";
  }

  public String cancel() {
    String p = comeBackParam != null && comeBackParam.length() > 0 ? comeBackParam : getRequestParameter(DELIVERY_COMEBACK_PARAM);
    return p != null && p.length() >0 ? p : "DELIVERIES";

  }

  public String sendTest() {
    if(delivery.getType() != Delivery.Type.SingleText) {
      return null;
    }
    try{
      User u = config.getUser(getUserName());
      TestSms sms = new TestSms();
      sms.setDestAddr(new Address(u.getPhone()));
      sms.setFlash(delivery.isFlash());
      switch (delivery.getDeliveryMode()) {
        case USSD_PUSH: sms.setMode(TestSms.Mode.USSD_PUSH); break;
        case SMS: sms.setMode(TestSms.Mode.SMS); break;
        case USSD_PUSH_VLR: sms.setMode(TestSms.Mode.USSD_PUSH_VLR); break;
      }
      sms.setSourceAddr(delivery.getSourceAddress());
      sms.setText(delivery.getSingleText());
      config.sendTestSms(sms);
      addLocalizedMessage(FacesMessage.SEVERITY_INFO, "delivery.test.sms", u.getPhone());
    }catch (AdminException e) {
      addError(e);
    }
    return null;
  }



  public Delivery getDelivery() {
    return delivery;
  }

  public Integer getId() {
    return id;
  }

  public void setId(Integer id) {
    this.id = id;
  }
}
