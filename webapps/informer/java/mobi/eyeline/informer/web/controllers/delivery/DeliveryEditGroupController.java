package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryMode;
import mobi.eyeline.informer.util.Address;

import javax.faces.application.FacesMessage;
import java.util.Date;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
@SuppressWarnings({"unchecked"})
public class DeliveryEditGroupController extends DeliveryController{

  private List<Integer> ids;

  private int priority;
  private boolean transactionMode;


  private Date endDate;
  private Date activePeriodEnd;
  private Date activePeriodStart;
  private Delivery.Day[] activeWeekDays;
  private String validityPeriod;

  private boolean flash;
  private DeliveryMode deliveryMode;

  private boolean retryOnFail;
  private String retryPolicy;

  private Address smsNotification;

  private String emailNotification;

  private Address sourceAddress;


  public DeliveryEditGroupController() {
    super();
    Object o = getRequest().get(DELIVERY_IDS_PARAM);
    if(o != null) {
      ids =  (List<Integer>)o;
    }
  }

  public String save() {
    try{
      for(Integer i : ids) {
        Delivery d = config.getDelivery(user.getLogin(), user.getPassword(), i);
        if(d == null) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN, "informer.deliveries.delivery.not.found", i);
        } else {
          d.setPriority(priority);
          d.setTransactionMode(transactionMode);
          d.setEndDate(endDate);
          d.setActivePeriodStart(activePeriodStart);
          d.setActivePeriodEnd(activePeriodEnd);
          d.setActiveWeekDays(activeWeekDays);
          if(validityPeriod == null || (validityPeriod = validityPeriod.trim()).length() !=0) {
            d.setValidityPeriod(validityPeriod);
          }else {
            d.setValidityPeriod(null);
          }
          d.setFlash(flash);
          if(deliveryMode != null) {
            d.setDeliveryMode(deliveryMode);
          }
          d.setRetryOnFail(retryOnFail);
          d.setRetryPolicy(retryPolicy);
          if(smsNotification != null) {
            d.setProperty(UserDataConsts.SMS_NOTIF_ADDRESS, smsNotification.getSimpleAddress());
          }else{
            d.removeProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
          }
          if(emailNotification != null && (emailNotification = emailNotification.trim()).length() != 0) {
            d.setProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS, emailNotification);
          }else {
            d.removeProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
          }
          config.modifyDelivery(user.getLogin(), user.getPassword(), d);
        }
      }
      return "DELIVERIES";
    }catch (AdminException e){
      addError(e);
    }
    return null;
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) {
    this.priority = priority;
  }

  public boolean isTransactionMode() {
    return transactionMode;
  }

  public void setTransactionMode(boolean transactionMode) {
    this.transactionMode = transactionMode;
  }

  public Date getEndDate() {
    return endDate;
  }

  public void setEndDate(Date endDate) {
    this.endDate = endDate;
  }

  public Date getActivePeriodEnd() {
    return activePeriodEnd;
  }

  public void setActivePeriodEnd(Date activePeriodEnd) {
    this.activePeriodEnd = activePeriodEnd;
  }

  public Date getActivePeriodStart() {
    return activePeriodStart;
  }

  public void setActivePeriodStart(Date activePeriodStart) {
    this.activePeriodStart = activePeriodStart;
  }

  public Delivery.Day[] getActiveWeekDays() {
    return activeWeekDays;
  }

  public void setActiveWeekDays(Delivery.Day[] activeWeekDays) {
    this.activeWeekDays = activeWeekDays;
  }

  public String getValidityPeriod() {
    return validityPeriod;
  }

  public void setValidityPeriod(String validityPeriod) {
    this.validityPeriod = validityPeriod;
  }

  public boolean isFlash() {
    return flash;
  }

  public void setFlash(boolean flash) {
    this.flash = flash;
  }

  public DeliveryMode getDeliveryMode() {
    return deliveryMode;
  }

  public void setDeliveryMode(DeliveryMode deliveryMode) {
    this.deliveryMode = deliveryMode;
  }

  public boolean isRetryOnFail() {
    return retryOnFail;
  }

  public void setRetryOnFail(boolean retryOnFail) {
    this.retryOnFail = retryOnFail;
  }

  public String getRetryPolicy() {
    return retryPolicy;
  }

  public void setRetryPolicy(String retryPolicy) {
    this.retryPolicy = retryPolicy;
  }

  public Address getSmsNotification() {
    return smsNotification;
  }

  public void setSmsNotification(Address smsNotification) {
    this.smsNotification = smsNotification;
  }

  public String getEmailNotification() {
    return emailNotification;
  }

  public void setEmailNotification(String emailNotification) {
    this.emailNotification = emailNotification;
  }

  public Address getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(Address sourceAddress) {
    this.sourceAddress = sourceAddress;
  }

  public List<Integer> getIds() {
    return ids;
  }

  public void setIds(List<Integer> ids) {
    this.ids = ids;
  }
}
