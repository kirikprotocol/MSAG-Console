package mobi.eyeline.informer.web.controllers.notifications;


import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.notifications.DeliveryNotificationTemplatesConstants;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import java.util.Properties;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 13:39:50
 */
public class NotificationSettingsController extends InformerController {

  private final Properties javaMailProps;
  private final Properties notificationTemplates;
  private Address smsSenderAddress;
  private String testEmail;
  private Address testAddress;


  public NotificationSettingsController() {
    super();
    javaMailProps = getConfig().getJavaMailProperties();
    notificationTemplates = getConfig().getNotificationTemplates();
    smsSenderAddress = getConfig().getSmsSenderAddress();
  }


  public String save() {
    try {
      getConfig().updateJavaMailProperties(javaMailProps, getUserName());
      getConfig().updateNotificationTemplates(notificationTemplates, getUserName());
      getConfig().setSmsSenderAddress(smsSenderAddress, getUserName());
    }
    catch (AdminException e) {
      addError(e);
      return null;
    }
    return "INDEX";
  }

  public String getSmsSenderAddress() {
    return smsSenderAddress.getSimpleAddress();
  }

  public void setSmsSenderAddress(String sAddr) {
    smsSenderAddress = new Address(sAddr);
  }


  public void setSmsTemplateActivated(String smsTemplate) {
    notificationTemplates.put(DeliveryNotificationTemplatesConstants.SMS_TEMPLATE_ACTIVATED, smsTemplate);
  }

  public void setSmsTemplateFinished(String smsTemplate) {
    notificationTemplates.put(DeliveryNotificationTemplatesConstants.SMS_TEMPLATE_FINISHED, smsTemplate);
  }

  public String getSmsTemplateActivated() {
    return (String) notificationTemplates.get(DeliveryNotificationTemplatesConstants.SMS_TEMPLATE_ACTIVATED);
  }

  public String getSmsTemplateFinished() {
    return (String) notificationTemplates.get(DeliveryNotificationTemplatesConstants.SMS_TEMPLATE_FINISHED);
  }


  public void setEmailTemplateActivated(String smsTemplate) {
    notificationTemplates.put(DeliveryNotificationTemplatesConstants.EMAIL_TEMPLATE_ACTIVATED, smsTemplate);
  }

  public void setEmailTemplateFinished(String smsTemplate) {
    notificationTemplates.put(DeliveryNotificationTemplatesConstants.EMAIL_TEMPLATE_FINISHED, smsTemplate);
  }

  public String getEmailTemplateActivated() {
    return (String) notificationTemplates.get(DeliveryNotificationTemplatesConstants.EMAIL_TEMPLATE_ACTIVATED);
  }

  public String getEmailTemplateFinished() {
    return (String) notificationTemplates.get(DeliveryNotificationTemplatesConstants.EMAIL_TEMPLATE_FINISHED);
  }

  public void setEmailSubjectTemplate(String smsTemplate) {
    notificationTemplates.put(DeliveryNotificationTemplatesConstants.EMAIL_TEMPLATE_SUBJECT, smsTemplate);
  }

  public String getEmailSubjectTemplate() {
    return (String) notificationTemplates.get(DeliveryNotificationTemplatesConstants.EMAIL_TEMPLATE_SUBJECT);
  }

  public String getMailFrom() {
    return (String) javaMailProps.get("mail.from");
  }

  public void setMailFrom(String from) {
    javaMailProps.put("mail.from", from);
  }

  public void setMailHost(String mailHost) {
    javaMailProps.put("mail.host", mailHost);
  }

  public String getMailHost() {
    return (String) javaMailProps.get("mail.host");
  }

  public void setMailUser(String mailUser) {
    javaMailProps.put("mail.user", mailUser);
  }

  public String getMailUser() {
    return (String) javaMailProps.get("mail.user");
  }

  public void setMailPassword(String mailPassword) {
    javaMailProps.put("mail.password", mailPassword);
  }

  public String getMailPassword() {
    return (String) javaMailProps.get("mail.password");
  }


  public String getTestEmailAddr() {
    return testEmail;
  }

  public void setTestEmailAddr(String testEmail) {
    this.testEmail = testEmail;
  }

  public String sendTestEmail() {
    if(testEmail==null) {
      addMessage(FacesMessage.SEVERITY_WARN,"notifications.test.email.required");
      return null;
    }
    try {
      User user = getConfig().getUser(getUserName());
      getConfig().sendTestEmailNotification(user,testEmail,javaMailProps,notificationTemplates);
      addLocalizedMessage(FacesMessage.SEVERITY_INFO,"notifications.test.ok",testEmail);
    }
    catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String getTestSmsAddress() {
    return testAddress==null ? null : testAddress.getSimpleAddress();
  }

  public void setTestSmsAddress(String testAddress) {
    this.testAddress  = (testAddress==null || testAddress.length()==0) ?  null:new Address(testAddress);
  }

  public String sendTestSms() {
    if(testAddress==null) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN,"notifications.test.address.required");
      return null;
    }
    try {
      User user = getConfig().getUser(getUserName());
      getConfig().sendTestSmsNotification(user,testAddress, DeliveryStatus.Active,notificationTemplates);
      getConfig().sendTestSmsNotification(user,testAddress, DeliveryStatus.Finished,notificationTemplates);
      addLocalizedMessage(FacesMessage.SEVERITY_INFO,"notifications.test.ok",testAddress.getSimpleAddress());
    }
    catch (AdminException e) {
      addError(e);
    }
    return null;
  }


}
