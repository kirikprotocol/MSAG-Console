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


  public void setSmsTemplate(String smsTemplate) {
    notificationTemplates.put("delivery.state.changed.sms.template",smsTemplate);
  }

  public String getSmsTemplate() {
    return (String) notificationTemplates.get("delivery.state.changed.sms.template");
  }


  public void setEmailBodyTemplate(String smsTemplate) {
    notificationTemplates.put("delivery.state.changed.email.template.body",smsTemplate);
  }

  public String getEmailBodyTemplate() {
    return (String) notificationTemplates.get("delivery.state.changed.email.template.body");
  }

  public void setEmailSubjectTemplate(String smsTemplate) {
    notificationTemplates.put("delivery.state.changed.email.template.subject",smsTemplate);
  }

  public String getEmailSubjectTemplate() {
    return (String) notificationTemplates.get("delivery.state.changed.email.template.subject");
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
