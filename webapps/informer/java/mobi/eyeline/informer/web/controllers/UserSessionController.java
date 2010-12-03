package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.AdminException;

import javax.faces.application.FacesMessage;
import javax.faces.context.FacesContext;
import javax.servlet.http.HttpSession;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

/**
 * Контроллер сеанса пользователя
 *
 * @author Aleksandr Khalitov
 */
public class UserSessionController extends InformerController {

  public boolean isLogined() {
    return getUserPrincipal() != null;
  }

  public boolean isBlacklistEnabled() {
    try{
      return getConfig().isBlackListEnabled();
    }catch (AdminException e) {
      addError(e);
      return false;
    }
  }

  public String logout() {
    HttpSession session = getSession(false);
    if (session != null) {
      session.invalidate();
    }
    return "INDEX";
  }

  public List<FacesMessage> getMessages() {
    List<FacesMessage> result = new LinkedList<FacesMessage>();
    Iterator<FacesMessage> i=  FacesContext.getCurrentInstance().getMessages();
    while(i.hasNext()) {
      result.add(i.next());
    }
    return result;
  }

  public boolean isHasMessages() {
    return FacesContext.getCurrentInstance().getMessages().hasNext();
  }
}
