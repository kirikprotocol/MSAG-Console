package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryMode;
import mobi.eyeline.informer.admin.delivery.DeliveryStatistics;
import mobi.eyeline.informer.admin.delivery.DeliveryStatusHistory;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.infosme.TestSmsException;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.WebContext;
import mobi.eyeline.informer.web.config.InformerTimezone;
import mobi.eyeline.informer.web.controllers.stats.ErrorStatsController;
import mobi.eyeline.informer.web.controllers.stats.MessagesByPeriodController;
import mobi.eyeline.informer.web.controllers.users.UserEditController;
import org.apache.log4j.Logger;

import javax.faces.application.FacesMessage;
import javax.faces.event.ActionEvent;
import java.util.Date;
import java.util.MissingResourceException;
import java.util.ResourceBundle;
import java.util.TimeZone;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryEditController extends DeliveryController {


  private static final Logger logger = Logger.getLogger(DeliveryEditController.class);

  private Integer id;

  private Delivery delivery;
  private DeliveryStatistics status;
  private DeliveryStatusHistory statusHistory;

  private String comeBackParam;
  private String retryOnFail;

  private boolean smsNotificationCheck;
  private boolean emailNotificationCheck;

  private boolean archivateCheck;

  private String archiveTime;

  private String testAddress;

  private Date createDate;

  public DeliveryEditController() {
    super();

    String p = getRequestParameter(DELIVERY_PARAM);
    if(p == null || p.length() == 0) {
      p = getRequestParameter(ErrorStatsController.COME_BACK_PARAMS);
    }
    if(p == null || p.length() == 0) {
      p = getRequestParameter(MessagesByPeriodController.COME_BACK_PARAMS);
    }
    if(p == null || p.length() == 0) {
      p = getRequestParameter(MessageListController.COME_BACK_PARAMS);
    }
    id = p == null || (p = p.trim()).length() == 0 ? null : Integer.parseInt(p);

    if(id == null) {
      p = getRequestParameter(UserEditController.COME_BACK_PARAMS);
      if(p != null) {
        String[] s = p.split("=");
        if(s.length>1) {
          id = Integer.parseInt(s[1]);
        }
      }
    }

    p = getRequestParameter(DELIVERY_COMEBACK_PARAM);
    if (p != null && p.length() != 0) {
      comeBackParam = p;
    }

    try {
      reload();
    } catch (AdminException e) {
      addError(e);
    }
  }

  public boolean isBoundToLocalTIme() {
    return delivery != null && delivery.isBoundToLocalTime();
  }

  public String getServerTimezone() {
    String defId = TimeZone.getDefault().getID();
    InformerTimezone timezone = WebContext.getInstance().getWebTimezones().getTimezoneByID(defId) ;
    return timezone == null ? null : timezone.getAlias(getLocale());
  }

  public String getTestAddress() {
    return testAddress;
  }

  public void setTestAddress(String testAddress) {
    this.testAddress = testAddress;
  }

  public boolean isArchiveDaemonDeployed() {
    return config.isArchiveDaemonDeployed();
  }

  public String getArchiveTime() {
    return archiveTime;
  }

  public void setArchiveTime(String archiveTime) {
    this.archiveTime = archiveTime;
  }

  public String getComeBackParam() {
    return comeBackParam;
  }

  public void setComeBackParam(String comeBackParam) {
    this.comeBackParam = comeBackParam;
  }

  public String getUserComeBackParams() {
    return "id="+delivery.getId();
  }

  private void reload() throws AdminException {
    if (id == null || (delivery = config.getDelivery(user.getLogin(), id)) == null) {
      logger.error("Delivery is not found with id=" + id);
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "delivery.not.found", id);
      return;
    }
    String p = delivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
    if (p != null) {
      smsNotificationAddress = new Address(p);
    }
    emailNotificationAddress = delivery.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
    smsNotificationCheck = smsNotificationAddress != null;
    emailNotificationCheck = emailNotificationAddress != null;
    secret = Boolean.valueOf(delivery.getProperty(UserDataConsts.SECRET));
    flashSecret = Boolean.valueOf(delivery.getProperty(UserDataConsts.SECRET_FLASH));
    secretMessage = delivery.getProperty(UserDataConsts.SECRET_TEXT);

    if (delivery.isRetryOnFail()) {
      retryOnFail = (delivery.getRetryPolicy() == null || delivery.getRetryPolicy().length() == 0) ? "default" : "custom";
    } else
      retryOnFail = "off";

    status = getConfig().getDeliveryStats(user.getLogin(), id);
    statusHistory = getConfig().getDeliveryStatusHistory(user.getLogin(), id);

    User u = getConfig().getUser(delivery.getOwner());
    archiveTime = delivery.getArchiveTime() == null ? null : Integer.toString(delivery.getArchiveTime());
    archivateCheck = delivery.getArchiveTime() != null;
    if(u.getPhone() != null) {
      testAddress = u.getPhone().getSimpleAddress();
    }
    createDate = delivery.getCreateDate();
  }


  public Date getCreateDate() {
    return createDate;
  }

  public void setCreateDate(Date createDate) {
    this.createDate = createDate;
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

    if (retryOnFail.equals("off")) {
      delivery.setRetryOnFail(false);
      delivery.setRetryPolicy(null);
    } else if (retryOnFail.equals("default")) {
      delivery.setRetryOnFail(true);
      delivery.setRetryPolicy("");
    } else {
      delivery.setRetryOnFail(true);
    }


    if (emailNotificationCheck && emailNotificationAddress != null && (emailNotificationAddress = emailNotificationAddress.trim()).length() != 0) {
      delivery.setProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS, emailNotificationAddress);
      delivery.setEnableStateChangeLogging(true);
    } else {
      delivery.removeProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
    }
    if (smsNotificationCheck && smsNotificationAddress != null) {
      delivery.setProperty(UserDataConsts.SMS_NOTIF_ADDRESS, smsNotificationAddress.getSimpleAddress());
      delivery.setEnableStateChangeLogging(true);
    } else {
      delivery.removeProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
    }

    if(archivateCheck && archiveTime != null && archiveTime.length()>0) {
      delivery.setArchiveTime(Integer.parseInt(archiveTime));
    }else {
      delivery.setArchiveTime(null);
    }

    try {
      config.modifyDelivery(user.getLogin(), delivery);
    } catch (AdminException e) {
      addError(e);
      return null;
    }
    return comeBackParam != null && comeBackParam.length() != 0 ? comeBackParam : "DELIVERIES";
  }

  public String cancel() {
    String p = comeBackParam != null && comeBackParam.length() > 0 ? comeBackParam : getRequestParameter(DELIVERY_COMEBACK_PARAM);
    return p != null && p.length() > 0 ? p : "DELIVERIES";

  }

  public String sendTest() {
    if (delivery.getType() != Delivery.Type.SingleText) {
      return null;
    }
    if(testAddress == null || !Address.validate(testAddress)) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.test.sms.address.illegal", testAddress);
      return null;
    }
    try {
      Address src = delivery.getSourceAddress();
      Address dst = new Address(testAddress);
      boolean flash = delivery.isFlash();
      String text = delivery.getSingleText();

      TestSms sms;
      switch (delivery.getDeliveryMode()) {
        case USSD_PUSH:
          sms = TestSms.ussdPush(src, dst, text);
          break;
        case SMS:
          sms = TestSms.sms(src, dst, text, flash);
          break;
        default:
          sms = TestSms.ussdPushViaVlr(src, dst, text);
      }
      config.sendTestSms(sms);
      addLocalizedMessage(FacesMessage.SEVERITY_INFO, "delivery.test.sms", testAddress);
    } catch (TestSmsException e) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.test.sms.error", testAddress,
          Integer.toString(e.getSmppStatus()), getSmppCodeDescription(e.getSmppStatus()));
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }


  private String getSmppCodeDescription(int smppCode) {
    ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.admin.SmppStatus", getLocale());
    try {
      return bundle.getString("informer.errcode." + smppCode);
    } catch (MissingResourceException e) {
      return bundle.getString("informer.errcode.unknown");
    }
  }

  public boolean isSmsMode() {
    return delivery.getDeliveryMode() == DeliveryMode.SMS;
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



  public boolean isSmsNotificationCheck() {
    return smsNotificationCheck;
  }

  public void setSmsNotificationCheck(boolean smsNotificationCheck) {
    this.smsNotificationCheck = smsNotificationCheck;
  }

  public boolean isArchivateCheck() {
    return archivateCheck;
  }

  public void setArchivateCheck(boolean archivateCheck) {
    this.archivateCheck = archivateCheck;
  }

  public boolean isEmailNotificationCheck() {
    return emailNotificationCheck;
  }

  public void setEmailNotificationCheck(boolean emailNotificationCheck) {
    this.emailNotificationCheck = emailNotificationCheck;
  }

  public String getRetryOnFail() {
    return retryOnFail;
  }

  public void setRetryOnFail(String retryOnFail) {
    this.retryOnFail = retryOnFail;
  }

  public DeliveryStatistics getStatus() {
    return status;
  }

  public DeliveryStatusHistory getStatusHistory() {
    return statusHistory;
  }

  public boolean isUseDataSm() {
    return delivery.isUseDataSm();
  }

  public void setUseDataSm(boolean useDataSm) {
    delivery.setUseDataSm(useDataSm);
  }

  public void statForSelected(ActionEvent e) {
    getRequest().put("delivery", delivery.getId());
  }

  public String getMessagesComeBackName() {
    return MessageListController.COME_BACK;
  }

  public String getMessagesComeBackParamsName() {
    return MessageListController.COME_BACK_PARAMS;
  }
}
