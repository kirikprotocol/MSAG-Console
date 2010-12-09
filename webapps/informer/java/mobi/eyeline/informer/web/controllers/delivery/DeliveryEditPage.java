package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.DeliveryMode;
import mobi.eyeline.informer.admin.delivery.DeliveryPrototype;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryEditPage extends InformerController implements CreateDeliveryPage {

  private final DeliveryPrototype delivery;


  private boolean smsNotificationCheck;

  private boolean emailNotificationCheck;

  private Address smsNotificationAddress;

  private String emailNotificationAddress;

  private boolean secret;

  private String secretMessage;

  private boolean secretFlash;

  private String retryOnFail;

  private DeliveryBuilder fact;

  private boolean singleText;


  private final Configuration config;

  public DeliveryEditPage(DeliveryBuilder fact, boolean singleText, Configuration config, String user) throws AdminException {
    this.delivery = new DeliveryPrototype();
    this.delivery.setStartDate(new Date());
    config.copyUserSettingsToDeliveryPrototype(user, delivery);

    this.fact = fact;
    this.singleText = singleText;
    this.config = config;

    if (delivery.isRetryOnFail()) {
      retryOnFail = (delivery.getRetryPolicy() == null || delivery.getRetryPolicy().length() == 0) ? "default" : "custom";
    } else {
      retryOnFail = "off";
    }

    User u = config.getUser(user);

    String p = delivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
    if (p != null) {
      smsNotificationAddress = new Address(p);
      smsNotificationCheck = true;
    } else if (u.getPhone() != null) {
      smsNotificationAddress = new Address(u.getPhone());
    }

    p = delivery.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
    if (p != null) {
      emailNotificationAddress = p;
      emailNotificationCheck = true;
    } else if (u.getEmail() != null) {
      emailNotificationAddress = u.getEmail();
    }

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

  public boolean isDeliveryWithSingleText() {
    return singleText;
  }

  public CreateDeliveryPage process(String user, Configuration config, Locale locale) throws AdminException {
    if(delivery.getName() == null || delivery.getName().length() == 0) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.name.empty");
      return null;

    }
    if (secret && (secretMessage == null || (secretMessage = secretMessage.trim()).length() == 0)) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.secret.message.empty");
      return null;
    }
    if (smsNotificationCheck) {
      if (smsNotificationAddress == null) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.sms.incorrect");
        return null;
      } else {
        delivery.setProperty(UserDataConsts.SMS_NOTIF_ADDRESS, smsNotificationAddress.getSimpleAddress());
        delivery.setEnableStateChangeLogging(true);
      }
    }
    if (emailNotificationCheck) {
      if (emailNotificationAddress == null || (emailNotificationAddress = emailNotificationAddress.trim()).length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.email.incorrect");
        return null;
      } else {
        delivery.setProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS, emailNotificationAddress);
        delivery.setEnableStateChangeLogging(true);
      }
    }

    if (retryOnFail.equals("off")) {
      delivery.setRetryOnFail(false);
      delivery.setRetryPolicy(null);
    } else if (retryOnFail.equals("default")) {
      delivery.setRetryOnFail(true);
      delivery.setRetryPolicy("");
    } else {
      delivery.setRetryOnFail(true);
    }

    if(singleText) {
      config.validateDeliveryWithSingleText(delivery);
    }else {
      config.validateDeliveryWithIndividualTexts(delivery);
    }

    delivery.setProperty(UserDataConsts.SECRET, Boolean.toString(secret));
    if (secret) {
      delivery.setProperty(UserDataConsts.SECRET_TEXT, secretMessage);
      delivery.setProperty(UserDataConsts.SECRET_FLASH, Boolean.toString(secretFlash));
    }

    if (!smsNotificationCheck) {
      delivery.removeProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
    }
    if (!emailNotificationCheck) {
      delivery.removeProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
    }

    return new ProcessDeliveryPage(delivery, fact, locale);
  }

  public List<SelectItem> getUniqueDeliveryModes() {
    List<SelectItem> sIs = new LinkedList<SelectItem>();
    for (DeliveryMode m : DeliveryMode.values()) {
      sIs.add(new SelectItem(m, m.toString()));
    }
    return sIs;
  }

  public List<SelectItem> getAllDays() {
    List<SelectItem> result = new ArrayList<SelectItem>(7);
    int i = 1;
    for (Day d : Day.values()) {
      result.add(new SelectItem(d, getLocalizedString("weekday." + i % 7)));
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

  public void setActivePeriodEnd(Date t) throws AdminException {
    if(t==null) delivery.setActivePeriodEnd(null);
    else delivery.setActivePeriodEnd(new Time(t));
  }
  public void setActivePeriodStart(Date t) throws AdminException {
    if(t==null) delivery.setActivePeriodStart(null);
    else delivery.setActivePeriodStart(new Time(t));
  }

  public Date getActivePeriodEnd() {
    if(delivery.getActivePeriodEnd()==null) return null;
    return delivery.getActivePeriodEnd().getTimeDate();
  }

  public Date getActivePeriodStart() {
    if(delivery.getActivePeriodStart()==null) return null;
    return delivery.getActivePeriodStart().getTimeDate();
  }

  public void setValidityPeriod(Date period) throws AdminException {
    if (period == null)
      delivery.setValidityPeriod(null);
    else
      delivery.setValidityPeriod(new Time(period));
  }

  public Date getValidityPeriod() {
    if (delivery.getValidityPeriod() == null)
      return null;
    return delivery.getValidityPeriod().getTimeDate();
  }

  public String getRetryOnFail() {
    return retryOnFail;
  }

  public void setRetryOnFail(String retryOnFail) {
    this.retryOnFail = retryOnFail;
  }

  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  public void cancel() {
    try {
      fact.shutdown();
    } catch (AdminException e) {
      addError(e);
    }
  }


  public String sendTest() {
    if (!singleText) {
      return null;
    }
    try {
      User u = config.getUser(getUserName());
      TestSms sms = new TestSms();
      sms.setDestAddr(new Address(u.getPhone()));
      sms.setFlash(delivery.isFlash());
      switch (delivery.getDeliveryMode()) {
        case USSD_PUSH:
          sms.setMode(TestSms.Mode.USSD_PUSH);
          break;
        case SMS:
          sms.setMode(TestSms.Mode.SMS);
          break;
        case USSD_PUSH_VLR:
          sms.setMode(TestSms.Mode.USSD_PUSH_VLR);
          break;
      }
      sms.setSourceAddr(delivery.getSourceAddress());
      sms.setText(delivery.getSingleText());
      config.sendTestSms(sms);
      addLocalizedMessage(FacesMessage.SEVERITY_INFO, "delivery.test.sms", u.getPhone());
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public DeliveryPrototype getDelivery() {
    return delivery;
  }
}