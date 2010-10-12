package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.util.config.ConfigFileManager;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Управление СМС-Центрами
 * @author Aleksandr Khalitov
 */
public class SmscManager {

  private static final Logger logger = Logger.getLogger(SmscManager.class);

  private Infosme infosme;

  private final ConfigFileManager<SmscSettings> cfgFileManager;

  private ReadWriteLock lock = new ReentrantReadWriteLock();

  private SmscSettings settings;



  public SmscManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException{

    this.infosme = infosme;

    cfgFileManager = new ConfigFileManager<SmscSettings>(config, backup, fileSystem, new SmscConfig());
    try{
      this.settings = cfgFileManager.load();
    }catch (AdminException e){
      throw new InitException(e);
    }
  }


  private File save() throws AdminException {
    return cfgFileManager.save(settings);
  }

  private void rollback(File backupFile) {
    try{
      if(cfgFileManager.rollback(backupFile)) {
        settings = cfgFileManager.load();
      }else {
        logger.error("Can't rollback config file");
      }
    }catch (Exception ex){
      logger.error(ex,ex);
    }
  }

  /**
   * Добавляет новый СМСЦ
   * @param smsc СМСЦ
   * @throws AdminException ошибка сохранения
   */
  public void addSmsc(Smsc smsc) throws AdminException{
    try{
      lock.writeLock().lock();
      settings.addSmsc(smsc);
      File backup = save();
      if(infosme.isOnline()) {
        try{
          infosme.addSmsc(smsc.getName());
        }catch (AdminException e) {
          rollback(backup);
          throw e;
        }
      }
    }finally {
      lock.writeLock().unlock();
    }

  }

  /**
   * Обновляет существующий СМСЦ
   * @param smsc СМСЦ
   * @throws AdminException ошибка сохранения
   */
  public void updateSmsc(Smsc smsc) throws AdminException{
    try{
      lock.writeLock().lock();
      settings.updateSmsc(smsc);
      File backup = save();
      if(infosme.isOnline()) {
        try{
          infosme.updateSmsc(smsc.getName());
        }catch (AdminException e) {
          rollback(backup);
          throw e;
        }
      }
    }finally {
      lock.writeLock().unlock();
    }
  }

  /**
   * Возвращает список СМСЦ
   * @return список СМСЦ
   */
  public List<Smsc> getSmscs() {
    try{
      lock.readLock().lock();
      List<Smsc> result = new LinkedList<Smsc>();
      for(Smsc s : settings.getSmscs()) {
        result.add(s.cloneSmsc());
      }
      return result;
    }finally {
      lock.readLock().unlock();
    }
  }

  /**
   * ВОзвращает СМСЦ имени
   * @param name имя СМСЦ
   * @return СМСЦ
   */
  public Smsc getSmsc(String name){
    try{
      lock.readLock().lock();
      Smsc smsc = settings.getSmsc(name);
      return smsc == null ? null : smsc.cloneSmsc();
    }finally {
      lock.readLock().unlock();
    }
  }

  /**
   * Удаляет СМСЦ
   * @param smscName имя СМСЦ
   * @throws AdminException ошибка сохранения
   */
  public void removeSmsc(String smscName) throws AdminException{
    try{
      lock.writeLock().lock();
      settings.removeSmsc(smscName);
      File backup = save();
      if(infosme.isOnline()) {
        try{
          infosme.removeSmsc(smscName);
        }catch (AdminException e) {
          rollback(backup);
          throw e;
        }
      }
    }finally {
      lock.writeLock().unlock();
    }
  }

  /**
   * Устанавливает СМСЦ по умолчанию
   * @param smsc имя СМСЦ
   * @throws AdminException ошибка сохранения
   */
  public void setDefaultSmsc(String smsc) throws AdminException {
    try{
      lock.writeLock().lock();
      settings.setDefaultSmsc(smsc);
      File backup = save();
      if(infosme.isOnline()) {
        try{
          infosme.setDefaultSmsc(smsc);
        }catch (AdminException e) {
          rollback(backup);
          throw e;
        }
      }
    }finally {
      lock.writeLock().unlock();
    }
  }

  /**
   * Возвращает СМСЦ по умолчанию
   * @return СМСЦ по умолчанию
   */
  public String getDefaultSmsc() {
    try{
      lock.writeLock().lock();
      return settings.getDefaultSmsc();
    }finally {
      lock.writeLock().unlock();
    }
  }

}
