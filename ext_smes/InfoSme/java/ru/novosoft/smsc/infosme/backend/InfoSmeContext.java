package ru.novosoft.smsc.infosme.backend;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;

import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:22:21 PM
 */
public class InfoSmeContext {
  private static InfoSmeContext instance = null;

  public static synchronized InfoSmeContext getInstance(SMSCAppContext appContext) throws AdminException, ParserConfigurationException, IOException, SAXException
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

  public InfoSmeContext(SMSCAppContext appContext) throws AdminException, ParserConfigurationException, SAXException, IOException
  {
    this.appContext = appContext;
    this.infoSme = new InfoSme(appContext.getHostsManager().getServiceInfo(Constants.INFO_SME_ID));
    resetConfig();
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
    config = new Config(new File(appContext.getHostsManager().getServiceInfo(Constants.INFO_SME_ID).getServiceFolder(),
                                 "conf" + File.separatorChar + "config.xml"));
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
}
