package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.10.2010
 * Time: 15:26:51
 */
public class UserEditController extends UserController {

  private String userId;
  private User userToEdit;
  private static final String USER_ID_PARAMETER = "userId";
  private boolean initError;
  private String passwordConfirm;
  private Object fileEncodings;


  public UserEditController() {
    super();
    try {
      setUserId(getRequestParameter(USER_ID_PARAMETER));
    }
    catch (AdminException e){
      initError = true;
    }
  }

  public void setSelectedRows(List<String> rows) throws AdminException {
    if(rows.size()==1) {
      setUserId(rows.get(0));
    }
  }

  public String getUserId() {
    return userId;
  }

  private void setUserId(String userId) throws AdminException {
    if(userId==null || userId.length()==0) {
      userId=null;
      userToEdit = new User();
      userToEdit.setDeliveryStartTime(new Time(9,0,0));
      userToEdit.setDeliveryEndTime(new Time(22,0,0));
      userToEdit.setDeliveryDays(Arrays.<Integer>asList(0,1,2,3,4,5,6));
      userToEdit.setValidHours(3);
      userToEdit.setPriority(1);
      userToEdit.setAllRegionsAllowed(true);
    }
    else {
      userToEdit = getConfig().getUser(userId);
    }    
    this.userId = userId;
    passwordConfirm = userToEdit.getPassword();
  }

  public void setActiveWeekDays(Integer[] days) throws AdminException {
    userToEdit.setDeliveryDays(Arrays.<Integer>asList(days));
  }

  public Integer[] getActiveWeekDays() {
    return userToEdit.getDeliveryDays().toArray(new Integer[0]);
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }

  public String save() {
    try {
      if(!passwordConfirm.equals(userToEdit.getPassword())) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN,"user.edit.passwdConfirmMissmatch");
        return null;
      }
      if(userToEdit.getPolicyId() != null && !Smsc.RETRY_POLICY_PATTERN.matcher(userToEdit.getPolicyId()).matches()) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN,"retry_policy_incorrect");
        return null;
      }

      if(userId==null) {
        getConfig().addUser(userToEdit,getUserName());
      }
      else {
        getConfig().updateUser(userToEdit,getUserName());
      }
    }
    catch (AdminException e) {
      addError(e);
      return null;
    }

    return "USERS";  //To change body of created methods use File | Settings | File Templates.
  }

  public User getUserToEdit() {
    return userToEdit;
  }


  public boolean isAdmin() {
    return userToEdit.hasRole(User.INFORMER_ADMIN_ROLE);
  }

  public void setAdmin(boolean inRole) {
    if(inRole && !userToEdit.hasRole(User.INFORMER_ADMIN_ROLE)) {
      userToEdit.getRoles().add(User.INFORMER_ADMIN_ROLE);
      return;
    }
    if(!inRole && userToEdit.hasRole(User.INFORMER_ADMIN_ROLE)) {
      userToEdit.getRoles().remove(User.INFORMER_ADMIN_ROLE);
    }
  }



  public String getPasswordConfirm() {
    return passwordConfirm;
  }

  public void setPasswordConfirm(String passwordConfirm) {
    this.passwordConfirm = passwordConfirm;
  }



  public void setSourceAddr(String sourceAddr) throws AdminException {
    userToEdit.setSourceAddr(new Address(sourceAddr));
  }

  public String getSourceAddr() {
    return userToEdit.getSourceAddr()==null ? null : userToEdit.getSourceAddr().getSimpleAddress();
  }



  public void setDeliveryEndTime(Date t) {
    if(t==null) userToEdit.setDeliveryEndTime(null);
    else userToEdit.setDeliveryEndTime(new Time(t));
  }
  public void setDeliveryStartTime(Date t) {
    if(t==null) userToEdit.setDeliveryStartTime(null);
    else userToEdit.setDeliveryStartTime(new Time(t));
  }

  public Date getDeliveryEndTime() {
    if(userToEdit.getDeliveryEndTime()==null) return null;
    return userToEdit.getDeliveryEndTime().getTimeDate();
  }

  public Date getDeliveryStartTime() {
    if(userToEdit.getDeliveryStartTime()==null) return null;
    return userToEdit.getDeliveryStartTime().getTimeDate();
  }

  public boolean isBlocked() {
    return userToEdit.getStatus() == User.Status.DISABLED;
  }

  public void setBlocked(boolean blocked) throws AdminException {
    userToEdit.setStatus(blocked ? User.Status.DISABLED : User.Status.ENABLED);
  }

  public boolean isSmsDeliveryMode() {
    return userToEdit.getDeliveryType() == User.DeliveryType.SMS;
  }

  public List<SelectItem> getLocales() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    Locale loc = new Locale("en");
    ret.add(new SelectItem(loc.getLanguage()));
    loc = new Locale("ru");
    ret.add(new SelectItem(loc.getLanguage()));
    return ret;
  }


  public String getUserLocale() {
    Locale loc = userToEdit.getLocale();
    return loc==null ? null : loc.getLanguage();
  }

  public  void setUserLocale(String l) {
    userToEdit.setLocale(new Locale(l));
  }

  public Object getFileEncodings() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem("cp1251"));
    ret.add(new SelectItem("UTF-8"));
    ret.add(new SelectItem("ASCII"));
    return ret;
  }
}