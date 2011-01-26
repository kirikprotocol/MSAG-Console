package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Time;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;
import java.util.List;


/**
 * @author Aleksandr Khalitov
 */
public abstract class CPResourceEditController extends CPResourceController{

  protected String userId;

  protected String id;

  protected UserCPsettings settings = new UserCPsettings();

  protected String message;

  protected String messageType="error";

  protected boolean edit;

  protected UserCPsettings.Protocol protocol;

  protected CPResourceEditController(UserCPsettings.Protocol protocol) {
    this.protocol = protocol;
    settings.setProtocol(protocol);
    settings.setActivePeriodStart(new Time(0,0,0));
    settings.setActivePeriodEnd(new Time(23,59,59));
    settings.setPeriodInMin(1);
    settings.setDirectoryMaxSize(10);
    loadSettings();
  }

  public String verifyConnection() {
    User u = getConfig().getUser(userId);
    if(u == null) {
      message = getLocalizedString("cp.resource.edit.userNotFound");
    } else {
      try {
        getConfig().verifyCPSettings(u, settings);
        message = getLocalizedString("cp.resource.edit.conn.verified");
        messageType = "info";
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  protected void loadSettings() {

    String userId = getRequestParameter(USER_ID_PARAMETER);

    if(userId != null && userId.length() >0 ) {
      this.userId = userId;
      edit = true;
      String id = getRequestParameter(CP_SETTINGS_ID_PARAMETER);

      if(id != null && id.length()>0) {
        this.id = id;
        User u = getConfig().getUser(userId);
        if(u == null) {
          message = getLocalizedString("cp.resource.edit.userNotFound");
        }else {
          boolean  found = false;
          if(u.getCpSettings() != null) {
            for(UserCPsettings s : u.getCpSettings()) {
              try {
                if(id.equals(s.getHashId())) {
                  settings = s;
                  found = true;
                  break;
                }
              } catch (AdminException e) {
                addError(e);
              }
            }
          }
          if(!found) {
            message = getLocalizedString("cp.resource.edit.resourceNotFound");
          }
        }
      }

    }

  }

  public String getMessageType() {
    return messageType;
  }

  public boolean isEdit() {
    return edit;
  }

  public void setEdit(boolean edit) {
    this.edit = edit;
  }

  public String getMessage() {
    return message;
  }

  public void setMessage(String message) {
    this.message = message;
  }

  public UserCPsettings getSettings() {
    return settings;
  }


  public List<SelectItem> getFileEncodings() {
    List<SelectItem> ret = new ArrayList<SelectItem>(3);
    ret.add(new SelectItem("cp1251", "cp1251"));
    ret.add(new SelectItem("UTF-8", "UTF-8"));
    ret.add(new SelectItem("ASCII", "ASCII"));
    return ret;
  }

  public Date getActivePeriodStart() {
    if(settings == null || settings.getActivePeriodStart() == null) {
      return null;
    }
    return settings.getActivePeriodStart().getTimeDate();
  }

  public void setActivePeriodStart(Date date) {
    if(settings == null) {
      return;
    }
    settings.setActivePeriodStart(date == null ? null : new Time(date));
  }

  public Date getActivePeriodEnd() {
    if(settings == null || settings.getActivePeriodEnd() == null) {
      return null;
    }
    return settings.getActivePeriodEnd().getTimeDate();
  }

  public void setActivePeriodEnd(Date date) {
    if(settings == null) {
      return;
    }
    settings.setActivePeriodEnd(date == null ? null : new Time(date));
  }



  public String save() {
    User u = getConfig().getUser(userId);
    if(u == null) {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "cp.resource.edit.userNotFound");
      return null;
    }
    try {

      List<UserCPsettings> ss = u.getCpSettings();
      if(ss == null) {
        ss = new ArrayList<UserCPsettings>(1);
      }else if(id != null) {
        Iterator<UserCPsettings> i = ss.iterator();
        while(i.hasNext()) {
          UserCPsettings s = i.next();
          if(id.equals(s.getHashId())) {
            i.remove();
            break;
          }
        }
      }
      ss.add(settings);
      u.setCpSettings(ss);
      getConfig().updateUser(u, getUserName());
    } catch (AdminException e) {
      addError(e);
      return null;
    }
    return "CP_RESOURCE_LIST";

  }

  public String getUserId() {
    return userId;
  }

  public void setUserId(String userId) {
    this.userId = userId;
  }

  public String getId() {
    return id;
  }

  public void setId(String id) {
    this.id = id;
  }
}
