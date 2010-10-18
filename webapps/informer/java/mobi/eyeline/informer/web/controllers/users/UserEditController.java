package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicy;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.List;

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
    }
    else {
      userToEdit = getConfig().getUser(userId);
    }    
    this.userId = userId;
    passwordConfirm = userToEdit.getPassword();
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



  public void setDeliveryEndTime(String t) {
    if(t==null || t.trim().length()==0) userToEdit.setDeliveryEndTime(null);
    else userToEdit.setDeliveryEndTime(new Time(t));
  }
  public void setDeliveryStartTime(String t) {
    if(t==null || t.trim().length()==0) userToEdit.setDeliveryStartTime(null);
    else userToEdit.setDeliveryStartTime(new Time(t));
  }

  public String getDeliveryEndTime() {
    if(userToEdit.getDeliveryEndTime()==null) return null;
    return userToEdit.getDeliveryEndTime().getTimeString();
  }

  public String getDeliveryStartTime() {
    if(userToEdit.getDeliveryStartTime()==null) return null;
    return userToEdit.getDeliveryStartTime().getTimeString();
  }

  


}