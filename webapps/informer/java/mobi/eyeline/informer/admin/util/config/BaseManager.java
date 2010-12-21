package mobi.eyeline.informer.admin.util.config;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.OfflineException;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Базовый менеджер настроек. Выполняет 2 функции:
 * 1. Синхронизирует доступ к настройкам.
 * 2. Описывает алгоритм сохранения и применения настроек.
 *
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.10.2010
 * Time: 18:44:32
 */
public class BaseManager<C> {
  private static final Logger logger = Logger.getLogger(BaseManager.class);

  private final ReadWriteLock lock = new ReentrantReadWriteLock();
  private C settings;
  private final Infosme infosme;
  private final ConfigFileManager<C> cfgFileManager;

  public BaseManager(Infosme infosme, File config, File backup, FileSystem fileSystem, ManagedConfigFile<C> cfgFileImpl) throws InitException {

    this.infosme = infosme;
    cfgFileManager = new ConfigFileManager<C>(config, backup, fileSystem, cfgFileImpl);
    try {
      this.settings = cfgFileManager.load();
    } catch (AdminException e) {
      throw new InitException(e);
    }
  }

  private File save() throws AdminException {
    return cfgFileManager.save(settings);
  }

  private void rollback(File backupFile) {
    try {
      if (cfgFileManager.rollback(backupFile)) {
        settings = cfgFileManager.load();
      } else {
        logger.error("Can't rollback config file");
      }
    } catch (Exception ex) {
      logger.error(ex, ex);
    }
  }

  protected void updateSettings(SettingsWriter<C> e) throws AdminException {
    try {
      lock.writeLock().lock();
      e.changeSettings(settings);
      File backup = save();
        try {
          e.infosmeCommand(infosme);
        }
        catch (OfflineException ex) {
          // nothing to do
        }
        catch (AdminException ex){
          rollback(backup);
          throw ex;
        }

    } finally {
      lock.writeLock().unlock();
    }
  }

  protected <R> R readSettings(SettingsReader<C, R> e) {
    try {
      lock.readLock().lock();
      return e.executeRead(settings);
    } finally {
      lock.readLock().unlock();
    }
  }

}
