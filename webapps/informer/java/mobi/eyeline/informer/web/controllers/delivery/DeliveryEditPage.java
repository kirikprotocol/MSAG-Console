package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.io.File;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryEditPage extends InformerController implements CreateDeliveryPage{

  private Delivery delivery;

  private File tmpFile;

  private boolean smsNotificationCheck;

  private boolean emailNotificationCheck;

  private Address smsNotificationAddress;

  private String emailNotificationAddress;

  private boolean secret;

  private String secretMessage;

  private boolean secretFlash;


  private Configuration config;

  public DeliveryEditPage(Delivery delivery, File tmpFile, Configuration config) {
    this.delivery = delivery;
    this.tmpFile = tmpFile;
    this.config = config;

    String p = delivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
    if(p != null) {
      smsNotificationAddress = new Address(p);
    }
    emailNotificationAddress = delivery.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
    secret = Boolean.valueOf(delivery.getProperty(UserDataConsts.SECRET));
    secretFlash = Boolean.valueOf(delivery.getProperty(UserDataConsts.SECRET_FLASH));
    secretMessage = delivery.getProperty(UserDataConsts.SECRET_TEXT);
  }


  public Address getSmsNotificationAddress() {
    return smsNotificationAddress;
  }

  public void setSmsNotificationAddress(Address smsNotificationAddress) {
    this.smsNotificationAddress = smsNotificationAddress;
  }

  public String getEmailNotificationAddress() {
    return emailNotificationAddress;
  }

  public void setEmailNotificationAddress(String emailNotificationAddress) {
    this.emailNotificationAddress = emailNotificationAddress;
  }

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

  public boolean isSecretFlash() {
    return secretFlash;
  }

  public void setSecretFlash(boolean secretFlash) {
    this.secretFlash = secretFlash;
  }

  public CreateDeliveryPage process(String user, Configuration config, Locale locale) throws AdminException {
    if(secret && (secretMessage == null || (secretMessage = secretMessage.trim()).length() == 0)) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.secret.message.empty");
      return null;
    }
    if(smsNotificationCheck) {
      if(smsNotificationAddress == null) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.sms.incorrect");
        return null;
      }else {
        delivery.setProperty(UserDataConsts.SMS_NOTIF_ADDRESS, smsNotificationAddress.getSimpleAddress());
      }
    }
    if(emailNotificationCheck) {
      if(emailNotificationAddress == null || (emailNotificationAddress = emailNotificationAddress.trim()).length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.email.incorrect");
        return null;
      }else {
        delivery.setProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS, emailNotificationAddress);
      }
    }
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
    User u = config.getUser(user);
    DeliveryFilter filter = new DeliveryFilter();
    filter.setNameFilter(new String[]{delivery.getName()});
    final boolean[] intersection = new boolean[]{false};
    config.getDeliveries(u.getLogin(), u.getPassword(), filter, 1, new Visitor<DeliveryInfo>() {
      public boolean visit(DeliveryInfo value) throws AdminException {
        intersection[0] = true;
        return false;
      }
    });
    if(intersection[0]) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.name.exist");
      return null;
    }

    delivery.setProperty(UserDataConsts.SECRET, Boolean.toString(secret));
    if(secret) {
      delivery.setProperty(UserDataConsts.SECRET_TEXT, secretMessage);
      delivery.setProperty(UserDataConsts.SECRET_FLASH, Boolean.toString(secretFlash));
    }

    if(!smsNotificationCheck) {
      delivery.removeProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
    }
    if(!emailNotificationCheck) {
      delivery.removeProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
    }

    return new ProcessDeliveryPage(delivery, tmpFile, config, locale, user);
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

  public String getPageId() {
    return "DELIVERY_CREATE_EDIT";
  }

  public boolean isEmailNotificationCheck() {
    return emailNotificationCheck;
  }

  public void setEmailNotificationCheck(boolean emailNotificationCheck) {
    this.emailNotificationCheck = emailNotificationCheck;
  }

  public boolean isSmsNotificationCheck() {
    return smsNotificationCheck;
  }

  public void setSmsNotificationCheck(boolean smsNotificationCheck) {
    this.smsNotificationCheck = smsNotificationCheck;
  }

  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  public void cancel() {
    tmpFile.delete();
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
}
