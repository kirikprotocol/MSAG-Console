package ru.novosoft.smsc.web.controllers.users;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.web.WebContext;

import javax.faces.application.FacesMessage;
import java.io.Serializable;
import java.util.*;

/**
 * author: alkhal
 */
@SuppressWarnings({"unchecked"})
public class UserEditController extends UsersController{

  private String oldLogin;

  private User user;

  private List<Role> roles;

  private String lang;

  private String confirm;

  public UserEditController() {

    Map<String, User> users = getSettings().getUsersMap();

    if(getRequestParameter("edit_initialized") == null) {
      oldLogin = getRequestParameter("login");
      user = oldLogin != null ? users.get(oldLogin) : new User();

      lang = user.getPrefs().getLocale() == null ? "en" : user.getPrefs().getLocale().getLanguage();
      confirm = user.getPassword();

      Set<String> allRoles = WebContext.getInstance().getWebXml().getRoles();
      roles = new ArrayList<Role>(allRoles.size());
      for(String r : allRoles)
        roles.add(new Role(r, user.hasRole(r)));
    }
  }

  public String getOldLogin() {
    return oldLogin;
  }

  public void setOldLogin(String oldLogin) {
    this.oldLogin = oldLogin;
  }

  public String done() {

    if(!user.getPassword().equals(confirm)) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.user.edit.not.confirm");
      return null;      
    }

    UsersSettings settings = getSettings();
    Map<String, User> users = settings.getUsersMap();

    if(users.get(user.getLogin()) != null && (oldLogin == null || oldLogin.length() == 0)) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.users.duplicate");
      return null;
    }


    Set<String> newRoles = new TreeSet<String>();
    for(Role r : roles) {
      if(r.checked) {
        newRoles.add(r.name);
      }
    }

    user.setRoles(newRoles);

    try{
      user.getPrefs().setLocale(new Locale(lang));
    }catch (AdminException e) {
      addError(e);
    }

    if(oldLogin != null && oldLogin.length() != 0) {
      users.remove(oldLogin);
    }
    users.put(user.getLogin(), user);

    try {
      settings.setUsers(users.values());
      setSettings(settings);
    } catch (AdminException e) {
      addError(e);
    }

    return "USERS";
  }

  public String getLang() {
    return lang;
  }

  public void setLang(String lang) {
    this.lang = lang;
  }

  public User getUser() {
    return user;
  }

  public void setUser(User user) {
    this.user = user;
  }

  public List<Role> getRoles() {
    return roles;
  }

  public void setRoles(List<Role> roles) {
    this.roles = roles;
  }

  public String getConfirm() {
    return confirm;
  }

  public void setConfirm(String confirm) {
    this.confirm = confirm;
  }

  public static class Role implements Serializable{
    private String name;
    private boolean checked;

    public Role() {
    }

    public Role(String name, boolean checked) {
      this.name = name;
      this.checked = checked;
    }

    public String getName() {
      return name;
    }

    public void setName(String name) {
      this.name = name;
    }

    public boolean isChecked() {
      return checked;
    }

    public void setChecked(boolean checked) {
      this.checked = checked;
    }
  }



}
