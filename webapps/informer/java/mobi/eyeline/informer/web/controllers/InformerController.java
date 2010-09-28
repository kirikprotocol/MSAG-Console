package mobi.eyeline.informer.web.controllers;


import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.LocaleFilter;
import mobi.eyeline.informer.web.WebContext;
import mobi.eyeline.informer.web.config.Configuration;
import org.apache.myfaces.trinidad.model.UploadedFile;

import javax.faces.application.FacesMessage;
import javax.faces.context.FacesContext;
import javax.faces.event.ValueChangeEvent;
import javax.servlet.http.HttpSession;
import java.io.*;
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
   * @param context Faces context
   * @param out Output Stream
   * @throws java.io.IOException ошибка записи
   */
  public void download(FacesContext context, OutputStream out) throws IOException{
    PrintWriter w = new PrintWriter(new OutputStreamWriter(out, "UTF-8"));
    _download(w);
    w.flush();
  }

  /**
   * Метод для отдачи файлов (требует переопределения в наследнике)
   * @param writer Print writer
   * @throws java.io.IOException ошибка записи
   */
  protected void _download(PrintWriter writer) throws IOException{
  }


  /**
   * Метод для загрузки файла
   * @param e ValueChangeEvent
   */
  @SuppressWarnings({"EmptyCatchBlock"})
  public void uploaded(ValueChangeEvent e) {
    UploadedFile file = (UploadedFile)e.getNewValue();
    if(file == null) {
      return;
    }
    BufferedReader is = null;
    try{
      is = new BufferedReader(new InputStreamReader(file.getInputStream(), "UTF-8"));
      _uploaded(is);
    }catch (IOException ex) {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "informer.upload.error");
    } finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException ex){}
      }
    }
  }

   /**
   * Метод для загрузки файла (требует переопределения в наследнике)
   * @param reader BufferedReader
   * @throws java.io.IOException ошибка чтения
   */
  protected void _uploaded(BufferedReader reader) throws IOException{  
  }

}
