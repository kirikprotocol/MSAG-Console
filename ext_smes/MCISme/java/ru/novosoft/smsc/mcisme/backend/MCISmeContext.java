package ru.novosoft.smsc.mcisme.backend;

import org.apache.log4j.Category;
import org.xml.sax.SAXException;

import java.util.Map;
import java.util.HashMap;
import java.util.Properties;
import java.io.IOException;
import java.io.File;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.util.conpool.NSConnectionPool;

import javax.xml.parsers.ParserConfigurationException;
import javax.sql.DataSource;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 16:19:19
 * To change this template use Options | File Templates.
 */
public class MCISmeContext
{
  private final static Map instances = new HashMap();
  private Category logger = Category.getInstance(this.getClass());

  public static synchronized MCISmeContext getInstance(SMSCAppContext appContext, String smeId)
      throws AdminException, ParserConfigurationException, IOException, SAXException
  {
    MCISmeContext instance = (MCISmeContext) instances.get(smeId);
    if (instance == null) {
      instance = new MCISmeContext(appContext, smeId);
      instances.put(smeId, instance);
    }
    return instance;
  }

  private final SMSCAppContext appContext;
  private Config config = null;
  private MCISme mciSme = null;

  private boolean changedOptions = false;
  private boolean changedDrivers = false;
  private DataSource dataSource = null;
  private String smeId = "MCISme";

  private MCISmeContext(SMSCAppContext appContext, String smeId)
      throws AdminException, ParserConfigurationException, SAXException, IOException
  {
    this.smeId = smeId;
    this.appContext = appContext;
    this.mciSme = new MCISme(appContext.getHostsManager().getServiceInfo(this.smeId));
    resetConfig();
  }

  public Config loadCurrentConfig()
      throws AdminException, IOException, SAXException, ParserConfigurationException
  {
    return new Config(new File(appContext.getHostsManager().getServiceInfo(smeId).getServiceFolder(),
                               "conf" + File.separatorChar + "config.xml"));
  }
  public void resetConfig()
      throws AdminException, SAXException, ParserConfigurationException, IOException
  {
    Config newConfig = loadCurrentConfig();
    reloadDataSource(config, newConfig);
    config = newConfig;
  }
  public Config getConfig() {
    return config;
  }

  public void reloadDataSource(Config oldConfig, Config newConfig)
  {
    try {
      if (oldConfig == null
              || !Config.isParamEquals(oldConfig, newConfig, "MCISme.DataSource.jdbc.source")
              || !Config.isParamEquals(oldConfig, newConfig, "MCISme.DataSource.jdbc.driver")
              || !Config.isParamEquals(oldConfig, newConfig, "MCISme.DataSource.dbUserName")
              || !Config.isParamEquals(oldConfig, newConfig, "MCISme.DataSource.dbUserPassword") )
      {
        dataSource = null;
        Properties properties = new Properties();
        properties.setProperty("jdbc.source", newConfig.getString("MCISme.DataSource.jdbc.source"));
        properties.setProperty("jdbc.driver", newConfig.getString("MCISme.DataSource.jdbc.driver"));
        properties.setProperty("jdbc.user", newConfig.getString("MCISme.DataSource.dbUserName"));
        properties.setProperty("jdbc.pass", newConfig.getString("MCISme.DataSource.dbUserPassword"));
        dataSource = new NSConnectionPool(properties);
      }
    } catch (Throwable e) {
      logger.error("Could not init datasource", e);
    }
  }

  public MCISme getMCISme() {
    return mciSme;
  }

  public boolean isChangedOptions() {
    return changedOptions;
  }
  public void setChangedOptions(boolean changedOptions) {
    this.changedOptions = changedOptions;
  }
  public boolean isChangedDrivers() {
    return changedDrivers;
  }
  public void setChangedDrivers(boolean changedDrivers) {
    this.changedDrivers = changedDrivers;
  }

  public DataSource getDataSource() {
    return dataSource;
  }

}
