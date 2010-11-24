package mobi.eyeline.informer.web.controllers;

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

  public String logout() {
    HttpSession session = getSession(false);
    if (session != null) {
      session.invalidate();
    }
    return "INDEX";
  }
}
