package ru.novosoft.smsc.infosme.backend;

import org.apache.log4j.Category;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMEAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.sibinco.util.conpool.ConnectionPool;

import javax.sql.DataSource;
import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.*;
import java.sql.SQLException;

import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:22:21 PM
 */
public class InfoSmeContext implements SMEAppContext
{
  private final static Map instances = new HashMap();
  private Category logger = Category.getInstance(this.getClass());

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
  private final TaskManager taskManager;
  private Config config = null;
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

  private boolean changedOptions = false;
  private boolean changedDrivers = false;
  private boolean changedProviders = false;
  private boolean changedTasks = false;
  private boolean changedSchedules = false;
  private BlackListManager blackListManager;
  private String smeId = "InfoSme";
  //private Long appContextId = null;

  private InfoSmeContext(SMSCAppContext appContext, String smeId)
      throws AdminException, ParserConfigurationException, SAXException, IOException,
      Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    this.smeId = smeId;
    this.appContext = appContext;
    //this.appContextId = appContext.registerSMEContext(this);
    appContext.registerSMEContext(this);
    resetConfig();
    this.infoSme = new InfoSme(appContext.getHostsManager().getServiceInfo(this.smeId),
        config.getString("InfoSme.Admin.host"),
        config.getInt("InfoSme.Admin.port"));
    this.taskManager = new TaskManager(appContext.getHostsManager().getServiceInfo(smeId).getServiceFolder().getAbsolutePath());
    this.blackListManager = new BlackListManager(appContext.getPersonalizationClientPool());
  }



  public void shutdown()
  {

  }

  public Config loadCurrentConfig()
      throws AdminException, IOException, SAXException, ParserConfigurationException
  {
    return new Config(new File(appContext.getHostsManager().getServiceInfo(smeId).getServiceFolder(),
        "conf" + File.separatorChar + "config.xml"));
  }

  public Config getConfig()
  {
    return config;
  }

  public String getProvidersSort()
  {
    return providersSort;
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

  public void resetConfig() throws AdminException, SAXException, ParserConfigurationException, IOException
  {
    Config newConfig = loadCurrentConfig();
    config = newConfig;
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

  public boolean isChangedOptions()
  {
    return changedOptions;
  }

  public void setChangedOptions(boolean changedOptions)
  {
    this.changedOptions = changedOptions;
  }

  public boolean isChangedDrivers()
  {
    return changedDrivers;
  }

  public void setChangedDrivers(boolean changedDrivers)
  {
    this.changedDrivers = changedDrivers;
  }

  public boolean isChangedProviders()
  {
    return changedProviders;
  }

  public void setChangedProviders(boolean changedProviders)
  {
    this.changedProviders = changedProviders;
  }

  public boolean isChangedTasks()
  {
    return changedTasks;
  }

  public void setChangedTasks(boolean changedTasks)
  {
    this.changedTasks = changedTasks;
  }

  public boolean isChangedSchedules()
  {
    return changedSchedules;
  }

  public void setChangedSchedules(boolean changedSchedules)
  {
    this.changedSchedules = changedSchedules;
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

  public TaskManager getTaskManager() {
    return taskManager;
  }

  public BlackListManager getBlackListManager() {
    return blackListManager;
  }

}
