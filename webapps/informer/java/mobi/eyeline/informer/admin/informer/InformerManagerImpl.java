package mobi.eyeline.informer.admin.informer;

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
public class InformerManagerImpl implements InformerManager{

  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  private final ConfigFileManager<InformerSettings> cfgFileManager;

  private final ServiceManager serviceManager;

  private final static String SERVICE_ID = "Informer";

  /**
   * Конструктор
   * @param configFile файл с конфигурацией
   * @param backupDir директория для бэкапа
   * @param fs файловая система
   * @param serviceManager service manager
   */
  public InformerManagerImpl(File configFile, File backupDir, FileSystem fs, ServiceManager serviceManager) {
    cfgFileManager = new ConfigFileManager<InformerSettings>(configFile, backupDir, fs, new InformerConfig());
    this.serviceManager = serviceManager;
  }


  public InformerSettings getConfigSettings() throws AdminException {
    try{
      lock.readLock().lock();
      return cfgFileManager.load();
    }finally {
      lock.readLock().unlock();
    }
  }


  public void updateSettings(InformerSettings informerSettings) throws AdminException{
    try{
      lock.writeLock().lock();
      cfgFileManager.save(informerSettings);
    }finally {
      lock.writeLock().unlock();
    }

  }

  public void startInformer() throws AdminException {
    serviceManager.startService(SERVICE_ID);
  }

  public void stopInformer() throws AdminException {
    serviceManager.stopService(SERVICE_ID);
  }

  public void switchInformer(String toHost) throws AdminException {
    serviceManager.swichService(SERVICE_ID, toHost);
  }

  public String getInformerOnlineHost() throws AdminException {
    return getInfo().getOnlineHost();
  }

  public List<String> getInformerHosts() throws AdminException {
    return getInfo().getHosts();
  }

  private ServiceInfo getInfo() throws AdminException {
    ServiceInfo si = serviceManager.getService(SERVICE_ID);
    if (si == null)
      throw new InformerException("service_manager_offline");
    return si;
  }
}
