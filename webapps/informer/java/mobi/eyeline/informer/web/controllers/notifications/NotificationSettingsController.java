package mobi.eyeline.informer.web.controllers.notifications;


import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.notifications.NotificationSettings;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import java.util.regex.Pattern;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 13:39:50
 */
public class NotificationSettingsController extends InformerController {

  private String testEmail;
  private Address testAddress;

  private Address smsSenderAddress;

  public String smsTemplateActivated;

  public String smsTemplateFinished;

  public String emailTemplateActivated;

  public String emailTemplateFinished;

  public String emailSubjectTemplate;

  public String mailFrom;

  public String mailHost;

  public String mailUser;

  public String mailPassword;

  private static final Pattern emailPattern =
      Pattern.compile("^[A-Za-z0-9]+[\\.\\-_A-Za-z0-9!#$&'*+/=?^_`{|}~:]*@[A-Za-z0-9]+[\\.\\-_A-Za-z0-9!#$&'*+/=?^_`{|}~:]*$");


  public NotificationSettingsController() {

    super();

    NotificationSettings settings = getConfig().getNotificationSettings();

    smsSenderAddress = settings.getSmsSenderAddress();

    smsTemplateActivated = settings.getSmsTemplateActivated();

    smsTemplateFinished = settings.getSmsTemplateFinished();

    emailTemplateActivated = settings.getEmailTemplateActivated();

    emailTemplateFinished = settings.getEmailTemplateFinished();

    emailSubjectTemplate = settings.getEmailSubjectTemplate();

    mailFrom = settings.getMailFrom();

    mailHost = settings.getMailHost();

    mailUser = settings.getMailUser();

    mailPassword = settings.getMailPassword();

  }

  public String save() {
    try {
      getConfig().updateNotificationSettings(createSettings(), getUserName());
    }
    catch (AdminException e) {
      addError(e);
      return null;
    }
    return "INDEX";
  }


  public String getTestEmailAddr() {
    return testEmail;
  }

  public void setTestEmailAddr(String testEmail) {
    this.testEmail = testEmail;
  }

  private NotificationSettings createSettings() {
    NotificationSettings settings = getConfig().getNotificationSettings();
    settings.setSmsSenderAddress(smsSenderAddress);
    settings.setSmsTemplateActivated(smsTemplateActivated);
    settings.setSmsTemplateFinished(smsTemplateFinished);
    settings.setEmailTemplateActivated(emailTemplateActivated);
    settings.setEmailTemplateFinished(emailTemplateFinished);
    settings.setEmailSubjectTemplate(emailSubjectTemplate);
    settings.setMailFrom(mailFrom);
    settings.setMailHost(mailHost);
    settings.setMailUser(mailUser);
    settings.setMailPassword(mailPassword);
    return settings;

  }

  public String sendTestEmail() {
    if(testEmail==null || !emailPattern.matcher(testEmail).matches()) {
      addMessage(FacesMessage.SEVERITY_WARN,"notifications.test.email.required");
      return null;
    }
    try {
      User user = getConfig().getUser(getUserName());
      getConfig().sendTestEmailNotification(user,testEmail, createSettings());
      addLocalizedMessage(FacesMessage.SEVERITY_INFO,"notifications.test.ok",testEmail);
    }
    catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public Address getTestSmsAddress() {
    return testAddress;
  }

  public void setTestSmsAddress(Address testAddress) {
    this.testAddress  = testAddress;
  }

  public String getTestEmail() {
    return testEmail;
  }

  public void setTestEmail(String testEmail) {
    this.testEmail = testEmail;
  }

  public Address getTestAddress() {
    return testAddress;
  }

  public void setTestAddress(Address testAddress) {
    this.testAddress = testAddress;
  }

  public Address getSmsSenderAddress() {
    return smsSenderAddress;
  }

  public void setSmsSenderAddress(Address smsSenderAddress) {
    this.smsSenderAddress = smsSenderAddress;
  }

  public String getSmsTemplateActivated() {
    return smsTemplateActivated;
  }

  public void setSmsTemplateActivated(String smsTemplateActivated) {
    this.smsTemplateActivated = smsTemplateActivated;
  }

  public String getSmsTemplateFinished() {
    return smsTemplateFinished;
  }

  public void setSmsTemplateFinished(String smsTemplateFinished) {
    this.smsTemplateFinished = smsTemplateFinished;
  }

  public String getEmailTemplateActivated() {
    return emailTemplateActivated;
  }

  public void setEmailTemplateActivated(String emailTemplateActivated) {
    this.emailTemplateActivated = emailTemplateActivated;
  }

  public String getEmailTemplateFinished() {
    return emailTemplateFinished;
  }

  public void setEmailTemplateFinished(String emailTemplateFinished) {
    this.emailTemplateFinished = emailTemplateFinished;
  }

  public String getEmailSubjectTemplate() {
    return emailSubjectTemplate;
  }

  public void setEmailSubjectTemplate(String emailSubjectTemplate) {
    this.emailSubjectTemplate = emailSubjectTemplate;
  }

  public String getMailFrom() {
    return mailFrom;
  }

  public void setMailFrom(String mailFrom) {
    this.mailFrom = mailFrom;
  }

  public String getMailHost() {
    return mailHost;
  }

  public void setMailHost(String mailHost) {
    this.mailHost = mailHost;
  }

  public String getMailUser() {
    return mailUser;
  }

  public void setMailUser(String mailUser) {
    this.mailUser = mailUser;
  }

  public String getMailPassword() {
    return mailPassword;
  }

  public void setMailPassword(String mailPassword) {
    this.mailPassword = mailPassword;
  }

  public String sendTestSms() {
    if(testAddress==null) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN,"notifications.test.address.required");
      return null;
    }
    try {
      User user = getConfig().getUser(getUserName());
      NotificationSettings settings = createSettings();
      getConfig().sendTestSmsNotification(user,testAddress, DeliveryStatus.Active, settings);
      getConfig().sendTestSmsNotification(user,testAddress, DeliveryStatus.Finished, settings);
      addLocalizedMessage(FacesMessage.SEVERITY_INFO,"notifications.test.ok",testAddress.getSimpleAddress());
    }
    catch (AdminException e) {
      addError(e);
    }
    return null;
  }


}
