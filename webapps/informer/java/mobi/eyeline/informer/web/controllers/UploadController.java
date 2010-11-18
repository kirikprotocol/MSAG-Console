package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.LocalizedException;
import org.apache.log4j.Logger;
import org.apache.myfaces.trinidad.model.UploadedFile;

import javax.faces.context.FacesContext;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.ResourceBundle;

/**
 * Контроллер для загрузки файлов на сервер в асинхронном режиме
 * @author Aleksandr Khalitov
 */
public abstract class UploadController extends InformerController{

  protected static final Logger logger = Logger.getLogger(UploadController.class);

  private int state = 0;

  private UploadedFile file;

  protected UploadThread thread;

  private String error;

  /**
   * Возвращает текст ощибки
   * @return текст ощибки
   */
  public String getError() {
    return error;
  }

  /**
   * Возвращает текущее значение прогресса обработки файла (для отображения шкалы)
   * @return текущее значение прогресса
   */
  public abstract int getCurrent();

  /**
   * Возвращает максимальное значение прогресса обработки файла (для отображения шкалы)
   * @return максимальное значение прогресса
   */
  public abstract int getMaximum();

  /**
   * Действие выполняется при выходе со страницы загрузки
   * @return navigation rule's action
   */
  protected abstract String _next();

  /**
   * Обработка загруженного файла
   * @param file файл
   * @param user залогиненный пользователь
   * @param requestParams параметры запроса
   * @throws Exception ошибка обработки. Если возникает AdminException, то показывается её локализованное сообщение.
   * При возникновении IllegalArgumentException показывается сообщение "Неверный формат файл".
   * В остальный случаях - "Внутрення ошибка"
   */
  protected abstract void _process(UploadedFile file, String user, Map<String, String> requestParams) throws Exception;


  public UploadedFile getFile() {
    return file;
  }

  public void setFile(UploadedFile file) {
    this.file = file;
  }

  /**
   * Возвращает флаг, Была ли прервана обработка файла пользователем (метод должен вызываться при обработки файл)
   * @return true - да, false - нет
   */
  protected boolean isStoped() {
    return thread != null && thread.stop;
  }


  public int getState() {
    return state;
  }


  /**
   * Action, вызываемый при начале обработки файла
   * @return null
   */
  public String upload(){
    if(file != null) {
      thread = new UploadThread(file, getUserName(), getLocale(), FacesContext.getCurrentInstance());
      thread.start();
      state=1;
    }
    return null;
  }

  /**
   * Action, вызываемый при уходе со страницы загрузки
   * @return navifgation rule's action
   */
  public String next() {
    error = null;
    state = 0;
    file = null;
    return _next();
  }

  /**
   * Action, вызываемый при остановки обработки файла
   * @return тnull
   */
  public String stop() {
    if(thread != null) {
      thread.stop = true;
    }
    return null;
  }

  /**
   * Возвращает флаг, завершился ли поток обработки файла
   * @return true -да, false - нет, файл обрабатывается
   */
  public boolean isFinished() {
    return thread != null && thread.isFinished();
  }


  private class UploadThread extends Thread {

    private UploadedFile file;

    private boolean finished = false;

    protected final String user;

    protected final Locale locale;

    private boolean stop = false;

    private Map<String, String> requestParams;

    private UploadThread(UploadedFile file, String user, Locale locale, FacesContext context) {
      this.file = file;
      this.user = user;
      this.locale = locale;
      this.requestParams = new HashMap<String, String>(context.getExternalContext().getRequestParameterMap());
    }

    @Override
    public void run() {
      try{
        _process(file, user, requestParams);
      }catch (LocalizedException e){
        error = e.getMessage(locale);
        logger.warn(e,e);
      }catch (IllegalArgumentException e){
        error = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("upload.illegal.file");
        logger.warn(e,e);
      } catch (Exception e){
        error = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("upload.error.text");
        logger.error(e,e);
      }finally {
        finished = true;
      }
    }

    private boolean isFinished() {
      return finished;
    }
  }
}
