package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryMode;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;

import javax.faces.application.FacesMessage;
import java.util.Date;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
@SuppressWarnings({"unchecked"})
public class DeliveryEditGroupController extends DeliveryController {

  private List<String> ids;

  private int priority;
  private boolean editPriority;

  private boolean transactionMode;
  private boolean editTransactionMode;


  private Date endDate;
  private boolean editEndDate;

  private Date activePeriodEnd;
  private boolean editActivePeriodEnd;

  private Date activePeriodStart;
  private boolean editActivePeriodStart;

  private Day[] activeWeekDays;
  private boolean editActiveWeekDays;

  private Time validityPeriod;
  private boolean editValidityPeriod;

  private boolean flash;
  private boolean editFlash;

  private boolean useDataSm;
  private boolean editUseDataSm;

  private DeliveryMode deliveryMode;
  private boolean editDeliveryMode;

  private String retryOnFail;

  private String retryPolicy;
  private boolean editRetryPolicy;

  private Address smsNotification;
  private boolean editSmsNotification;

  private String emailNotification;
  private boolean editEmailNotification;

  private Address sourceAddress;
  private boolean editSourceAddress;

  private boolean editMessageTimeToLive;
  private Time messageTimeToLive;

  private boolean editArchiveTime;
  private String archiveTime;


  public DeliveryEditGroupController() {
    super();
    Object o = getRequest().get(DELIVERY_IDS_PARAM);
    if (o != null) {
      ids = (List<String>) o;
    }
  }

