package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.config.Configuration;

import javax.faces.application.FacesMessage;
import javax.servlet.http.HttpSession;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.List;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 14.10.2010
 * Time: 18:25:48
 */
public class UserGroupEditController extends UserController {
  private List<String> userIds;

  private boolean editOrganization;
  private String organization;

  private boolean editAdmin;
  private boolean admin;

  private boolean editBlocked;
  private boolean blocked;

  private boolean editSourceAddr;
  private Address sourceAddr;

  private boolean editDeliveryDays;
  private List<Integer> deliveryDays = new ArrayList<Integer>();

  private boolean editDeliveryStartTime;
  private Date deliveryStartTime;

  private boolean editDeliveryEndTime;
  private Date deliveryEndTime;

  private boolean editPriority;
  private int priority;

  private boolean editRetryPolicy;
  private String retryOnFail = "off";
  private String policyId;

  private boolean editMessageTimeToLive;
  private Time messageTimeToLive;

  private boolean editUseDataSm;
  private boolean useDataSm;

  private boolean editDeliveryType;
  private User.DeliveryType deliveryType;

  private boolean editTransactionMode;
  private boolean transactionMode;

  private boolean editValidityPeriod;
  private Time validityPeriod;

  private boolean editSmsPerSec;
  private int smsPerSec;

  private boolean editAllowedRegions;
  private boolean allRegionsAllowed;
  private List<Integer> regions = new ArrayList<Integer>();

  private boolean editSmsNotification;
  private boolean smsNotification;

  private boolean editEmailNotification;
  private boolean emailNotification;

  private boolean editArchivateDeliveries;
  private boolean createArchive;
  private int deliveryLifetime;

  private boolean editCreateCdr;
  private boolean createCdr;

  private String cdrDestination;
  private String cdrOriginator;


//Вкл/откл принудительную архивацию рассылок и макс. время жизни рассылки.
//Настройки рассылок по-умолчанию (см. выше).

  public UserGroupEditController() throws AdminException {
    super();
    HttpSession session = getSession(false);
    if (session != null) {
      userIds = (List<String>) session.getAttribute("userIds");
    }
    deliveryStartTime = new Time(9,0,0).getTimeDate();
    deliveryEndTime = new Time(22,0,0).getTimeDate();
    deliveryType=User.DeliveryType.SMS;
    validityPeriod=new Time(1,0,0);
    messageTimeToLive=new Time(2,0,0);
    smsPerSec=10;
    priority=1;
    deliveryLifetime=72;
    deliveryDays=Arrays.asList(0,1,2,3,4,5,6);
    smsPerSec = 10;
    priority = 1;
    deliveryLifetime = 72;
    deliveryDays = Arrays.asList(0, 1, 2, 3, 4, 5, 6);
  }


