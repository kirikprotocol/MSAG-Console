package mobi.eyeline.informer.admin;

import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import mobi.eyeline.informer.admin.archive.ArchiveDaemonManager;
import mobi.eyeline.informer.admin.blacklist.BlacklistManager;
import mobi.eyeline.informer.admin.blacklist.BlacklistManagerImpl;
import mobi.eyeline.informer.admin.delivery.DeliveryManager;
import mobi.eyeline.informer.admin.delivery.StatisticsManager;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetectorImpl;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.ftpserver.FtpServerManager;
import mobi.eyeline.informer.admin.ftpserver.PureFtpServerManager;
import mobi.eyeline.informer.admin.informer.InformerManager;
import mobi.eyeline.informer.admin.informer.InformerManagerImpl;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.InfosmeImpl;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.regions.RegionsManager;
import mobi.eyeline.informer.admin.service.ServiceManager;
import mobi.eyeline.informer.admin.smsc.SmscManager;
import mobi.eyeline.informer.admin.users.UsersManager;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.Properties;
import java.util.concurrent.ConcurrentHashMap;

/**
 * @author Artem Snopkov
 */
class AdminContextBase {
  protected static final Logger logger = Logger.getLogger(AdminContext.class);

  protected FileSystem fileSystem;

  private InstallationType instType;

  protected Journal journal;

  protected UsersManager usersManager;

  protected WebConfigManager webConfig;

  protected InformerManager informerManager;

  protected Infosme infosme;

  protected BlacklistManager blacklistManager;

  private PersonalizationClientPool personalizationClientPool;

  protected SmscManager smscManager;

  protected RegionsManager regionsManager;

  protected ServiceManager serviceManager;

  protected DeliveryManager deliveryManager;

  protected StatisticsManager statisticsManager;

  protected File workDir;

  protected DeliveryChangesDetectorImpl deliveryChangesDetector;

  protected FtpServerManager ftpServerManager;

  protected ArchiveDaemonManager archiveDaemonManager;

// delivery ->user ->region->smsc

  final private ConcurrentHashMap<Integer, Object> deliveriesLock = new ConcurrentHashMap<Integer, Object>(10);


  protected AdminContextBase() {
  }

  protected AdminContextBase(File appBaseDir) throws InitException {

    try {
      File webConfDir = new File(appBaseDir, "conf");
      this.webConfig = new WebConfigManager(new File(webConfDir, "webconfig.xml"), new File(webConfDir, "backup"), FileSystem.getFSForSingleInst());

      this.instType = webConfig.getInstallationType();

      File servicesDir = new File(appBaseDir, "services");

      File confDir = new File(servicesDir, "Informer" + File.separatorChar + "conf");

      workDir = new File(webConfig.getWorkDir());

      if (!workDir.exists() && !workDir.mkdirs()) {
        throw new InitException("Can't create work dir: " + workDir.getAbsolutePath());
      }

      switch (this.instType) {
        case SINGLE:
          serviceManager = ServiceManager.getServiceManagerForSingleInst(webConfig.getSingleDaemonHost(), webConfig.getSingleDaemonPort(), servicesDir);
          fileSystem = FileSystem.getFSForSingleInst();
          break;
        case HS:
          serviceManager = ServiceManager.getServiceManagerForHSInst(webConfig.getHSDaemonHost(), webConfig.getHSDaemonPort(), servicesDir, webConfig.getHSDaemonHosts());
          fileSystem = FileSystem.getFSForHSInst(appBaseDir, webConfig.getAppMirrorDirs());
          break;
        default:
          serviceManager = ServiceManager.getServiceManagerForHAInst(new File(appBaseDir, "conf/resourceGroups.properties"), servicesDir);
          fileSystem = FileSystem.getFSForHAInst();
      }

      if (serviceManager.getService("ftpserver") != null)
        ftpServerManager = new PureFtpServerManager(serviceManager, "ftpserver", fileSystem);

      journal = new Journal(new File(webConfig.getJournalDir()), fileSystem);
      informerManager = new InformerManagerImpl(new File(confDir, "config.xml"),
          new File(confDir, "backup"), fileSystem, serviceManager);


      File archiveDir = new File(servicesDir, "ArchiveDaemon" + File.separatorChar + "conf");

      if(serviceManager.getService("ArchiveDaemon") != null) {
        archiveDaemonManager = new ArchiveDaemonManager(new File(archiveDir, "config.xml"),
            new File(confDir, "backup"), fileSystem, serviceManager);
      }

      InformerSettings is = informerManager.getConfigSettings();
      infosme = new InfosmeImpl(is.getAdminHost(), is.getAdminPort());

      usersManager = new UsersManager(infosme, new File(confDir, "users.xml"), new File(confDir, "backup"), fileSystem);

      Properties pers = new Properties();
      pers.setProperty("personalization.host", is.getPersHost());
      pers.setProperty("personalization.port", Integer.toString(is.getPersSyncPort()));

      personalizationClientPool = new PersonalizationClientPool(pers);

      blacklistManager = new BlacklistManagerImpl(personalizationClientPool);

      smscManager = new SmscManager(infosme, new File(confDir, "smsc.xml"),
          new File(confDir, "backup"), fileSystem);
      regionsManager = new RegionsManager(infosme, new File(confDir, "regions.xml"),
          new File(confDir, "backup"), fileSystem);

      deliveryManager = new DeliveryManager(is.getDcpHost(), is.getDcpPort());
      statisticsManager = new StatisticsManager(new File(is.getStatDir()), fileSystem);

    } catch (AdminException e) {
      throw new InitException(e);
    } catch (PersonalizationClientException e) {
      throw new InitException(e);
    }
  }

  protected Object getLock(int deliveryId) {
    Object lock = deliveriesLock.get(deliveryId);
    if (lock == null) {
      Object l = deliveriesLock.putIfAbsent(deliveryId, lock = new Object());
      if (l != null) {
        lock = l;
      }
    }
    return lock;
  }

  public boolean isFtpServerDeployed() {
    return ftpServerManager != null;
  }

  public boolean isArchiveDaemonDeployed() {
    return archiveDaemonManager!= null;
  }


  @SuppressWarnings({"EmptyCatchBlock"})
  public void shutdown() {

    if (personalizationClientPool != null) {
      try {
        personalizationClientPool.shutdown();
      } catch (Exception e) {
      }
    }
    if (infosme != null) {
      try {
        infosme.shutdown();
      } catch (Exception e) {
      }
    }
    if (deliveryManager != null) {
      try {
        deliveryManager.shutdown();
      } catch (Exception e) {
      }
    }
  }

}
