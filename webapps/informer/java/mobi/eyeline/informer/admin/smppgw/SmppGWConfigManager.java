package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.util.config.ConfigFileManager;

import java.io.File;
import java.util.Iterator;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWConfigManager {

  private SmppGWProviderSettings providerSettings;
  private SmppGWEndpointSettings endpointSettings;

  private final SmppGW smppGW;

  private final ConfigFileManager<SmppGWProviderSettings> providerCfgManager;
  private final ConfigFileManager<SmppGWEndpointSettings> endpointCfgManager;

  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  private final SmppGWConfigManagerContext context;

  public SmppGWConfigManager(SmppGW smppGW, File config, File backup, SmppGWConfigManagerContext context) throws InitException {
    this.smppGW = smppGW;
    this.context = context;
    this.providerCfgManager = new ConfigFileManager<SmppGWProviderSettings>(new File(config,"deliveries.xml"), backup, context.getFileSystem(), new SmppGWProviderConfig(context));
    this.endpointCfgManager = new ConfigFileManager<SmppGWEndpointSettings>(new File(config,"endpoints.xml"), backup, context.getFileSystem(), new SmppGWEndpointConfig());
    try {
      providerSettings = providerCfgManager.load();
      endpointSettings = endpointCfgManager.load();
    } catch (AdminException e) {
      throw new InitException(e);
    }
  }

  private void save() throws AdminException {
    endpointCfgManager.save(endpointSettings);
    providerCfgManager.save(providerSettings);
  }

  private void validate(SmppGWProviderSettings providerSettings, SmppGWEndpointSettings endpointSettings) throws AdminException {
    endpointSettings.validate();
    providerSettings.validate();
    for(SmppGWProvider p : providerSettings.getProviders()) {
      Iterator<String> i = p.getEndpoints().iterator();
      while(i.hasNext()) {
        String e = i.next();
        if(!endpointSettings.containsEndpoint(e)) {   // Проверяем, существует ли соединение, указанное в КП
          i.remove();
        }
      }
      for(SmppGWRoute r : p.getRoutes()) {
        if(context.getUser(r.getUser()) == null) {
          throw new SmppGWException("user_doesnt_exist", r.getUser());
        }
        if(!context.containsDelivery(r.getUser(), r.getDeliveryId()))  {
          throw new SmppGWException("delivery_doesnt_exist", Integer.toString(r.getDeliveryId()));
        }
      }
    }
  }

  public void updateSettings(SmppGWProviderSettings providerSettings, SmppGWEndpointSettings endpointSettings) throws AdminException {
    validate(providerSettings, endpointSettings);
    try {
      lock.writeLock().lock();
      this.providerSettings = providerSettings;
      this.endpointSettings = endpointSettings;
      save();
      smppGW.updateConfig();
    } finally {
      lock.writeLock().unlock();
    }
  }

  public void updateSettings(SmppGWProviderSettings providerSettings) throws AdminException {
    validate(providerSettings, endpointSettings);
    try {
      lock.writeLock().lock();
      this.providerSettings = providerSettings;
      save();
      smppGW.updateConfig();
    } finally {
      lock.writeLock().unlock();
    }
  }

  public void updateSettings() throws AdminException {
    try {
      lock.writeLock().lock();
      save();
      smppGW.updateConfig();
    } finally {
      lock.writeLock().unlock();
    }
  }

  public void updateSettings(SmppGWEndpointSettings endpointSettings) throws AdminException {
    validate(providerSettings, endpointSettings);
    try {
      lock.writeLock().lock();
      this.endpointSettings = endpointSettings;
      save();
      smppGW.updateConfig();
    } finally {
      lock.writeLock().unlock();
    }
  }

  public SmppGWProviderSettings getProviderSettings() {
    try {
      lock.readLock().lock();
      return new SmppGWProviderSettings(providerSettings);
    } finally {
      lock.readLock().unlock();
    }
  }

  public SmppGWEndpointSettings getEndpointSettings() {
    try {
      lock.readLock().lock();
      return new SmppGWEndpointSettings(endpointSettings);
    } finally {
      lock.readLock().unlock();
    }
  }

}
