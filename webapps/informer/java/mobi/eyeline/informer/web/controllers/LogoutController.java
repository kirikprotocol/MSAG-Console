package mobi.eyeline.informer.web.controllers;

import javax.servlet.http.HttpSession;

/**
 * Контроллер для прекращения сеанса пользователя
 *
 * @author Aleksandr Khalitov
 */
public class LogoutController extends InformerController {

  public String logout() {
    HttpSession session = getSession(false);
    if (session != null) {
      session.invalidate();
    }
    return "INDEX";
  }
}
