package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.web.config.SettingsManager;

import javax.servlet.http.HttpSession;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Artem Snopkov
 */
public class SettingsMController<T> extends SmscController {

  private static Lock submitLock = new ReentrantLock();

  private final String settingsAttr;
  private final String revisionAttr;

  private Long revision = 0L;
  private final SettingsManager<T> mngr;

  protected SettingsMController(SettingsManager<T> mngr) {
    this.mngr = mngr;
    this.settingsAttr = getClass().getCanonicalName() + "_settings";
    this.revisionAttr = getClass().getCanonicalName() + "_viewStartTime";

  }

  protected void init() throws AdminException {
    resetRevision();
  }

  private void resetRevision() throws AdminException {
    revision = (Long) getSessionAttr(revisionAttr);
    if (revision == null) {
      try {
        submitLock.lock();
        revision = mngr.getLastUpdateTime();
      } finally {
        submitLock.unlock();
      }

      setSessionAttr(settingsAttr, loadSettings());
    }
  }

  public long getRevision() {
    return revision;
  }

  public void setRevision(long revision) {
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
  private T loadSettings() throws AdminException {
    return mngr.getSettings();
  }

  /**
   * Сохраняет конфиг
   *
   * @param settings конфиг, который надо сохранить
   * @throws AdminException если произошла ошибка
   */
  private void saveSettings(T settings) throws AdminException {
    mngr.updateSettings(settings);
  }

  /**
   * Задает новую версию конфига для данного пользователя
   *
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
   *
   * @return текущую версию конфига для данного пользователя
   */
  protected T getSettings() {
    return mngr.cloneSettings((T) getSessionAttr(settingsAttr));
  }

  private void clearSession() {
    HttpSession s = getSession(true);
    s.removeAttribute(settingsAttr);
    s.removeAttribute(revisionAttr);
  }

  /**
   * Расшаривает изменения, сделанные пользователем
   *
   * @throws AdminException если произошла ошибка
   */
  protected void submitSettings() throws AdminException {
    try {
      submitLock.lock();

      long lastRevision = mngr.getLastUpdateTime();

      if (lastRevision != revision)
        throw new ConcurrentModificationException(mngr.getLastUpdateUser(), lastRevision);

      T settings = getSettings();
      saveSettings(settings);

    } finally {
      submitLock.unlock();
    }
    clearSession();
  }

  /**
   * Откатывает все изменения, сделанные пользователем
   *
   * @throws AdminException если произошла ошибка
   */
  protected void resetSettings() throws AdminException {
    clearSession();
    resetRevision();
  }

  /**
   * Возвращает true, если пользователь произвел ккакие-то изменения в конфиге
   *
   * @return true, если пользователь произвел ккакие-то изменения в конфиге
   */
  protected boolean isSettingsChanged() {
    return getSessionAttr(revisionAttr) != null;
  }

}
