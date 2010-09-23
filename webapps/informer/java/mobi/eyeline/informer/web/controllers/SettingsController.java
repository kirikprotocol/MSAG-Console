package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.AdminException;

import javax.servlet.http.HttpSession;
import java.util.EnumMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Абстрактный контроллер для работы с конфигами, все изменения в которых применяются одной операцией "submit".
 *
 * @author Artem Snopkov
 */
public abstract class SettingsController<T> extends InformerController {

  private static Map<ConfigType, Revision> lastRevisions = new EnumMap<ConfigType, Revision>(ConfigType.class);
  private static Lock submitLock = new ReentrantLock();

  private final String settingsAttr;
  private final String revisionAttr;
  private final ConfigType configType;

  private Integer revision = 0;

  protected SettingsController(ConfigType configType) {
    if (configType == null)
      throw new NullPointerException();

    this.settingsAttr = configType.name() + "_settings";
    this.revisionAttr = configType.name() + "_viewStartTime";
    this.configType = configType;

    if (getRequestParameter(revisionAttr) == null) {
      try {
        resetRevision();
      } catch (AdminException e) {
        e.printStackTrace();
        addError(e);
      }
    }
  }

  private void resetRevision() throws AdminException {
    revision = (Integer) getSessionAttr(revisionAttr);
    if (revision == null) {
      try {
        submitLock.lock();
        Revision rev = lastRevisions.get(configType);
        revision = rev == null ? 0 : rev.number;
      } finally {
        submitLock.unlock();
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
      submitLock.lock();

      Revision rev = lastRevisions.get(configType);
      if (rev != null && rev.getNumber() > revision)
        return rev;

      T settings = getSettings();      
      saveSettings(settings);

      lastRevisions.put(configType, new Revision(getUserPrincipal().getName(), revision + 1));
    } finally {
      submitLock.unlock();
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


  protected enum ConfigType {
    Main, Reschedule, User, MapLimit, Logger, Fraud, Snmp
  }


  /**
   *  Структура, содержащая данные о последней ревизии конфига
   */
  protected static class Revision {
    private String user;
    private int number;

    private Revision(String user, int number) {
      this.user = user;
      this.number = number;
    }

    public String getUser() {
      return user;
    }

    public void setUser(String user) {
      this.user = user;
    }

    public int getNumber() {
      return number;
    }

    public void setNumber(int number) {
      this.number = number;
    }
  }
}
