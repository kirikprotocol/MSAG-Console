package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.io.File;
import java.util.List;

/**
 * author: Aleksandr Khalitov
 */
public class MCISmeManagerImpl implements MCISmeManager{

  private static final String SERVICE_ID = "MCISme";

  private final ConfigFileManager<MCISmeSettings> cfgFileManager;
  private final ServiceManager serviceManager;
  private final MCISmeStatistics statistics;

  public MCISmeManagerImpl(ServiceManager serviceManager, FileSystem fs) throws AdminException {
    this.serviceManager = serviceManager;
    ServiceInfo info = getInfo();
    File archiveDaemonConf = new File(info.getBaseDir(), "conf");
    this.statistics = new MCISmeStatistics(new File(info.getBaseDir(),"stat"), fs);   //todo?
    File archiveDaemonBackup = new File(archiveDaemonConf, "backup");
    this.cfgFileManager = new ConfigFileManager<MCISmeSettings>(new File(archiveDaemonConf, "config.xml"), archiveDaemonBackup, fs, new MCISmeConfig());
  }

  public MessageStatistics getStatistics(StatFilter filter) throws AdminException {
    return statistics.getStatistics(filter);
  }

  public static boolean isSmeDeployed(ServiceManager serviceManager) throws AdminException {
    return serviceManager.getService(SERVICE_ID) != null;
  }

  private ServiceInfo getInfo() throws AdminException {
    ServiceInfo info = serviceManager.getService(SERVICE_ID);
    if (info == null)
      throw new MCISmeException("mcisme_not_found");
    return info;
  }

  public MCISmeSettings getSettings() throws AdminException {
    return cfgFileManager.load();
  }

  public void updateSettings(MCISmeSettings settings) throws AdminException {
    cfgFileManager.save(settings);
  }

  public String getSmeOnlineHost() throws AdminException {
    return getInfo().getOnlineHost();
  }

  public void switchSme(String toHost) throws AdminException {
    serviceManager.swichService(SERVICE_ID, toHost);
  }

  public void startSme() throws AdminException {
    serviceManager.startService(SERVICE_ID);
  }

  public void stopSme() throws AdminException {
    serviceManager.stopService(SERVICE_ID);
  }

  public List<String> getSmeHosts() throws AdminException {
    return getInfo().getHosts();
  }
}
