package ru.novosoft.smsc.infosme.backend;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.util.conpool.NSConnectionPool;

import javax.xml.parsers.ParserConfigurationException;
import javax.sql.DataSource;
import java.io.File;
import java.io.IOException;
import java.util.*;
import java.sql.SQLException;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:22:21 PM
 */
public class InfoSmeContext
{
  private static InfoSmeContext instance = null;

  public static synchronized InfoSmeContext getInstance(SMSCAppContext appContext) throws AdminException, ParserConfigurationException, IOException, SAXException, Config.WrongParamTypeException, Config.ParamNotFoundException, SQLException
  {
    return instance != null ? instance : (instance = new InfoSmeContext(appContext));
  }


  private final SMSCAppContext appContext;
  private Config config = null;
  private InfoSme infoSme = null;
  private String providersSort = "provider";
  private int providersPageSize = 20;
  private String tasksSort = "name";
  private int tasksPageSize = 20;
  private String schedulesSort = "name";
  private int schedulesPageSize = 20;

  private boolean changedOptions = false;
  private boolean changedDrivers = false;
  private boolean changedProviders = false;
  private boolean changedTasks = false;
  private boolean changedSchedules = false;
  private DataSource dataSource = null;

  public InfoSmeContext(SMSCAppContext appContext) throws AdminException, ParserConfigurationException, SAXException, IOException, Config.WrongParamTypeException, Config.ParamNotFoundException, SQLException
  {
    this.appContext = appContext;
    this.infoSme = new InfoSme(appContext.getHostsManager().getServiceInfo(Constants.INFO_SME_ID));
    resetConfig();
  }

  public Config loadCurrentConfig() throws AdminException, IOException, SAXException, ParserConfigurationException
  {
    return new Config(new File(appContext.getHostsManager().getServiceInfo(Constants.INFO_SME_ID).getServiceFolder(),
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

  public void resetConfig() throws AdminException, SAXException, ParserConfigurationException, IOException, Config.WrongParamTypeException, Config.ParamNotFoundException, SQLException
  {
    Config newConfig = loadCurrentConfig();
    final String newSource = newConfig.getString("InfoSme.systemDataSource.jdbc.source");
    final String newDriver = newConfig.getString("InfoSme.systemDataSource.jdbc.driver");
    final String newUser = newConfig.getString("InfoSme.systemDataSource.dbUserName");
    final String newPassword = newConfig.getString("InfoSme.systemDataSource.dbUserPassword");
    if (config == null
            || !newSource.equals(config.getString("InfoSme.systemDataSource.jdbc.source"))
            || !newDriver.equals(config.getString("InfoSme.systemDataSource.jdbc.driver"))
            || !newUser.equals(config.getString("InfoSme.systemDataSource.dbUserName"))
            || !newPassword.equals(config.getString("InfoSme.systemDataSource.dbUserPassword"))
    ) {
      Properties properties = new Properties();
      properties.setProperty("jdbc.source", newSource);
      properties.setProperty("jdbc.driver", newDriver);
      properties.setProperty("jdbc.user", newUser);
      properties.setProperty("jdbc.pass", newPassword);
      dataSource = new NSConnectionPool(properties);
    }
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

  public static InfoSmeContext getInstance()
  {
    return instance;
  }

  public static void setInstance(InfoSmeContext instance)
  {
    InfoSmeContext.instance = instance;
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

  public DataSource getDataSource()
  {
    return dataSource;
  }
}
