package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.service.ServiceInfo;
import mobi.eyeline.informer.admin.service.ServiceManager;
import mobi.eyeline.informer.admin.util.config.ConfigFileManager;

import java.io.File;
import java.util.List;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * @author Aleksandr Khalitov
 */
public class ArchiveDaemonManager {

  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  private final ConfigFileManager<ArchiveDaemonSettings> cfgFileManager;

  private final ServiceManager serviceManager;

  private final static String SERVICE_ID = "ArchiveDaemon";


  public ArchiveDaemonManager(File configFile, File backupDir, FileSystem fs, ServiceManager serviceManager) {
    cfgFileManager = new ConfigFileManager<ArchiveDaemonSettings>(configFile, backupDir, fs, new ArchiveDaemonConfig());
    this.serviceManager = serviceManager;
  }

  
  /**
   * Возвращает настройки ArchiveDaemon
   *
   * @return настройки ArchiveDaemon
   * @throws mobi.eyeline.informer.admin.AdminException ошибка чтения настроек
   */
  public ArchiveDaemonSettings getSettings() throws AdminException {
    try {
      lock.readLock().lock();
      return cfgFileManager.load();
    } finally {
      lock.readLock().unlock();
    }
  }

  /**
   * Сохраняет настроки
   *
   * @param archiveSettings настройки
   * @throws AdminException ошибка сохранения
   */
  public void updateDaemonSettings(ArchiveDaemonSettings archiveSettings) throws AdminException{
    archiveSettings.validate();
    try {
      lock.writeLock().lock();
      cfgFileManager.save(archiveSettings);
    } finally {
      lock.writeLock().unlock();
    }
  }


  /**
   * Запуск ArchiveDaemon
   *
   * @throws AdminException ошибка при запуске
   */
  public void startArchiveDaemon() throws AdminException {
    serviceManager.startService(SERVICE_ID);
  }

  /**
   * Остановка ArchiveDaemon
   *
   * @throws AdminException ошибка при остановке
   */
  public void stopArchiveDaemon() throws AdminException {
    serviceManager.stopService(SERVICE_ID);
  }

  /**
   * Переключение ArchiveDaemon на другую ноду
   *
   * @param toHost хост
   * @throws AdminException ошибка при переключении
   */
  public void switchArchiveDaemon(String toHost) throws AdminException {
    serviceManager.swichService(SERVICE_ID, toHost);
  }

  /**
   * Возвращает хост, на котором запущен ArchiveDaemon
   *
   * @return хост
   * @throws AdminException ошибка чтения статуса ArchiveDaemon
   */
  public String getArchiveDaemonOnlineHost() throws AdminException {
    return getInfo().getOnlineHost();
  }

  /**
   * Возвращает список всех нод, на которых может быть запущен ArchiveDaemon
   *
   * @return список нод
   * @throws AdminException ошибка чтения статуса ArchiveDaemon
   */
  public List<String> getArchiveDaemonHosts() throws AdminException {
    return getInfo().getHosts();
  }



  private ServiceInfo getInfo() throws AdminException {
    ServiceInfo si = serviceManager.getService(SERVICE_ID);
    if (si == null)
      throw new ArchiveException("service_manager_offline");
    return si;
  }
}
