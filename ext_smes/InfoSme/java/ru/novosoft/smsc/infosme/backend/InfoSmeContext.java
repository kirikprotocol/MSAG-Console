package ru.novosoft.smsc.infosme.backend;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;
import ru.novosoft.smsc.infosme.backend.siebel.SiebelDataProvider;
import ru.novosoft.smsc.infosme.backend.siebel.SiebelTaskManager;
import ru.novosoft.smsc.infosme.backend.siebel.SiebelFinalStateThread;
import ru.novosoft.smsc.infosme.backend.siebel.impl.SiebelDataProviderImpl;
import ru.novosoft.smsc.infosme.backend.commands.InfoSmeTaskManager;
import ru.novosoft.smsc.infosme.backend.commands.InfoSmeExportStatManager;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataSource;
import ru.novosoft.smsc.jsp.SMEAppContext;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;

import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:22:21 PM
 */
public class InfoSmeContext implements SMEAppContext
{
  private final static Map instances = new HashMap(10);

  public static synchronized InfoSmeContext getInstance(SMSCAppContext appContext, String smeId)
      throws AdminException, ParserConfigurationException, IOException, SAXException, Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    InfoSmeContext instance = (InfoSmeContext) instances.get(smeId);
    if (instance == null) {
      instance = new InfoSmeContext(appContext, smeId);
      instances.put(smeId, instance);
    }
    return instance;
  }

  private final SMSCAppContext appContext;
  private final InfoSmeConfig infoSmeConfig;
  private InfoSme infoSme = null;
  private String providersSort = "provider";
  private int providersPageSize = 20;
  private String tasksSort = "name";
  private int tasksPageSize = 20;
  private String schedulesSort = "name";
  private int schedulesPageSize = 20;
  private String messagesSort = Message.SORT_BY_DATE;
  private int messagesPageSize = 20;
  private int maxMessagesTotalSize = 1000;

  private BlackListManager blackListManager;
  private String smeId = "InfoSme";

  private final InfoSmeTaskManager taskManager;
  private final InfoSmeExportStatManager exportStatManager;

  private SiebelDataProvider siebelDataProvider;
  private SiebelTaskManager siebelTaskManager;
  private SiebelFinalStateThread siebelFinalStateThread;

  private InfoSmeLicense license;

  private TaskArchiveDaemon taskArchiveDaemon;

  private InfoSmeContext(SMSCAppContext appContext, String smeId)
      throws AdminException, ParserConfigurationException, SAXException, IOException,
      Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    this.smeId = smeId;
    this.appContext = appContext;
    appContext.registerSMEContext(this);
    String serviceFolder = appContext.getHostsManager().getServiceInfo(smeId).getServiceFolder().getAbsolutePath();
    File configDir = new File(serviceFolder, "conf");
    this.infoSmeConfig = new InfoSmeConfig(configDir.getAbsolutePath(), this);
    try{
      this.license = new InfoSmeLicense(new File(configDir, "license.ini"));
    }catch (ParseException e) {
      throw new AdminException(e.getMessage());
    }
    this.infoSme = new InfoSme(appContext.getHostsManager().getServiceInfo(this.smeId),
        infoSmeConfig.getAdminHost(),
        infoSmeConfig.getAdminPort());

    this.blackListManager = new BlackListManager(appContext.getPersonalizationClientPool());
    this.taskManager = new InfoSmeTaskManager(appContext);
    this.taskManager.start();



    String msgStoreDir = this.infoSmeConfig.getStoreLocation();
    if( msgStoreDir.length() > 0 && msgStoreDir.charAt(0) != '/' )
      msgStoreDir = serviceFolder + '/' + msgStoreDir;

    this.exportStatManager = new InfoSmeExportStatManager(new MessageDataSource(this));
    this.exportStatManager.start();

    if(infoSmeConfig.isSiebelTMStarted()) {
      startSiebelTaskManager();
      System.out.println("Siebel: Siebel is started");
    }

    if(infoSmeConfig.isArchiveDaemonStarted()) {
      startArchiveDaemon();
      System.out.println("ArchiveDaemon is started");
    }
  }

  public boolean isSiebelOnline() {
    return siebelTaskManager != null && siebelTaskManager.isOnline();
  }

  public boolean isTaskArchiveDaemonOnline() {
    return taskArchiveDaemon != null && taskArchiveDaemon.isOnline();
  }

  public void startSiebelTaskManager() throws AdminException{
    try{
      if(siebelDataProvider == null) {
        siebelDataProvider = new SiebelDataProviderImpl();
        siebelTaskManager = new SiebelTaskManager(siebelDataProvider, appContext, this);
        siebelFinalStateThread = new SiebelFinalStateThread(infoSmeConfig.getStoreLocation(),
            infoSmeConfig.getArchiveLocation(), siebelDataProvider);
      }
      if(siebelDataProvider.isShutdowned()) {
        Properties props = new Properties();
        props.setProperty("jdbc.source", infoSmeConfig.getSiebelJDBCSource());
        props.setProperty("jdbc.driver", infoSmeConfig.getSiebelJDBCDriver());
        props.setProperty("jdbc.user", infoSmeConfig.getSiebelJDBCUser());
        props.setProperty("jdbc.pass", infoSmeConfig.getSiebelJDBCPass());
        props.setProperty("jdbc.pool.name", "siebel");
        siebelDataProvider.connect(props);
      }
      siebelFinalStateThread.start();
      siebelTaskManager.start();
      infoSmeConfig.setSiebelTMStarted(true);
    }catch(Throwable e) {
      throw new AdminException("Can't init SiebelTaskManager", e);
    }
  }


  public synchronized void startArchiveDaemon() throws AdminException{
    try{
      if(taskArchiveDaemon == null) {
        taskArchiveDaemon = new TaskArchiveDaemon(this, appContext);
      }
      taskArchiveDaemon.start();
      infoSmeConfig.setArchiveDaemonStarted(true);
    }catch(Throwable e) {
      throw new AdminException("Can't init TaskArchiveDaemon", e);
    }
  }

  public void stopSiebelTaskManager() throws AdminException{
    if(siebelTaskManager != null) {
      siebelTaskManager.shutdown();
      if(siebelDataProvider != null) {
        siebelDataProvider.shutdown();
        infoSmeConfig.setSiebelTMStarted(false);
      }
      if(siebelFinalStateThread != null) {
        siebelFinalStateThread.shutdown();
      }
    }
  }

  public void stopTaskArchiveDaemon() throws AdminException{
    if(taskArchiveDaemon != null) {
      taskArchiveDaemon.shutdown();
      infoSmeConfig.setArchiveDaemonStarted(false);
    }
  }

  public void shutdown()
  {
    if(siebelTaskManager != null) {
      siebelTaskManager.shutdown();
    }
    if(siebelFinalStateThread != null) {
      siebelFinalStateThread.shutdown();
    }
    if(siebelDataProvider != null) {
      siebelDataProvider.shutdown();
    }
    if(taskArchiveDaemon != null) {
      taskArchiveDaemon.shutdown();
    }
    if(taskManager != null) {
      taskManager.shutdown();
    }
    if(exportStatManager != null) {
      exportStatManager.shutdown();
    }
  }

  public Config loadCurrentConfig()
      throws AdminException, IOException, SAXException, ParserConfigurationException
  {
    return new Config(new File(appContext.getHostsManager().getServiceInfo(smeId).getServiceFolder(),
        "conf" + File.separatorChar + "config.xml"));
  }

  public String getProvidersSort()
  {
    return providersSort;
  }

  public SMSCAppContext getAppContext() {
    return appContext;
  }

  public void setProvidersSort(String providersSort)
  {
    this.providersSort = providersSort;
  }

  public int getProvidersPageSize()
  {
    return providersPageSize;
  }

  public void setProvidersPageSize(int providersPageSize)
  {
    this.providersPageSize = providersPageSize;
  }

  public String getTasksSort()
  {
    return tasksSort;
  }

  public void setTasksSort(String tasksSort)
  {
    this.tasksSort = tasksSort;
  }

  public int getTasksPageSize()
  {
    return tasksPageSize;
  }

  public void setTasksPageSize(int tasksPageSize)
  {
    this.tasksPageSize = tasksPageSize;
  }


  public String getSchedulesSort()
  {
    return schedulesSort;
  }

  public void setSchedulesSort(String schedulesSort)
  {
    this.schedulesSort = schedulesSort;
  }

  public int getSchedulesPageSize()
  {
    return schedulesPageSize;
  }

  public void setSchedulesPageSize(int schedulesPageSize)
  {
    this.schedulesPageSize = schedulesPageSize;
  }

  public InfoSme getInfoSme()
  {
    return infoSme;
  }

  public Collection getSmscConectors() {
    return getInfoSmeConfig().getSmscConns().keySet();
  }
  public String getMessagesSort()
  {
    return messagesSort;
  }

  public void setMessagesSort(String messagesSort)
  {
    this.messagesSort = messagesSort;
  }

  public int getMessagesPageSize()
  {
    return messagesPageSize;
  }

  public void setMessagesPageSize(int messagesPageSize)
  {
    this.messagesPageSize = messagesPageSize;
  }

  public int getMaxMessagesTotalSize() {
    return maxMessagesTotalSize;
  }

  public InfoSmeConfig getInfoSmeConfig() {
    return infoSmeConfig;
  }

  public BlackListManager getBlackListManager() {
    return blackListManager;
  }

  public InfoSmeTaskManager getTaskManager() {
    return taskManager;
  }

  public InfoSmeExportStatManager getExportStatManager() {
    return exportStatManager;
  }

  public Boolean getUssdFeature() {
    return infoSmeConfig.getUssdPushFeature();
  }

  public InfoSmeLicense getLicense() {
    return license;
  }

  public String validateInfosmePrefs(UserPreferences preferences){
    try{
      InfoSmeConfig.validateInfoSmePreferences(preferences);
      return null;
    }catch(AdminException e) {
      return e.getMessage();
    }
  }
}
