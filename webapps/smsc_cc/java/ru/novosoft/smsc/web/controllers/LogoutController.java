package ru.novosoft.smsc.web.controllers;

import javax.faces.context.FacesContext;
import javax.servlet.http.HttpSession;
import java.security.Principal;

/**
 * author: alkhal
 */
public class LogoutController {

  public String getUserName() {
    Principal p = FacesContext.getCurrentInstance().getExternalContext().getUserPrincipal();
    return p == null ? null : p.getName();
  }

  public String logout() {
    HttpSession session = (HttpSession) FacesContext.getCurrentInstance().getExternalContext().getSession(true);
    if (session != null) {
      session.invalidate();
    }
    return "INDEX";
  }
}
