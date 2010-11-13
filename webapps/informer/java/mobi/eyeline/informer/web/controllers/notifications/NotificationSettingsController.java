package mobi.eyeline.informer.web.controllers.notifications;


import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.controllers.InformerController;


import java.util.Properties;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 13:39:50
 */
public class NotificationSettingsController extends InformerController {

  private Properties javaMailProps;
  private Properties notificationTemplates;
  private Address smsSenderAddress;

  private static final String SMS_TEMPLATE_ACTIVATED = "delivery.state.activated.sms.template";
  private static final String SMS_TEMPLATE_FINISHED  = "delivery.state.finished.sms.template";
  private static final String EMAIL_TEMPLATE_SUBJECT ="delivery.state.changed.email.template.subject";
  private static final String EMAIL_TEMPLATE_ACTIVATED="delivery.state.activated.email.template.line";
  private static final String EMAIL_TEMPLATE_FINISHED="delivery.state.finished.email.template.line";



  public NotificationSettingsController() {
    super();
    javaMailProps = getConfig().getJavaMailProperties();
    notificationTemplates = getConfig().getNotificationTemplates();
    smsSenderAddress = getConfig().getSmsSenderAddress();
  }



  public String save() {
    try {
      getConfig().updateJavaMailProperties(javaMailProps,getUserName());
      getConfig().updateNotificationTemplates(notificationTemplates,getUserName());
      getConfig().setSmsSenderAddress(smsSenderAddress,getUserName());
    }
    catch (AdminException e) {
      addError(e);
      return null;
    }
    return "INDEX";
  }

  public String reset() {
    return null;
  }

  public String getSmsSenderAddress() {
    return smsSenderAddress.getSimpleAddress();
  }
  public void setSmsSenderAddress(String sAddr) {
    smsSenderAddress=new Address(sAddr);
  }


  public void setSmsTemplateActivated(String smsTemplate) {
    notificationTemplates.put(SMS_TEMPLATE_ACTIVATED,smsTemplate);
  }
  public void setSmsTemplateFinished(String smsTemplate) {
    notificationTemplates.put(SMS_TEMPLATE_FINISHED,smsTemplate);
  }

  public String getSmsTemplateActivated() {
    return (String) notificationTemplates.get(SMS_TEMPLATE_ACTIVATED);
  }
  public String getSmsTemplateFinished() {
    return (String) notificationTemplates.get(SMS_TEMPLATE_FINISHED);
  }


  public void setEmailTemplateActivated(String smsTemplate) {
    notificationTemplates.put(EMAIL_TEMPLATE_ACTIVATED,smsTemplate);
  }
  public void setEmailTemplateFinished(String smsTemplate) {
    notificationTemplates.put(EMAIL_TEMPLATE_FINISHED,smsTemplate);
  }

  public String getEmailTemplateActivated() {
    return (String) notificationTemplates.get(EMAIL_TEMPLATE_ACTIVATED);
  }
  public String getEmailTemplateFinished() {
    return (String) notificationTemplates.get(EMAIL_TEMPLATE_FINISHED);
  }

  public void setEmailSubjectTemplate(String smsTemplate) {
    notificationTemplates.put(EMAIL_TEMPLATE_SUBJECT,smsTemplate);
  }

  public String getEmailSubjectTemplate() {
    return (String) notificationTemplates.get(EMAIL_TEMPLATE_SUBJECT);
  }

  public String getMailFrom() {
    return (String) javaMailProps.get("mail.from");
  }
  public void setMailFrom(String from) {
    javaMailProps.put("mail.from",from);
  }

  public void setMailHost(String mailHost) {
    javaMailProps.put("mail.host",mailHost);
  }

  public String getMailHost() {
    return (String) javaMailProps.get("mail.host");
  }

  public void setMailUser(String mailUser) {
    javaMailProps.put("mail.user",mailUser);
  }

  public String getMailUser() {
    return (String) javaMailProps.get("mail.user");
  }

  public void setMailPassword(String mailPassword) {
    javaMailProps.put("mail.password",mailPassword);
  }

  public String getMailPassword() {
    return (String) javaMailProps.get("mail.password");
  }
}
