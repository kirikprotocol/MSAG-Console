package mobi.eyeline.informer.web.controllers;


import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.LocaleFilter;
import mobi.eyeline.informer.web.WebContext;
import mobi.eyeline.informer.web.config.Configuration;

import javax.faces.application.FacesMessage;
import javax.faces.context.FacesContext;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.Serializable;
import java.security.Principal;
import java.text.MessageFormat;
import java.util.Locale;
import java.util.Map;
import java.util.ResourceBundle;

/**
 * Базовый контроллер
 * @author Aleksandr Khalitov
 */
public abstract class InformerController implements Serializable {

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
        ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", getLocale()).getString(bundleKey), "");
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
        ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", getLocale()).getString(bundleKey),
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

  /**
   * Метод для отдачи файлов
   * @param fileName имя файла, показываемое пользователю
   * @param contentType content type, к примеру "application/csv"
   * @param outputter интерфейс вывода в файл
   * @throws IOException ошибка записи в пото
   */
  @SuppressWarnings({"EmptyCatchBlock"})
  protected void downloadFile( String fileName, String contentType, DownloadOutputter outputter) throws IOException{
    FacesContext context = FacesContext.getCurrentInstance();
    HttpServletResponse response =(HttpServletResponse) context.getExternalContext().getResponse();
    response.setCharacterEncoding("utf-8");
    response.setContentType(contentType);
    response.setHeader("Content-Disposition", "attachment;filename=\""+fileName+"\"");
    PrintWriter writer = null;
    try{
      writer = response.getWriter();;
      outputter.output(writer);
      writer.flush();
    }finally {
      if(writer != null) {
          writer.close();
      }
    }
    FacesContext.getCurrentInstance().responseComplete();
  }

  /**
   * Интерфейс вывода в файл
   */
  protected static interface DownloadOutputter {
    void output(PrintWriter s) throws IOException;
  }

}
