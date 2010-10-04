package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.logging.Logger;
import ru.novosoft.smsc.web.LocaleFilter;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.config.Configuration;
import ru.novosoft.smsc.web.config.SmscStatusManager;

import javax.faces.application.FacesMessage;
import javax.faces.context.FacesContext;
import javax.servlet.http.HttpSession;
import java.io.Serializable;
import java.security.Principal;
import java.text.MessageFormat;
import java.text.SimpleDateFormat;
import java.util.Locale;
import java.util.Map;
import java.util.ResourceBundle;

/**
 * author: alkhal
 */
public abstract class SmscController implements Serializable {

  private final static org.apache.log4j.Logger logger = org.apache.log4j.Logger.getLogger(SmscController.class);

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
            getLocalizedString(bundleKey), "");
    fc.addMessage("smsc_errors", facesMessage);        
  }

  protected String getLocalizedString(String bundleKey) {
    return ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc", getLocale()).getString(bundleKey);
  }

  protected String getLocalizedString(String bundleKey, Object... args) {
    return MessageFormat.format(
            ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc", getLocale()).getString(bundleKey),
            args
           );

  }

  protected void addLocalizedMessage(FacesMessage.Severity severity, String bundleKey, Object... args) {
    String message = getLocalizedString(bundleKey,args);
    addMessage(severity, message);
  }

  protected void addError(AdminException e) {
    logger.error(e,e);
    addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));
  }

  protected String getRequestParameter(String name) {
    return FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().get(name);
  }

  protected Map<String, String> getRequestParameters() {
    return FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap();
  }

  protected HttpSession getSession(boolean createIfNeeded) {
    return (HttpSession) FacesContext.getCurrentInstance().getExternalContext().getSession(createIfNeeded);
  }

  protected Locale getLocale() {
    return (Locale) FacesContext.getCurrentInstance().getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
  }

  protected Principal getUserPrincipal() {
    return FacesContext.getCurrentInstance().getExternalContext().getUserPrincipal();
  }

  protected String getUserName() {
    Principal p = FacesContext.getCurrentInstance().getExternalContext().getUserPrincipal();
    return p == null ? null : p.getName();
  }

  public boolean isAccessToUsers() {
    return FacesContext.getCurrentInstance().getExternalContext().isUserInRole("users");
  }

  protected Configuration getConfiguration() {
    return WebContext.getInstance().getConfiguration();
  }

  protected SmscStatusManager getSmscStatusManager() {
    return WebContext.getInstance().getSmscStatusManager();
  }

}
