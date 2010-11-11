package mobi.eyeline.informer.admin.util.config;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.10.2010
 * Time: 18:44:32
 */
public class BaseManager<C> {
  private static final Logger logger = Logger.getLogger(BaseManager.class);

  private ReadWriteLock lock = new ReentrantReadWriteLock();
  private C settings;
  private Infosme infosme;
  private final ConfigFileManager<C> cfgFileManager;

  public BaseManager(Infosme infosme, File config, File backup, FileSystem fileSystem, ManagedConfigFile<C> cfgFileImpl) throws InitException {

    this.infosme = infosme;
    cfgFileManager = new ConfigFileManager<C>(config, backup, fileSystem,  cfgFileImpl  );
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

  protected void updateSettings(SettingsWriter<C> e) throws AdminException {
    try{
      lock.writeLock().lock();
      e.changeSettings(settings);
      File backup = save();
      if(infosme!=null && infosme.isOnline()) {
        try {
          e.infosmeCommand(infosme);
        }
        catch (AdminException ex){
          rollback(backup);
          throw ex;
        }
      }
    }finally {
      lock.writeLock().unlock();
    }
  }

  protected <R> R readSettings(SettingsReader<C,R> e)  {
    try{
      lock.readLock().lock();
      return e.executeRead(settings);
    }finally {
      lock.readLock().unlock();
    }
  }

}
