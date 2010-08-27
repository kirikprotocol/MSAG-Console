package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.web.WebContext;

import javax.faces.application.FacesMessage;
import javax.servlet.http.HttpSession;
import java.io.Serializable;
import java.util.*;

/**
 * author: alkhal
 */
@SuppressWarnings({"unchecked"})
public class UserEditController extends SmscController{

  private boolean edit_initialized;

  private String oldLogin;

  private User user;

  private Map<String, User> users;

  private List<Role> roles;

  private String lang;

  private String confirm;

  public UserEditController() {
    HttpSession session = getSession(false);
    users = (Map<String, User>) session.getAttribute("users.users");
    if(users == null) {
      throw new IllegalStateException("Session's parameters aren't initialized correctly");
    }

    if(getRequestParameter("edit_initialized") == null) {
      oldLogin = getRequestParameter("login");
      user = oldLogin != null ? users.get(oldLogin) : new User();

      lang = user.getPrefs().getLocale() == null ? null : user.getPrefs().getLocale().getLanguage();
      confirm = user.getPassword();
      
      Set<String> allRoles = WebContext.getInstance().getWebXml().getRoles();
      roles = new ArrayList<Role>(allRoles.size());
      for(String r : allRoles) {
        roles.add(new Role(r, user.hasRole(r)));
      }
      edit_initialized = true;
    }
  }

  public boolean isEdit_initialized() {
    return edit_initialized;
  }

  public void setEdit_initialized(boolean edit_initialized) {
    this.edit_initialized = edit_initialized;
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
      addMessage(FacesMessage.SEVERITY_WARN, e.getMessage(getLocale()));
    }

    if(oldLogin != null && oldLogin.length() != 0) {
      users.remove(oldLogin);
    }
    users.put(user.getLogin(), user);

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
