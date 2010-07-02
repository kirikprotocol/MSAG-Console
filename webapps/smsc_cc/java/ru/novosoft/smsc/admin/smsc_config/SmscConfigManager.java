package ru.novosoft.smsc.admin.smsc_config;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Класс, управляющий конфигурационным файлом СМСЦ. Использует SmscConfig для чтения/записи конфига.
 *
 * @author Artem Snopkov
 */
@SuppressWarnings({"EmptyCatchBlock"})
public class SmscConfigManager {     //todo return copies

  private static final Logger logger = Logger.getLogger(SmscConfigManager.class);

  private File configFile;

  private File backupDir;

  private static SmscConfigManager instance;

  private static final Lock initLock = new ReentrantLock();

  private static final String SERVICE_NAME = "SMSC1";

  private SmscConfig config;

  private boolean changed;

  private Set<SmscConfigListener> listeners = new HashSet<SmscConfigListener>();

  public static SmscConfigManager getInstance() throws AdminException {
    if (instance == null) {
      try {
        initLock.lock();
        if (instance == null) {
          ServiceInfo si = ServiceManager.getInstance().getService(SERVICE_NAME);
          if (si == null) {
            throw new AdminException("SMSC not found.");
          }
          File conf = new File(si.getBaseDir(), "conf" + File.separatorChar + "config.xml");
          File backupDir = new File(si.getBaseDir(), "conf" + File.separatorChar + "backup");
          instance = new SmscConfigManager(conf, backupDir);
        }
      } finally {
        initLock.unlock();
      }
    }
    return instance;
  }

  /**
   * @param configFile конфигурациооный файл
   * @param backupDir  директория для бэкапов
   * @throws AdminException ошибка при чтении конфига
   */
  private SmscConfigManager(File configFile, File backupDir) throws AdminException {
    this.configFile = configFile;
    this.backupDir = backupDir;
    this.config = new SmscConfig();
    InputStream is = null;
    try {
      is = FileSystem.getInstance().getInputStream(this.configFile);
      this.config.load(is);
    } finally {
      if (is != null) {
        try {
          is.close();
        } catch (IOException e) {
        }
      }
    }
  }

  /**
   * Возвращает настройки, общие для всех СМСЦ.
   *
   * @return настройки, общие для всех СМСЦ.
   * @throws AdminException ошибка извлечения настроек
   */
  public CommonSettings getCommonSettings() throws AdminException {
    try {
      return (CommonSettings) config.getCommonSettings().clone();
    } catch (CloneNotSupportedException e) {
      logger.error(e, e);
      throw new AdminException(e.getMessage());
    }
  }

  public void setCommonSettings(CommonSettings settings) throws AdminException {
    for (SmscConfigListener l : listeners) {
      l.setCommonSettings(settings);
    }
    config.setCommonSettings(settings);
    changed = true;
  }

  public void setIntanceSettings(InstanceSettings settings) throws AdminException {
    for (SmscConfigListener l : listeners) {
      l.setInstanceSettings(settings);
    }
    config.setInstanceSettings(settings);
    changed = true;
  }

  /**
   * Возвращает количество инстанцев СМСЦ
   *
   * @return количество инстанцев СМСЦ
   */
  public int getSmscInstancesCount() {
    return config.getSmscInstancesCount();
  }

  /**
   * Возвращает настройки, специфические для каждого инстанца СМСЦ
   *
   * @param instanceNumber номер инстанца
   * @return настройки, специфические для каждого инстанца СМСЦ
   * @throws AdminException ошибка извлечения настроек
   */
  public InstanceSettings getInstanceSettings(int instanceNumber) throws AdminException {
    try {
      return (InstanceSettings) config.getInstanceSettings(instanceNumber).clone();
    } catch (CloneNotSupportedException e) {
      logger.error(e, e);
      throw new AdminException(e.getMessage());
    }
  }

  private final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yy.HH.mm.ss");

  /**
   * Сохраняет и применяет изменения, сделанные в конфиге
   *
   * @throws AdminException ошибка при сохранении конфига
   */
  public synchronized void apply() throws AdminException {
    for (SmscConfigListener l : listeners) {
      l.applySettings(config.getCommonSettings(), config.getAllInstanceSettings());
    }
    OutputStream os = null;
    try {
      os = FileSystem.getInstance().getOutputStream(new File(backupDir, "config.xml." + sdf.format(new Date())));
      this.config.backup(os);
    } finally {
      if (os != null) {
        try {
          os.close();
        } catch (IOException e) {
        }
      }
    }
    File tmp = new File(configFile.getAbsolutePath() + ".tmp");
    os = null;
    try {
      os = FileSystem.getInstance().getOutputStream(tmp);
      this.config.save(os);
    } finally {
      if (os != null) {
        try {
          os.close();
        } catch (IOException e) {
        }
      }
    }
    FileSystem.getInstance().rename(tmp, configFile);
    changed = false;
  }

  /**
   * Откатывает изменения, сделанные в конфиге
   *
   * @throws AdminException ошибка при чтении конфига
   */
  public synchronized void reset() throws AdminException {
    SmscConfig oldConfig = new SmscConfig();
    InputStream is = null;
    try {
      is = FileSystem.getInstance().getInputStream(this.configFile);
      oldConfig.load(is);
    } finally {
      if (is != null) {
        try {
          is.close();
        } catch (IOException e) {
        }
      }
    }
    for (SmscConfigListener l : listeners) {
      l.resetSettings(oldConfig.getCommonSettings(), oldConfig.getAllInstanceSettings());
    }
    this.config = oldConfig;
    changed = false;
  }

  /**
   * Метод возвращает true, если конфиг менялся, false - иначе
   *
   * @return true - конфиг менялся, false - иначе
   */
  public boolean isChanged() {
    return changed;
  }

  /**
   * Добавляет слушателя конфига
   *
   * @param l слушатель
   */
  public void addListener(SmscConfigListener l) {
    listeners.add(l);
  }

  /**
   * Удаляет слушателя конфига
   *
   * @param l слушатель
   */
  public void removeListener(SmscConfigListener l) {
    listeners.remove(l);
  }
}
