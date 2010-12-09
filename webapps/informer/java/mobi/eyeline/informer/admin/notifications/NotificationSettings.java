package mobi.eyeline.informer.admin.notifications;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;

import java.util.Properties;
import java.util.regex.Pattern;

/**
 * @author Aleksandr Khalitov
 */
public class NotificationSettings {

  private final ValidationHelper vh = new ValidationHelper(NotificationSettings.class);

  private Properties javaMailProperties;

  private Properties notificationTemplates;

  private Address smsSenderAddress;   
  
  private static final String SMS_TEMPLATE_ACTIVATED = "delivery.state.activated.sms.template";
  private static final String SMS_TEMPLATE_FINISHED = "delivery.state.finished.sms.template";
  private static final String EMAIL_TEMPLATE_SUBJECT = "delivery.state.changed.email.template.subject";
  private static final String EMAIL_TEMPLATE_ACTIVATED = "delivery.state.activated.email.template.line";
  private static final String EMAIL_TEMPLATE_FINISHED = "delivery.state.finished.email.template.line";

  public NotificationSettings(Properties javaMailProperties, Properties notificationTemplates, Address smsSenderAddress) {
    this.javaMailProperties = javaMailProperties;
    this.notificationTemplates = notificationTemplates;
    this.smsSenderAddress = smsSenderAddress;
  }

  public NotificationSettings(NotificationSettings s) {
    this(s.getMailProperties(), s.getNotificationTemplates(), s.getSmsSenderAddress());
  }


  private static final Pattern emailPattern =
      Pattern.compile("^[A-Za-z0-9]+[\\.\\-_A-Za-z0-9!#$&'*+/=?^_`{|}~:]*@[A-Za-z0-9]+[\\.\\-_A-Za-z0-9!#$&'*+/=?^_`{|}~:]*$");

  public void validate() throws AdminException {
    vh.checkNotNull("smsSenderAddr", smsSenderAddress);
    vh.checkNotEmpty("activatedSmsTemplate", getSmsTemplateActivated());
    vh.checkNotEmpty("finishedSmsTemplate", getSmsTemplateFinished());
    vh.checkNotEmpty("changedEmailTemplateSubject", getEmailSubjectTemplate());
    vh.checkNotEmpty("activatedEmailTemplateBody", getEmailTemplateActivated());
    vh.checkNotEmpty("finishedEmailTemplateBody", getEmailTemplateFinished());
    vh.checkNotEmpty("mailHost", getMailHost());
    vh.checkMatches("mailFrom", getMailFrom(), emailPattern);

  }

  public Properties getMailProperties() {
    Properties p = new Properties();
    p.putAll(javaMailProperties);
    return p;
  }

  public Properties getNotificationTemplates() {
    Properties p = new Properties();
    p.putAll(notificationTemplates);
    return p;
  }

  public Address getSmsSenderAddress() {
    return smsSenderAddress==null ? null : new Address(smsSenderAddress);
  }

  public void setSmsSenderAddress(Address sAddr) {
    smsSenderAddress = sAddr;
  }


  public void setSmsTemplateActivated(String smsTemplate) {
    if(smsTemplate != null) {
      notificationTemplates.setProperty(SMS_TEMPLATE_ACTIVATED, smsTemplate);
    }else {
      notificationTemplates.remove(SMS_TEMPLATE_ACTIVATED);
    }
  }

  public void setSmsTemplateFinished(String smsTemplate) {
    if(smsTemplate != null) {
      notificationTemplates.setProperty(SMS_TEMPLATE_FINISHED, smsTemplate);
    }else {
      notificationTemplates.remove(SMS_TEMPLATE_FINISHED);
    }
  }

  public String getSmsTemplateActivated() {
    return notificationTemplates.getProperty(SMS_TEMPLATE_ACTIVATED);
  }

  public String getSmsTemplateFinished() {
    return notificationTemplates.getProperty(SMS_TEMPLATE_FINISHED);
  }


  public void setEmailTemplateActivated(String smsTemplate) {
    if(smsTemplate != null) {
      notificationTemplates.setProperty(EMAIL_TEMPLATE_ACTIVATED, smsTemplate);
    }else {
      notificationTemplates.remove(EMAIL_TEMPLATE_ACTIVATED);
    }
  }

  public void setEmailTemplateFinished(String smsTemplate) {
    if(smsTemplate != null) {
      notificationTemplates.setProperty(EMAIL_TEMPLATE_FINISHED, smsTemplate);
    }else {
      notificationTemplates.remove(EMAIL_TEMPLATE_FINISHED);
    }
  }

  public String getEmailTemplateActivated() {
    return notificationTemplates.getProperty(EMAIL_TEMPLATE_ACTIVATED);
  }

  public String getEmailTemplateFinished() {
    return notificationTemplates.getProperty(EMAIL_TEMPLATE_FINISHED);
  }

  public void setEmailSubjectTemplate(String smsTemplate) {
    if(smsTemplate != null) {
      notificationTemplates.setProperty(EMAIL_TEMPLATE_SUBJECT, smsTemplate);
    }else {
      notificationTemplates.remove(EMAIL_TEMPLATE_SUBJECT);
    }
  }

  public String getEmailSubjectTemplate() {
    return notificationTemplates.getProperty(EMAIL_TEMPLATE_SUBJECT);
  }

  public String getMailFrom() {
    return javaMailProperties.getProperty("mail.from");
  }

  public void setMailFrom(String from) {
    if(from != null) {
      javaMailProperties.setProperty("mail.from", from);
    }else {
      javaMailProperties.remove("mail.from");
    }
  }

  public void setMailHost(String mailHost) {
    if(mailHost != null) {
      javaMailProperties.setProperty("mail.host", mailHost);
    }else {
      javaMailProperties.remove("mail.host");
    }
  }

  public String getMailHost() {
    return javaMailProperties.getProperty("mail.host");
  }

  public void setMailUser(String mailUser) {
    if(mailUser != null) {
      javaMailProperties.setProperty("mail.user", mailUser);
    }else {
      javaMailProperties.remove("mail.user");
    }
  }

  public String getMailUser() {
    return javaMailProperties.getProperty("mail.user");
  }

  public void setMailPassword(String mailPassword) {
    if(mailPassword != null) {
      javaMailProperties.setProperty("mail.password", mailPassword);
    }else {
      javaMailProperties.remove("mail.password");
    }
  }

  public String getMailPassword() {
    return javaMailProperties.getProperty("mail.password");
  }

}
