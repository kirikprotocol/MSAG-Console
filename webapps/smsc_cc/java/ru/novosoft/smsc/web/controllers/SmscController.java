package ru.novosoft.smsc.web.controllers;

import javax.faces.application.FacesMessage;
import javax.faces.context.FacesContext;
import javax.servlet.http.HttpSession;
import java.io.Serializable;
import java.security.Principal;
import java.util.Locale;
import java.util.Map;
import java.util.ResourceBundle;

/**
 * author: alkhal
 */
public abstract class SmscController implements Serializable {

  public void addMessage(FacesMessage.Severity severity, String message) {
    FacesContext fc = FacesContext.getCurrentInstance();
    FacesMessage facesMessage = new FacesMessage(severity, message, "");
    fc.addMessage("smsc_errors", facesMessage);
  }

  public void addLocalizedMessage(FacesMessage.Severity severity, String bundleKey) {
    FacesContext fc = FacesContext.getCurrentInstance();
    FacesMessage facesMessage = new FacesMessage(severity,
        ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc",
            fc.getExternalContext().getRequestLocale()).getString(bundleKey), "");
    fc.addMessage("smsc_errors", facesMessage);
  }

  public String getRequestParameter(String name) {
    return FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().get(name);
  }

  public Map<String,String> getRequestParameters() {
    return FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap();
  }

  public HttpSession getSession(boolean createIfNeeded) {
    return (HttpSession)FacesContext.getCurrentInstance().getExternalContext().getSession(createIfNeeded);
  }

  public Locale getLocale() {
    return FacesContext.getCurrentInstance().getExternalContext().getRequestLocale();
  }

  public Principal getUserPrincipal() {
    return FacesContext.getCurrentInstance().getExternalContext().getUserPrincipal();
  }

}
