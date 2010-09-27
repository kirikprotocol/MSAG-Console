package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.util.config.ConfigFileManager;

import java.io.File;
import java.util.Observable;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Сохранение/чтение настроек Informer
 * @author Aleksandr Khalitov
 */
public class InformerConfigManager extends Observable{

  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  private final ConfigFileManager<InformerSettings> cfgFileManager;

  /**
   * Конструктор
   * @param configFile файл с конфигурацией
   * @param backupDir директория для бэкапа
   * @param fs файловая система
   */
  public InformerConfigManager(File configFile, File backupDir, FileSystem fs) {
    cfgFileManager = new ConfigFileManager<InformerSettings>(configFile, backupDir, fs, new InformerConfig());
  }

  /**
   * Возвращает настройки Informer
   * @return настройки Informer
   * @throws AdminException ошибка чтения настроек
   */
  public InformerSettings getConfigSettings() throws AdminException {
    try{
      lock.readLock().lock();
      return cfgFileManager.load();
    }finally {
      lock.readLock().unlock();
    }
  }

  /**
   * Сохраняет настроки
   * @param informerSettings настройки
   * @throws AdminException ошибка сохранения
   */
  public void updateSettings(InformerSettings informerSettings) throws AdminException{
    try{
      lock.writeLock().lock();
      cfgFileManager.save(informerSettings);
      setChanged();
      try{
        notifyObservers(informerSettings);
      }catch (Exception e){
        e.printStackTrace();
      }
    }finally {
      lock.writeLock().unlock();
    }

  }
}
