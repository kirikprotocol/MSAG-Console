package mobi.eyeline.informer.admin.retry_policies;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.util.config.ConfigFileManager;
import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
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

  protected C getSettings() {
    return settings;
  }

  protected Infosme getInfosme() {
    return infosme;
  }


  abstract class BaseManagerWriteExecutor {

    abstract void changeSettings() throws AdminException ;

    abstract void infosmeCommand() throws AdminException ;

    protected final void execute() throws AdminException {
      try{
        lock.writeLock().lock();
        changeSettings();
        File backup = save();
        if(infosme.isOnline()) {
          try {
            infosmeCommand();
          }
          catch (AdminException e){
            rollback(backup);
            throw e;
          }
        }
      }finally {
        lock.writeLock().unlock();
      }
    }
  }


  abstract class BaseManagerReadExecutor<ReturnType> {
        
    abstract ReturnType executeRead() throws AdminException ;

    protected final ReturnType execute() throws AdminException {
      try{
        lock.readLock().lock();
        return executeRead();
      }finally {
        lock.readLock().unlock();
      }
    }
  }
}
