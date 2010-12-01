package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.AdminException;

import javax.servlet.http.HttpSession;

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
}
