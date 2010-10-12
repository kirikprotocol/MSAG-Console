package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.util.config.ConfigFileManager;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Управление конфигурацией регионов
 * @author Aleksandr Khalitov
 */
public class RegionsManager {

  private static final Logger logger = Logger.getLogger(RegionsManager.class);

  private Infosme infosme;

  private final ConfigFileManager<RegionsSettings> cfgFileManager;

  private ReadWriteLock lock = new ReentrantReadWriteLock();

  private RegionsSettings settings;


  public RegionsManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException {

    this.infosme = infosme;

    cfgFileManager = new ConfigFileManager<RegionsSettings>(config, backup, fileSystem, new RegionsConfig());
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
   * Добавляет новый регион
   * @param region регион
   * @throws AdminException ошибка сохранения
   */
  public void addRegion(Region region) throws AdminException{
    try{
      lock.writeLock().lock();
      settings.addRegion(region);
      File backup = save();
      if(infosme.isOnline()) {
        try{
          infosme.addRegion(region.getRegionId());
        }catch (AdminException e){
          rollback(backup);
          throw e;
        }
      }
    }finally {
      lock.writeLock().unlock();
    }

  }

  /**
   * Обновляет существующий регион
   * @param region регион
   * @throws AdminException ошибка сохранения
   */
  public void updateRegion(Region region) throws AdminException{
    try{
      lock.writeLock().lock();
      settings.updateRegion(region);
      File backup = save();
      if(infosme.isOnline()) {
        try{
          infosme.updateRegion(region.getRegionId());
        }catch (AdminException e){
          rollback(backup);
          throw e;
        }
      }
    }finally {
      lock.writeLock().unlock();
    }
  }

  /**
   * Возвращает список регионов
   * @return список регионов
   */
  public List<Region> getRegions() {
    try{
      lock.readLock().lock();
      List<Region> result = new LinkedList<Region>();
      for(Region s : settings.getRegions()) {
        result.add(s.cloneRegion());
      }
      return result;
    }finally {
      lock.readLock().unlock();
    }
  }

  /**
   * Возвращает регион по id
   * @param id id региона
   * @return регион
   */
  public Region getRegion(String id){
    try{
      lock.readLock().lock();
      Region region = settings.getRegion(id);
      return region == null ? null : region.cloneRegion();
    }finally {
      lock.readLock().unlock();
    }
  }

  /**
   * Удаляет регион
   * @param regionId id региона
   * @throws AdminException ошибка сохранения
   */
  public void removeRegion(String regionId) throws AdminException{
    try{
      lock.writeLock().lock();
      settings.removeRegion(regionId);
      File backup = save();
      if(infosme.isOnline()) {
        try{
          infosme.removeRegion(regionId);
        }catch (AdminException e){
          rollback(backup);
          throw e;
        }
      }
    }finally {
      lock.writeLock().unlock();
    }
  }

  /**
   * Возвращает регион по маске
   * @param address маска
   * @return регион
   */
  public Region getRegion(Address address) {
    try{
      lock.readLock().lock();
      Region region = settings.getRegionByAddress(address);
      return region == null ? null : region.cloneRegion();
    }finally {
      lock.readLock().unlock();
    }
  }

  /**
   * Возвращает список регионов по названию СМСЦ
   * @param smsc название СМСЦ
   * @return список регионов
   */
  public Collection<Region> getRegionsBySmsc(String smsc) {
    Collection<Region> result = new LinkedList<Region>();
    try{
      lock.readLock().lock();
      for(Region r : settings.getRegionsBySmsc(smsc)) {
        result.add(r.cloneRegion());
      }
      return result;
    }finally {
      lock.readLock().unlock();
    }
  }

}
