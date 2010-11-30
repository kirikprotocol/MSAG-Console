package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableRow;

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
  private DynamicTableModel dynamicModel = new DynamicTableModel();

  public UserEditController() {
    super();
    try {
      setUserId(getRequestParameter(USER_ID_PARAMETER));
    }
    catch (AdminException e) {
      initError = true;
    }
  }

  public void setSelectedRows(List<String> rows) throws AdminException {
    if (rows.size() == 1) {
      setUserId(rows.get(0));
    }
  }

  public String getUserId() {
    return userId;
  }

  private void setUserId(String userId) throws AdminException {
    if (userId == null || userId.length() == 0) {
      userId = null;
      userToEdit = new User();
      userToEdit.setDeliveryStartTime(new Time(9,0,0));
      userToEdit.setDeliveryEndTime(new Time(22,0,0));
      userToEdit.setDeliveryDays(Arrays.<Integer>asList(0,1,2,3,4,5,6));
      userToEdit.setValidityPeriod(new Time(1,0,0));
      userToEdit.setPriority(1);
      userToEdit.setAllRegionsAllowed(true);      
      userToEdit.setDeliveryLifetime(72);
      userToEdit.setDeliveryType(User.DeliveryType.SMS);
    } else {
      userToEdit = getConfig().getUser(userId);
    }
    this.userId = userId;
    passwordConfirm = userToEdit.getPassword();

    dynamicModel = new DynamicTableModel();
    if(userToEdit.getCpSettings()!=null) {
      for (UserCPsettings ucps : userToEdit.getCpSettings()) {
        DynamicTableRow row = new DynamicTableRow();
        row.setValue("host", ucps.getHost());
        row.setValue("port", ucps.getPort());
        row.setValue("login", ucps.getLogin());
        row.setValue("password", ucps.getPassword());
        row.setValue("encoding", ucps.getEncoding());
        row.setValue("directory", ucps.getDirectory());
        row.setValue("sourceAddress", ucps.getSourceAddress().getSimpleAddress());
        dynamicModel.addRow(row);
      }
    }
  }

  public void setActiveWeekDays(Integer[] days) throws AdminException {
    userToEdit.setDeliveryDays(Arrays.<Integer>asList(days));
  }

  public Integer[] getActiveWeekDays() {
    return userToEdit.getDeliveryDays().toArray(new Integer[userToEdit.getDeliveryDays().size()]);
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }

  public String save() {
    try {
      if(userToEdit.isCreateCDR()) {
        if(userToEdit.getCdrDestination() == null || userToEdit.getCdrDestination().length() == 0 ||
            userToEdit.getCdrOriginator() == null || userToEdit.getCdrOriginator().length() == 0) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN, "user.edit.cdr.props.required");
          return null;
        }
      }else {
        userToEdit.setCdrDestination(null);
        userToEdit.setCdrOriginator(null);
      }
      if (!passwordConfirm.equals(userToEdit.getPassword())) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "user.edit.passwdConfirmMissmatch");
        return null;
      }
      if (userToEdit.getPolicyId() != null && !Smsc.RETRY_POLICY_PATTERN.matcher(userToEdit.getPolicyId()).matches()) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "retry_policy_incorrect");
        return null;
      }

      if (userId == null) {
        getConfig().addUser(userToEdit, getUserName());
      } else {
        getConfig().updateUser(userToEdit, getUserName());
      }

      List<UserCPsettings> cpSettings = new ArrayList<UserCPsettings>();
      for(DynamicTableRow row :dynamicModel.getRows()) {
         UserCPsettings ucps = new UserCPsettings();
         ucps.setHost((String)row.getValue("host"));
         ucps.setPort(Integer.valueOf((String)row.getValue("port")));
         ucps.setLogin((String) row.getValue("login"));
         ucps.setPassword((String) row.getValue("password"));
         ucps.setDirectory((String) row.getValue("directory"));
         ucps.setEncoding((String) row.getValue("encoding"));
         ucps.setSourceAddress(new Address((String) row.getValue("sourceAddress")));
         cpSettings.add(ucps);
      }
      userToEdit.setCpSettings(cpSettings.isEmpty()?null:cpSettings);

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
    if (inRole && !userToEdit.hasRole(User.INFORMER_ADMIN_ROLE)) {
      userToEdit.getRoles().add(User.INFORMER_ADMIN_ROLE);
      return;
    }
    if (!inRole) {
      if (userToEdit.hasRole(User.INFORMER_ADMIN_ROLE))
        userToEdit.getRoles().remove(User.INFORMER_ADMIN_ROLE);
      if (!userToEdit.hasRole(User.INFORMER_USER_ROLE))
        userToEdit.getRoles().add(User.INFORMER_USER_ROLE);
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
    return userToEdit.getSourceAddr() == null ? null : userToEdit.getSourceAddr().getSimpleAddress();
  }


  public void setDeliveryEndTime(Date t) {
    if (t == null) userToEdit.setDeliveryEndTime(null);
    else userToEdit.setDeliveryEndTime(new Time(t));
  }

  public void setDeliveryStartTime(Date t) {
    if (t == null) userToEdit.setDeliveryStartTime(null);
    else userToEdit.setDeliveryStartTime(new Time(t));
  }

  public Date getDeliveryEndTime() {
    if (userToEdit.getDeliveryEndTime() == null) return null;
    return userToEdit.getDeliveryEndTime().getTimeDate();
  }

  public Date getDeliveryStartTime() {
    if (userToEdit.getDeliveryStartTime() == null) return null;
    return userToEdit.getDeliveryStartTime().getTimeDate();
  }

  public void setValidityPeriod(Date period) throws AdminException {
    if (period == null)
      userToEdit.setValidityPeriod(null);
    else
      userToEdit.setValidityPeriod(new Time(period));
  }

  public Date getValidityPeriod() {
    if (userToEdit.getValidityPeriod() == null)
      return null;
    return userToEdit.getValidityPeriod().getTimeDate();
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
    return loc == null ? null : loc.getLanguage();
  }

  public void setUserLocale(String l) {
    userToEdit.setLocale(new Locale(l));
  }

  public Object getFileEncodings() {
    List<String> ret = new ArrayList<String>();
    ret.add("cp1251");
    ret.add("UTF-8");
    ret.add("ASCII");
    return ret;
  }

  public DynamicTableModel getDynamicModel() {
    return dynamicModel;
  }

  public void setDynamicModel(DynamicTableModel dynamicModel) {
    this.dynamicModel = dynamicModel;
  }
}