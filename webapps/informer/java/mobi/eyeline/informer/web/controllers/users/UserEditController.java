package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.admin.users.UserException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableRow;

import javax.faces.application.FacesMessage;
import javax.faces.context.FacesContext;
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
  private DynamicTableModel dynamicSfptModel = new DynamicTableModel();
  private DynamicTableModel dynamicFileModel = new DynamicTableModel();
  private String retryOnFail;

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
      userToEdit.setSmsPerSec(1);
      retryOnFail = "off";
    } else {
      userToEdit = getConfig().getUser(userId);
      if (userToEdit.isRetryOnFail()) {
        retryOnFail = (userToEdit.getPolicyId() == null || userToEdit.getPolicyId().length()==0) ? "default" : "custom";
      } else
        retryOnFail = "off";
    }
    this.userId = userId;
    passwordConfirm = userToEdit.getPassword();

    dynamicSfptModel = new DynamicTableModel();
    dynamicFileModel = new DynamicTableModel();
    if(userToEdit.getCpSettings()!=null) {
      for (UserCPsettings ucps : userToEdit.getCpSettings()) {
        DynamicTableRow row = new DynamicTableRow();
        row.setValue("directory", ucps.getDirectory());
        row.setValue("encoding", ucps.getEncoding());
        row.setValue("sourceAddress", ucps.getSourceAddress().getSimpleAddress());

        if(ucps.getProtocol()==UserCPsettings.Protocol.sftp) {
          row.setValue("host", ucps.getHost());
          row.setValue("port", ucps.getPort()==null ? "": ucps.getPort().toString());
          row.setValue("login", ucps.getLogin());
          row.setValue("password", ucps.getPassword());
          dynamicSfptModel.addRow(row);
        }
        else {
          dynamicFileModel.addRow(row);
        }
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

      if (retryOnFail.equals("off")) {
        userToEdit.setRetryOnFail(false);
        userToEdit.setPolicyId(null);
      } else if (retryOnFail.equals("default")) {
        userToEdit.setRetryOnFail(true);
        userToEdit.setPolicyId("");
      } else {
        userToEdit.setRetryOnFail(true);        
      }

      if (retryOnFail.equals("custom") && (userToEdit.getPolicyId() == null || !Smsc.RETRY_POLICY_PATTERN.matcher(userToEdit.getPolicyId()).matches())) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "retry_policy_incorrect");
        return null;
      }

      if (userId == null) {
        getConfig().addUser(userToEdit, getUserName());
      } else {
        getConfig().updateUser(userToEdit, getUserName());
      }


        List<UserCPsettings> cpSettings = new ArrayList<UserCPsettings>();
        for(DynamicTableRow row : dynamicSfptModel.getRows()) {
           UserCPsettings ucps = new UserCPsettings();
           ucps.setProtocol(UserCPsettings.Protocol.sftp);
           ucps.setHost((String)row.getValue("host"));
           String sPort = (String)row.getValue("port");
           ucps.setPort( (sPort==null || sPort.length()==0) ? null : Integer.valueOf(sPort));
           ucps.setLogin((String) row.getValue("login"));
           ucps.setPassword((String) row.getValue("password"));
           ucps.setDirectory((String) row.getValue("directory"));
           ucps.setEncoding((String) row.getValue("encoding"));
           try {
            ucps.setSourceAddress(new Address((String) row.getValue("sourceAddress")));
           }
           catch (IllegalArgumentException e) {
            addLocalizedMessage(FacesMessage.SEVERITY_ERROR,"user.edit.invalid.sourceAddress",row.getValue("sourceAddress"));
            return null;
           }
           ucps.checkValid();

           cpSettings.add(ucps);
        }
        for(DynamicTableRow row : dynamicFileModel.getRows()) {
           UserCPsettings ucps = new UserCPsettings();
           ucps.setProtocol(UserCPsettings.Protocol.file);
           ucps.setDirectory((String) row.getValue("directory"));
           ucps.setEncoding((String) row.getValue("encoding"));
           try {
            ucps.setSourceAddress(new Address((String) row.getValue("sourceAddress")));
           }
           catch (IllegalArgumentException e) {
             addLocalizedMessage(FacesMessage.SEVERITY_ERROR,"user.edit.invalid.sourceAddress",row.getValue("sourceAddress"));
             return null;
           }
           ucps.checkValid();

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

  public String verifyUcps() {
    List<UserCPsettings> ucpsList = userToEdit.getCpSettings();
    if(ucpsList!=null) {
      for(UserCPsettings ucps : ucpsList) {
        try {
            getConfig().verifyCPSettings(userToEdit,ucps  );
        }
        catch (AdminException e) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN,"user.edit.connect.fail",ucps.toString());
        }
      }
    }
    return null;
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

  public List<String> getFileEncodings() {
    List<String> ret = new ArrayList<String>();
    ret.add("cp1251");
    ret.add("UTF-8");
    ret.add("ASCII");
    return ret;
  }

  public String getRetryOnFail() {
    return retryOnFail;
  }

  public void setRetryOnFail(String retryOnFail) {
    this.retryOnFail = retryOnFail;
  }

  public DynamicTableModel getDynamicSfptModel() {
    return dynamicSfptModel;
  }

  public void setDynamicSfptModel(DynamicTableModel dynamicSfptModel) {
    this.dynamicSfptModel = dynamicSfptModel;
  }

  public DynamicTableModel getDynamicFileModel() {
    return dynamicFileModel;
  }

  public void setDynamicFileModel(DynamicTableModel dynamicFileModel) {
    this.dynamicFileModel = dynamicFileModel;
  }

  public List<String> getUcpsProtocols() {
    List<String> ret = new ArrayList<String>();
    for(UserCPsettings.Protocol v : UserCPsettings.Protocol.values()) {
      ret.add(v.toString());
    }
    return ret;
  }
}