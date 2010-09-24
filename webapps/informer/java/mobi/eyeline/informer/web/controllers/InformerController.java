package mobi.eyeline.informer.web.controllers;


import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.WebContext;
import mobi.eyeline.informer.web.LocaleFilter;
import mobi.eyeline.informer.web.config.Configuration;

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
 * Базовый контроллер
 * @author Aleksandr Khalitov
 */
public abstract class InformerController implements Serializable {

  final static SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

  protected Configuration getConfiguration() {
    return WebContext.getInstance().getConfiguration();
  }

  /**
   * Добавляет сообщение на страницу
   * @param severity severiry
   * @param message сообщение
   */
  protected void addMessage(FacesMessage.Severity severity, String message) {
    addMessage(severity, message, "");
  }

  /**
   * Добавляет сообщение на страницу
   * @param severity severiry
   * @param message сообщение
   * @param detail подробности
   */
  protected void addMessage(FacesMessage.Severity severity, String message, String detail) {
    FacesContext fc = FacesContext.getCurrentInstance();
    FacesMessage facesMessage = new FacesMessage(severity, message, detail);
    fc.addMessage("informer_errors", facesMessage);
  }

  /**
   * Добавляет локализованное сообщение на страницу
   * @param severity severiry
   * @param bundleKey ключ сообщения в ResourceBundle-е
   */
  protected void addLocalizedMessage(FacesMessage.Severity severity, String bundleKey) {
    FacesContext fc = FacesContext.getCurrentInstance();

    FacesMessage facesMessage = new FacesMessage(severity,
        ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Smsc", getLocale()).getString(bundleKey), "");
    fc.addMessage("informer_errors", facesMessage);
  }
  /**
   * Добавляет локализованное сообщение на страницу с дополнительными параметрами шаблона
   * @param severity severiry
   * @param bundleKey ключ шаблона в ResourceBundle-е
   * @param args параметры шаблона
   */
  protected void addLocalizedMessage(FacesMessage.Severity severity, String bundleKey, Object... args) {
    String message = MessageFormat.format(
        ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Smsc", getLocale()).getString(bundleKey),
        args);
    addMessage(severity, message);
  }

  /**
   * Добавляет на страницу информацию об ошибке
   * @param e ошибка
   */
  protected void addError(AdminException e) {
    addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));
  }

  /**
   * Возвращает параметр из запроса
   * @param name имя параметра
   * @return параметр из запроса
   */
  protected String getRequestParameter(String name) {
    return FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().get(name);
  }

  /**
   * Возвращает все параметры запроса
   * @return параметры запроса
   */
  protected Map<String, String> getRequestParameters() {
    return FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap();
  }

  /**
   * Возвращает текущую сессию пользователя
   * @param createIfNeeded создать, если сессии не существует
   * @return сессия
   */
  protected HttpSession getSession(boolean createIfNeeded) {
    return (HttpSession) FacesContext.getCurrentInstance().getExternalContext().getSession(createIfNeeded);
  }

  /**
   * Возвращает текущую локаль
   * @return локаль
   */
  protected Locale getLocale() {
    return (Locale) FacesContext.getCurrentInstance().getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
  }

  /**
   * Возвращает залогиненного пользвателя
   * @return залогиненный пользователь
   */
  protected Principal getUserPrincipal() {
    return FacesContext.getCurrentInstance().getExternalContext().getUserPrincipal();
  }

  /**
   * Возвращает имя залогиненного пользвателя
   * @return имяx залогиненный пользователь
   */
  public String getUserName() {
    Principal p = FacesContext.getCurrentInstance().getExternalContext().getUserPrincipal();
    return p == null ? null : p.getName();
  }

}
