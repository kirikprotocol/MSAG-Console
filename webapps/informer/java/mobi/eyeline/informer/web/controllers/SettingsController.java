package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.config.Revision;

import javax.servlet.http.HttpSession;

/**
 * Абстрактный контроллер для работы с конфигами, все изменения в которых применяются одной операцией "submit".
 *
 * @author Artem Snopkov
 */
public abstract class SettingsController<T> extends InformerController {

  private String settingsAttr;
  private String revisionAttr;

  private Integer revision = 0;

  private Configuration.ConfigType configType;

  protected Configuration configuration;

  protected void init(Configuration.ConfigType configType) throws AdminException{
    configuration = getConfig();
    this.configType = configType;
    this.settingsAttr = configType + "_settings";
    this.revisionAttr = configType + "_viewStartTime";
    if (getRequestParameter(revisionAttr) == null) {
      resetRevision();
    }
  }

  private void resetRevision() throws AdminException {
    revision = (Integer) getSessionAttr(revisionAttr);
    if (revision == null) {
      try {
        configuration.lock();
        Revision rev = configuration.getLastRevision(configType);
        revision = rev == null ? 0 : rev.getNumber();
      } finally {
        configuration.unlock();
      }

      setSessionAttr(settingsAttr, loadSettings());
    }
  }

  public int getRevision() {
    return revision;
  }

  public void setRevision(int revision) {
    this.revision = revision;
  }

  private Object getSessionAttr(String name) {
    return getSession(true).getAttribute(name);
  }

  private void setSessionAttr(String name, Object value) {
    getSession(true).setAttribute(name, value);
  }

  /**
   * Загружает конфиг
   *
   * @return новый инстанц конфига
   * @throws AdminException если произошла ошибка
   */
  protected abstract T loadSettings() throws AdminException;

  /**
   * Сохраняет конфиг
   *
   * @param settings конфиг, который надо сохранить
   * @throws AdminException если произошла ошибка
   */
  protected abstract void saveSettings(T settings) throws AdminException;

  /**
   * Создает копию конфига
   * @param settings конфиг, который надо скопировать
   * @return копия
   */
  protected abstract T cloneSettings(T settings);

  /**
   * Задает новую версию конфига для данного пользователя
   * @param settings новая версия  конфига для данного пользователя
   */
  protected void setSettings(T settings) {
    if (settings == null)
      throw new NullPointerException();

    setSessionAttr(settingsAttr, settings);
    setSessionAttr(revisionAttr, revision);
  }

  /**
   * Возвращает текущую версию конфига для данного пользователя
   * @return текущую версию конфига для данного пользователя
   */
  @SuppressWarnings({"unchecked"})
  protected T getSettings() {
    return cloneSettings((T) getSessionAttr(settingsAttr));
  }

  private void clearSession() {
    HttpSession s = getSession(true);
    s.removeAttribute(settingsAttr);
    s.removeAttribute(revisionAttr);
  }

  /**
   * Расшаривает изменения, сделанные пользователем
   * @return null, если изменения успешно разшарены. Если же произошел конфиг с изменениями, сделанными другим пользователем,
   * то вернется объект Revision, содержащий данные о последнем изменении конфига.
   *
   * @throws AdminException если произошла ошибка
   */
  protected Revision submitSettings() throws AdminException {
    try {
      configuration.lock();

      Revision rev = configuration.getLastRevision(configType);
      if (rev != null && rev.getNumber() > revision)
        return rev;

      T settings = getSettings();
      saveSettings(settings);

    } finally {
      configuration.unlock();
    }
    clearSession();
    return null;
  }

  /**
   * Откатывает все изменения, сделанные пользователем
   * @throws AdminException если произошла ошибка
   */
  protected void resetSettings() throws AdminException {
    clearSession();
    resetRevision();
  }

  /**
   * Возвращает true, если пользователь произвел ккакие-то изменения в конфиге
   * @return  true, если пользователь произвел ккакие-то изменения в конфиге
   */
  protected boolean isSettingsChanged() {
    return getSessionAttr(revisionAttr) != null;
  }


}