  public String save() {
    if (policyId != null && !Smsc.RETRY_POLICY_PATTERN.matcher(policyId).matches()) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "retry_policy_incorrect");
      return null;
    }
    if(editCreateCdr && createCdr) {
      if(cdrDestination == null || cdrDestination.length() == 0 || cdrOriginator == null || cdrOriginator.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "user.edit.cdr.props.required");
        return null;
      }
    }else {
      cdrDestination = null;
      cdrOriginator = null;
    }
    Configuration config = getConfig();
    try {
      List<User> users = new ArrayList<User>();
      for (String id : userIds) {
        User u = config.getUser(id);

        if (editOrganization)
          u.setOrganization(organization);

        if (editAdmin) {
          if (admin && !u.hasRole(User.INFORMER_ADMIN_ROLE))
            u.getRoles().add(User.INFORMER_ADMIN_ROLE);
          if (!admin && u.hasRole(User.INFORMER_ADMIN_ROLE))
            u.getRoles().remove(User.INFORMER_ADMIN_ROLE);
        }

        if (editBlocked)
          u.setStatus(blocked ? User.Status.DISABLED : User.Status.ENABLED);

        if (editSourceAddr)
          u.setSourceAddr(sourceAddr);

        if (editDeliveryDays)
          u.setDeliveryDays(new ArrayList<Integer>(deliveryDays));

        if (editDeliveryStartTime) {
          if (deliveryStartTime == null) u.setDeliveryStartTime(null);
          else u.setDeliveryStartTime(new Time(deliveryStartTime));
        }

        if (editDeliveryEndTime) {
          if (deliveryEndTime == null) u.setDeliveryEndTime(null);
          else u.setDeliveryEndTime(new Time(deliveryEndTime));
        }

        if (editPriority)
          u.setPriority(priority);

        if (editRetryPolicy) {
          if (retryOnFail.equals("off")) {
            u.setRetryOnFail(false);
            u.setPolicyId(null);
          } else if (retryOnFail.equals("default")) {
            u.setRetryOnFail(true);
            u.setPolicyId("");
          } else {
            u.setRetryOnFail(true);
            u.setPolicyId(policyId);
          }
        }

        if (editUseDataSm)
          u.setUseDataSm(useDataSm);

        if (editDeliveryType)
          u.setDeliveryType(deliveryType);

        if (editTransactionMode)
          u.setTransactionMode(transactionMode);

        if (editValidityPeriod)
          u.setValidityPeriod(new Time(validityPeriod));

        if (editSmsPerSec)
          u.setSmsPerSec(smsPerSec);

        if (editAllowedRegions) {
          u.setAllRegionsAllowed(allRegionsAllowed);
          u.setRegions(new ArrayList<Integer>(regions));
        }

        if (editSmsNotification)
          u.setSmsNotification(smsNotification);

        if (editEmailNotification)
          u.setEmailNotification(emailNotification);

        if (editArchivateDeliveries) {
          u.setCreateArchive(createArchive);
          u.setDeliveryLifetime(deliveryLifetime);
        }

        if (editCreateCdr) {
          u.setCreateCDR(createCdr);
          u.setCdrDestination(cdrDestination);
          u.setCdrOriginator(cdrOriginator);
        }

        if (editMessageTimeToLive) {
          u.setMessageTimeToLive(messageTimeToLive);
        }

        users.add(u);
      }

      for (User u : users) {
        config.updateUser(u, getUserName());
      }
      return "USERS";
    }
    catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String getCdrDestination() {
    return cdrDestination;
  }

  public void setCdrDestination(String cdrDestination) {
    this.cdrDestination = cdrDestination;
  }

  public String getCdrOriginator() {
    return cdrOriginator;
  }

  public void setCdrOriginator(String cdrOriginator) {
    this.cdrOriginator = cdrOriginator;
  }

  public String getOrganization() {
    return organization;
  }

  public void setOrganization(String organization) {
    this.organization = organization;
  }

  public boolean isAdmin() {
    return admin;
  }

  public void setAdmin(boolean admin) {
    this.admin = admin;
  }

  public int getSmsPerSec() {
    return smsPerSec;
  }

  public void setSmsPerSec(int smsPerSec) {
    this.smsPerSec = smsPerSec;
  }

  public Integer[] getRegions() {
    return regions.toArray(new Integer[regions.size()]);
  }

  public void setRegions(Integer[] regions) {
    this.regions = Arrays.asList(regions);
  }

  public void setSourceAddr(Address sourceAddr) {
      this.sourceAddr = sourceAddr;
  }

  public Address getSourceAddr() {
    return sourceAddr;
  }

  public Integer[] getDeliveryDays() {
    return deliveryDays.toArray(new Integer[deliveryDays.size()]);
  }

  public void setDeliveryDays(Integer[] deliveryDays) {
    this.deliveryDays = Arrays.asList(deliveryDays);
  }

  public Date getDeliveryEndTime() {
    return deliveryEndTime;
  }

  public void setDeliveryEndTime(Date deliveryEndTime) {
    this.deliveryEndTime = deliveryEndTime;
  }

  public Date getDeliveryStartTime() {
    return deliveryStartTime;
  }

  public void setDeliveryStartTime(Date deliveryStartTime) {
    this.deliveryStartTime = deliveryStartTime;
  }

  public Time getValidityPeriod() {
    return validityPeriod;
  }

  public void setValidityPeriod(Time validHours) {
    this.validityPeriod = validHours;
  }

  public User.DeliveryType getDeliveryType() {
    return deliveryType;
  }

  public void setDeliveryType(User.DeliveryType deliveryType) {
    this.deliveryType = deliveryType;
  }

  public boolean isTransactionMode() {
    return transactionMode;
  }

  public void setTransactionMode(boolean transactionMode) {
    this.transactionMode = transactionMode;
  }

  public String getPolicyId() {
    return policyId;
  }

  public void setPolicyId(String policyId) {
    if (policyId != null && policyId.trim().length() == 0) policyId = null;
    this.policyId = policyId;
  }

  public boolean isSmsNotification() {
    return smsNotification;
  }

  public void setSmsNotification(boolean smsNotification) {
    this.smsNotification = smsNotification;
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) {
    this.priority = priority;
  }

  public boolean isEmailNotification() {
    return emailNotification;
  }

  public void setEmailNotification(boolean emailNotification) {
    this.emailNotification = emailNotification;
  }

  public boolean isCreateArchive() {
    return createArchive;
  }

  public void setCreateArchive(boolean createArchive) {
    this.createArchive = createArchive;
  }

  public int getDeliveryLifetime() {
    return deliveryLifetime;
  }

  public void setDeliveryLifetime(int deliveryLifetime) {
    this.deliveryLifetime = deliveryLifetime;
  }

  public String getRetryOnFail() {
    return retryOnFail;
  }

  public void setRetryOnFail(String retryOnFail) {
    this.retryOnFail = retryOnFail;
  }

  public boolean isBlocked() {
    return blocked;
  }

  public void setBlocked(boolean blocked) {
    this.blocked = blocked;
  }

  public boolean isSmsDeliveryMode() {
    return deliveryType == User.DeliveryType.SMS;
  }

  public boolean isAllRegionsAllowed() {
    return allRegionsAllowed;
  }

  public void setAllRegionsAllowed(boolean allRegionsAllowed) {
    this.allRegionsAllowed = allRegionsAllowed;
  }

  public boolean isCreateCdr() {
    return createCdr;
  }

  public void setCreateCdr(boolean createCdr) {
    this.createCdr = createCdr;
  }

  public boolean isEditOrganization() {
    return editOrganization;
  }

  public void setEditOrganization(boolean editOrganization) {
    this.editOrganization = editOrganization;
  }

  public boolean isEditAdmin() {
    return editAdmin;
  }

  public void setEditAdmin(boolean editAdmin) {
    this.editAdmin = editAdmin;
  }

  public boolean isEditBlocked() {
    return editBlocked;
  }

  public void setEditBlocked(boolean editBlocked) {
    this.editBlocked = editBlocked;
  }

  public boolean isEditSourceAddr() {
    return editSourceAddr;
  }

  public void setEditSourceAddr(boolean editSourceAddr) {
    this.editSourceAddr = editSourceAddr;
  }

  public boolean isEditDeliveryDays() {
    return editDeliveryDays;
  }

  public void setEditDeliveryDays(boolean editDeliveryDays) {
    this.editDeliveryDays = editDeliveryDays;
  }

  public boolean isEditDeliveryStartTime() {
    return editDeliveryStartTime;
  }

  public void setEditDeliveryStartTime(boolean editDeliveryStartTime) {
    this.editDeliveryStartTime = editDeliveryStartTime;
  }

  public boolean isEditDeliveryEndTime() {
    return editDeliveryEndTime;
  }

  public void setEditDeliveryEndTime(boolean editDeliveryEndTime) {
    this.editDeliveryEndTime = editDeliveryEndTime;
  }

  public boolean isEditPriority() {
    return editPriority;
  }

  public void setEditPriority(boolean editPriority) {
    this.editPriority = editPriority;
  }

  public boolean isEditRetryPolicy() {
    return editRetryPolicy;
  }

  public void setEditRetryPolicy(boolean editRetryPolicy) {
    this.editRetryPolicy = editRetryPolicy;
  }

  public boolean isEditDeliveryType() {
    return editDeliveryType;
  }

  public void setEditDeliveryType(boolean editDeliveryType) {
    this.editDeliveryType = editDeliveryType;
  }

  public boolean isEditTransactionMode() {
    return editTransactionMode;
  }

  public void setEditTransactionMode(boolean editTransactionMode) {
    this.editTransactionMode = editTransactionMode;
  }

  public boolean isEditValidityPeriod() {
    return editValidityPeriod;
  }

  public void setEditValidityPeriod(boolean editValidityPeriod) {
    this.editValidityPeriod = editValidityPeriod;
  }

  public boolean isEditSmsPerSec() {
    return editSmsPerSec;
  }

  public void setEditSmsPerSec(boolean editSmsPerSec) {
    this.editSmsPerSec = editSmsPerSec;
  }

  public boolean isEditAllowedRegions() {
    return editAllowedRegions;
  }

  public void setEditAllowedRegions(boolean editAllowedRegions) {
    this.editAllowedRegions = editAllowedRegions;
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

  public boolean isEditArchivateDeliveries() {
    return editArchivateDeliveries;
  }

  public void setEditArchivateDeliveries(boolean editArchivateDeliveries) {
    this.editArchivateDeliveries = editArchivateDeliveries;
  }

  public boolean isEditCreateCdr() {
    return editCreateCdr;
  }

  public void setEditCreateCdr(boolean editCreateCrd) {
    this.editCreateCdr = editCreateCrd;
  }

  public boolean isEditUseDataSm() {
    return editUseDataSm;
  }

  public void setEditUseDataSm(boolean editUseDataSm) {
    this.editUseDataSm = editUseDataSm;
  }

  public boolean isUseDataSm() {
    return useDataSm;
  }

  public void setUseDataSm(boolean useDataSm) {
    this.useDataSm = useDataSm;
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
}