  public String save() {

    boolean hasErrors = false;
    for (String i : ids) {
      try {
        Delivery d = config.getDelivery(user.getLogin(), Integer.parseInt(i));
        if (d == null) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN, "informer.deliveries.delivery.not.found", i);
        } else {
          if (editSourceAddress) {
            d.setSourceAddress(sourceAddress);
          }
          if (editPriority) {
            d.setPriority(priority);
          }
          if (editEndDate) {
            d.setEndDate(endDate);
          }
          if (editActivePeriodStart) {
            if (activePeriodStart != null) {
              d.setActivePeriodStart(new Time(activePeriodStart));
            } else {
              d.setActivePeriodStart(null);
            }
          }
          if (editActivePeriodEnd) {
            if (activePeriodEnd != null) {
              d.setActivePeriodEnd(new Time(activePeriodEnd));
            } else {
              d.setActivePeriodEnd(null);
            }
          }
          if (editActiveWeekDays) {
            d.setActiveWeekDays(activeWeekDays);
          }

          if (editUseDataSm) {
            d.setUseDataSm(useDataSm);
          }

          if (editDeliveryMode) {
            d.setDeliveryMode(deliveryMode);
          }

          if (editFlash) {
            d.setFlash(flash);
          }

          if (editTransactionMode) {
            d.setTransactionMode(transactionMode);
          }

          if (editValidityPeriod) {
            if (validityPeriod != null) {
              d.setValidityPeriod(new Time(validityPeriod));
            } else {
              d.setValidityPeriod(null);
            }
          }

          if (editRetryPolicy) {
            if (retryOnFail.equals("off")) {
              d.setRetryPolicy(null);
              d.setRetryOnFail(false);
            } else if (retryOnFail.equals("default")) {
              d.setRetryPolicy("");
              d.setRetryOnFail(true);
            } else {
              d.setRetryOnFail(true);
              d.setRetryPolicy(retryPolicy);
            }
          }
          if (editSmsNotification) {
            if (smsNotification != null) {
              d.setProperty(UserDataConsts.SMS_NOTIF_ADDRESS, smsNotification.getSimpleAddress());
            } else {
              d.removeProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
            }
          }
          if (editEmailNotification) {
            if (emailNotification != null && (emailNotification = emailNotification.trim()).length() != 0) {
              d.setProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS, emailNotification);
            } else {
              d.removeProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
            }
          }
          if (editArchiveTime) {
            if (archiveTime != null && (archiveTime = archiveTime.trim()).length() != 0) {
              d.setArchiveTime(Integer.parseInt(archiveTime));
            } else {
              d.setArchiveTime(null);
            }
          }
          if (editMessageTimeToLive)
            d.setMessageTimeToLive(messageTimeToLive);

          config.modifyDelivery(user.getLogin(), d);
        }
      } catch (AdminException e) {
        addError(new DeliveryControllerException("delivery.edit.error", e, i));
        hasErrors=true;
      }
    }
    return hasErrors ? null :  "DELIVERIES";
  }

  public boolean isEditArchiveTime() {
    return editArchiveTime;
  }

  public void setEditArchiveTime(boolean editArchiveTime) {
    this.editArchiveTime = editArchiveTime;
  }

  public String getArchiveTime() {
    return archiveTime;
  }

  public void setArchiveTime(String archiveTime) {
    this.archiveTime = archiveTime;
  }

  public boolean isArchiveDaemonDeployed() {
    return config.isArchiveDaemonDeployed();
  }

  public boolean isSmsDeliveryMode() {
    return deliveryMode == DeliveryMode.SMS;
  }

  public boolean isEditPriority() {
    return editPriority;
  }

  public void setEditPriority(boolean editPriority) {
    this.editPriority = editPriority;
  }

  public boolean isEditTransactionMode() {
    return editTransactionMode;
  }

  public void setEditTransactionMode(boolean editTransactionMode) {
    this.editTransactionMode = editTransactionMode;
  }

  public boolean isEditEndDate() {
    return editEndDate;
  }

  public void setEditEndDate(boolean editEndDate) {
    this.editEndDate = editEndDate;
  }

  public boolean isEditActivePeriodEnd() {
    return editActivePeriodEnd;
  }

  public void setEditActivePeriodEnd(boolean editActivePeriodEnd) {
    this.editActivePeriodEnd = editActivePeriodEnd;
  }

  public boolean isEditActivePeriodStart() {
    return editActivePeriodStart;
  }

  public void setEditActivePeriodStart(boolean editActivePeriodStart) {
    this.editActivePeriodStart = editActivePeriodStart;
  }

  public boolean isEditActiveWeekDays() {
    return editActiveWeekDays;
  }

  public void setEditActiveWeekDays(boolean editActiveWeekDays) {
    this.editActiveWeekDays = editActiveWeekDays;
  }

  public boolean isEditValidityPeriod() {
    return editValidityPeriod;
  }

  public void setEditValidityPeriod(boolean editValidityPeriod) {
    this.editValidityPeriod = editValidityPeriod;
  }

  public boolean isEditFlash() {
    return editFlash;
  }

  public void setEditFlash(boolean editFlash) {
    this.editFlash = editFlash;
  }

  public boolean isEditDeliveryMode() {
    return editDeliveryMode;
  }

  public void setEditDeliveryMode(boolean editDeliveryMode) {
    this.editDeliveryMode = editDeliveryMode;
  }

  public boolean isEditRetryPolicy() {
    return editRetryPolicy;
  }

  public void setEditRetryPolicy(boolean editRetryPolicy) {
    this.editRetryPolicy = editRetryPolicy;
  }

  public boolean isEditSmsNotification() {
    return editSmsNotification;
  }

  public void setEditSmsNotification(boolean editSmsNotification) {
    this.editSmsNotification = editSmsNotification;
  }

  public boolean isEditEmailNotification() {
    return editEmailNotification;
  }

  public void setEditEmailNotification(boolean editEmailNotification) {
    this.editEmailNotification = editEmailNotification;
  }

  public boolean isEditSourceAddress() {
    return editSourceAddress;
  }

  public void setEditSourceAddress(boolean editSourceAddress) {
    this.editSourceAddress = editSourceAddress;
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

  public Day[] getActiveWeekDays() {
    return activeWeekDays;
  }

  public void setActiveWeekDays(Day[] activeWeekDays) {
    this.activeWeekDays = activeWeekDays;
  }

  public Time getValidityPeriod() {
    return validityPeriod;
  }

  public void setValidityPeriod(Time validityPeriod) {
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

  public String getRetryOnFail() {
    return retryOnFail;
  }

  public void setRetryOnFail(String retryOnFail) {
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

  public boolean isUseDataSm() {
    return useDataSm;
  }

  public void setUseDataSm(boolean useDataSm) {
    this.useDataSm = useDataSm;
  }

  public boolean isEditUseDataSm() {
    return editUseDataSm;
  }

  public void setEditUseDataSm(boolean editUseDataSm) {
    this.editUseDataSm = editUseDataSm;
  }

  public boolean isEditMessageTimeToLive() {
    return editMessageTimeToLive;
  }

  public void setEditMessageTimeToLive(boolean editMessageTimeToLive) {
    this.editMessageTimeToLive = editMessageTimeToLive;
  }

  public Time getMessageTimeToLive() {
    return messageTimeToLive;
  }

  public void setMessageTimeToLive(Time messageTimeToLive) {
    this.messageTimeToLive = messageTimeToLive;
  }

  public List<String> getIds() {
    return ids;
  }

  public void setIds(List<String> ids) {
    this.ids = ids;
  }
}
