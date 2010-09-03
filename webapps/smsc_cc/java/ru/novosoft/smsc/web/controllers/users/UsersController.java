package ru.novosoft.smsc.web.controllers.users;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;
import javax.servlet.http.HttpSession;
import java.util.Map;

/**
 * author: alkhal
 */
public class UsersController extends SmscController {

  private static final Logger logger = Logger.getLogger(UsersController.class);

  protected Map<String, User> getUsersFromSession(boolean putIfNeeded) {
    return getUsersFromSession(getSession(false), putIfNeeded);
  }

  @SuppressWarnings({"unchecked"})
  protected Map<String, User> getUsersFromSession(HttpSession s, boolean putIfNeeded) {
    Map<String, User> users = (Map<String, User>)s.getAttribute("users.users");
    if(users == null && putIfNeeded) {
      setLastUpdate(s, WebContext.getInstance().getAppliableConfiguration().getUsersSettingsUpdateInfo().getLastUpdateTime());
      try {
        users = WebContext.getInstance().getAppliableConfiguration().getUsersSettings().getUsersMap();
        s.setAttribute("users.users", users);
      } catch (AdminException e) {
        logger.error(e,e);
        addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));
      }

     }
    return users;
  }

  protected void putUsersToSession(Map<String, User> users) {
    putUsersToSession(getSession(false), users);
  }

  protected void putUsersToSession(HttpSession s, Map<String, User> users) {
    s.setAttribute("users.users", users);
  }

  protected void setLastUpdate(long u) {
    setLastUpdate(getSession(false), u);
  }

  protected void setLastUpdate(HttpSession s, long u) {
    s.setAttribute("users.last.update", u);
  }

  protected Long getLastUpdate() {
    return getLastUpdate(getSession(false));
  }

  protected Long getLastUpdate(HttpSession s) {
    return (Long)s.getAttribute("users.last.update");
  }

  protected void cleanSession() {
    cleanSession(getSession(false));
  }

  protected void cleanSession(HttpSession s) {
    s.removeAttribute("users.users");
    s.removeAttribute("users.last.update");
  }

  public void setChanged(boolean changed) {
    HttpSession s = getSession(false);
    if(changed) {
      s.setAttribute("users.changed", true);
    }else {
      s.removeAttribute("users.changed");
    }
  }

  public boolean isChanged() {
    return getSession(false).getAttribute("users.changed") != null;
  }

}
