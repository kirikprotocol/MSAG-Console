package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.web.util.WebUtils;

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

  protected void addMessage(FacesMessage.Severity severity, String message) {
    addMessage(severity, message, "");
  }

  protected void addMessage(FacesMessage.Severity severity, String message, String detail) {
    FacesContext fc = FacesContext.getCurrentInstance();
    FacesMessage facesMessage = new FacesMessage(severity, message, detail);
    fc.addMessage("smsc_errors", facesMessage);
  }

  protected void addLocalizedMessage(FacesMessage.Severity severity, String bundleKey) {
    FacesContext fc = FacesContext.getCurrentInstance();
    FacesMessage facesMessage = new FacesMessage(severity,
        ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc",
            fc.getExternalContext().getRequestLocale()).getString(bundleKey), "");
    fc.addMessage("smsc_errors", facesMessage);
  }   

  protected String getRequestParameter(String name) {
    return FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().get(name);
  }

  protected Map<String,String> getRequestParameters() {
    return FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap();
  }

  protected HttpSession getSession(boolean createIfNeeded) {
    return (HttpSession)FacesContext.getCurrentInstance().getExternalContext().getSession(createIfNeeded);
  }

  protected Locale getLocale() {
    return WebUtils.getLocale();
  }

  protected Principal getUserPrincipal() {
    return FacesContext.getCurrentInstance().getExternalContext().getUserPrincipal();
  }

  public boolean isAccessToUsers() {
    return FacesContext.getCurrentInstance().getExternalContext().isUserInRole("users");
  }

}
