package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.context.FacesContext;
import javax.faces.model.SelectItem;
import javax.servlet.http.HttpSession;
import java.util.*;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 14.10.2010
 * Time: 18:25:48
 */
public class UserGroupEditController extends UserController {
  private List<String> userIds;
  private String organization;
  private boolean admin;
  private int smsPerSec;
  private List<String> regions = new ArrayList<String>();
  private Address sourceAddr;
  private List<Integer> deliveryDays = new ArrayList<Integer>();
  private String deliveryEndTime;
  private String deliveryStartTime;
  private int validHours;
  private User.DeliveryType deliveryType;

  private boolean transactionMode;
  private String policyId;
  private int priority;
  private boolean smsNotification;
  private boolean emailNotification;
  private boolean createArchive;
  private int deliveryLifetime;




//Вкл/откл принудительную архивацию рассылок и макс. время жизни рассылки.
//Настройки рассылок по-умолчанию (см. выше).

  public UserGroupEditController() throws AdminException {
    super();
    HttpSession session =  getSession(false);
    if(session!=null) {
      userIds = (List<String>) session.getAttribute("userIds");
    }
  }


  public String save() {
    Configuration config = getConfig();
    try {
      List<User> users = new ArrayList<User>();
      for(String id : userIds) {
        User u = config.getUser(id);
        u.setOrganization(organization);
        if(admin && !u.hasRole(User.INFORMER_ADMIN_ROLE)) {
          u.getRoles().add(User.INFORMER_ADMIN_ROLE);
        }
        if(!admin && u.hasRole(User.INFORMER_ADMIN_ROLE)) {
          u.getRoles().remove(User.INFORMER_ADMIN_ROLE);
        }
        u.setSmsPerSec(smsPerSec);
        u.setRegions(new ArrayList<String>(regions));
        u.setSourceAddr(sourceAddr);
        u.setDeliveryDays(new ArrayList<Integer>(deliveryDays));


        if(deliveryEndTime==null || deliveryEndTime.trim().length()==0) u.setDeliveryEndTime(null);
        else u.setDeliveryEndTime(new Time(deliveryEndTime));

        if(deliveryStartTime==null || deliveryStartTime.trim().length()==0) u.setDeliveryStartTime(null);
        else u.setDeliveryStartTime(new Time(deliveryStartTime));


        u.setValidHours(validHours);
        u.setDeliveryType(deliveryType);


        u.setTransactionMode(transactionMode);
        u.setPolicyId(policyId);
        u.setPriority(priority);
        u.setSmsNotification(smsNotification);
        u.setEmailNotification(emailNotification);
        u.setCreateArchive(createArchive);
        u.setDeliveryLifetime(deliveryLifetime);

        users.add(u);
      }
        
      for(User u : users) {
        config.updateUser(u,getUserName());
      }
      return "USERS";
    }
    catch (AdminException e) {
      addError(e);
    }
    return null;
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

  public List<String> getRegions() {
    return regions;
  }

  public void setRegions(List<String> regions) {
    this.regions = regions;
  }

  public void setSourceAddr(String sourceAddr) {
    if(sourceAddr!=null && sourceAddr.trim().length() > 0) {
      this.sourceAddr = new Address(sourceAddr);
    }
  }

  public String getSourceAddr() {
    return sourceAddr==null ? null : sourceAddr.getSimpleAddress();
  }

  public List<Integer> getDeliveryDays() {
    return deliveryDays;
  }

  public void setDeliveryDays(List<Integer> deliveryDays) {
    this.deliveryDays = deliveryDays;
  }

  public String getDeliveryEndTime() {
    return deliveryEndTime;
  }

  public void setDeliveryEndTime(String deliveryEndTime) {
    if(deliveryEndTime!=null && deliveryEndTime.trim().length()>0) new Time(deliveryEndTime);
    this.deliveryEndTime = deliveryEndTime;
  }

  public String getDeliveryStartTime() {
    return deliveryStartTime;
  }

  public void setDeliveryStartTime(String deliveryStartTime) {
    if(deliveryStartTime!=null && deliveryStartTime.trim().length()>0) new Time(deliveryStartTime);
    this.deliveryStartTime = deliveryStartTime;
  }

  public int getValidHours() {
    return validHours;
  }

  public void setValidHours(int validHours) {
    this.validHours = validHours;
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
    if(policyId!=null && policyId.trim().length()==0) policyId=null;
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
}