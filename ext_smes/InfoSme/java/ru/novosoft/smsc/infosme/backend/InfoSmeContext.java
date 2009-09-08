package ru.novosoft.smsc.infosme.backend;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;
import ru.novosoft.smsc.infosme.backend.siebel.SiebelDataProvider;
import ru.novosoft.smsc.infosme.backend.siebel.SiebelTaskManager;
import ru.novosoft.smsc.infosme.backend.siebel.impl.SiebelDataProviderImpl;
import ru.novosoft.smsc.jsp.SMEAppContext;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;

import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Collection;
import java.util.Properties;

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

  private SiebelDataProvider siebelDataProvider;
  private SiebelTaskManager siebelTaskManager;

  private InfoSmeContext(SMSCAppContext appContext, String smeId)
      throws AdminException, ParserConfigurationException, SAXException, IOException,
      Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    this.smeId = smeId;
    this.appContext = appContext;
    appContext.registerSMEContext(this);
    this.infoSmeConfig = new InfoSmeConfig(appContext.getHostsManager().getServiceInfo(smeId).getServiceFolder().getAbsolutePath(), this);
    this.infoSme = new InfoSme(appContext.getHostsManager().getServiceInfo(this.smeId),
        infoSmeConfig.getAdminHost(),
        infoSmeConfig.getAdminPort());

    this.blackListManager = new BlackListManager(appContext.getPersonalizationClientPool());
    if(infoSmeConfig.isSiebelTMStarted()) {
      startSiebelTaskManager();
      System.out.println("Siebel: Siebel is started");
    }
  }

  public boolean isSiebelOnline() {
    return siebelTaskManager != null && siebelTaskManager.isOnline();
  }

  public void startSiebelTaskManager() throws AdminException{
    if(siebelDataProvider == null && siebelTaskManager == null) {
      try{
        Properties props = new Properties();
        props.setProperty("jdbc.source", infoSmeConfig.getSiebelJDBCSource());
        props.setProperty("jdbc.driver", infoSmeConfig.getSiebelJDBCDriver());
        props.setProperty("jdbc.user", infoSmeConfig.getSiebelJDBCUser());
        props.setProperty("jdbc.pass", infoSmeConfig.getSiebelJDBCPass());
        siebelDataProvider = new SiebelDataProviderImpl(props);
        siebelTaskManager = new SiebelTaskManager(siebelDataProvider, this);
      }catch(Throwable e) {
        throw new AdminException("Can't init SiebelTaskManager", e);
      }
    }
    siebelTaskManager.start();
    infoSmeConfig.setSiebelTMStarted(true);
  }

  public void stopSiebelTaskManager() throws AdminException{
    if(siebelTaskManager != null) {
      siebelTaskManager.shutdown();
      if(siebelDataProvider != null) {
        siebelDataProvider.shutdown();
        infoSmeConfig.setSiebelTMStarted(false);
      }
    }
  }

  public void shutdown()
  {
    if(siebelTaskManager != null) {
      siebelTaskManager.shutdown();
    }
    if(siebelDataProvider != null) {
      siebelDataProvider.shutdown();
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

}
