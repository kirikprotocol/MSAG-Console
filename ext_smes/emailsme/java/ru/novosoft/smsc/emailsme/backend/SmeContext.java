package ru.novosoft.smsc.emailsme.backend;

import org.apache.log4j.Category;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.util.conpool.NSConnectionPool;

import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 11.09.2003
 * Time: 15:41:27
 * To change this template use Options | File Templates.
 */
public class SmeContext
{
  public static final String SME_ID = "emailsme";

  private static SmeContext instance = null;

  public static SmeContext getInstance(SMSCAppContext appContext) throws AdminException, IOException, SAXException, ParserConfigurationException
  {
    return instance == null ? instance = new SmeContext(appContext) : instance;
  }


  private final SMSCAppContext appContext;
  private Config config;
  private NSConnectionPool connectionPool;
  private String sort = "addr";
  private int pageSize = 20;
  private Category logger = Category.getInstance(this.getClass());

  public SmeContext(SMSCAppContext appContext) throws IOException, ParserConfigurationException, SAXException, AdminException
  {
    this.appContext = appContext;
    resetConfig();
  }

  public void resetConfig() throws AdminException, IOException, ParserConfigurationException, SAXException
  {
    config = loadCurrentConfig();
  }

  public Config getConfig()
  {
    return config;
  }

  public NSConnectionPool getConnectionPool()
  {
    return connectionPool;
  }

  public String getSort()
  {
    return sort;
  }

  public void setSort(String sort)
  {
    this.sort = sort;
  }

  public int getPageSize()
  {
    return pageSize;
  }

  public void setPageSize(int pageSize)
  {
    this.pageSize = pageSize;
  }

  public void applyJdbc(Config oldConfig)
  {
    try {
      final String newSource = config.getString("DataSource.jdbc.source");
      final String newDriver = config.getString("DataSource.jdbc.driver");
      final String newUser = config.getString("DataSource.dbUserName");
      final String newPassword = config.getString("DataSource.dbUserPassword");
      if (oldConfig == null
              || !newSource.equals(oldConfig.getString("DataSource.jdbc.source"))
              || !newDriver.equals(oldConfig.getString("DataSource.jdbc.driver"))
              || !newUser.equals(oldConfig.getString("DataSource.dbUserName"))
              || !newPassword.equals(oldConfig.getString("DataSource.dbUserPassword"))
      ) {
        connectionPool = null;
        Properties connectionPoolConfig = new Properties();
        connectionPoolConfig.setProperty("jdbc.source", newSource);
        connectionPoolConfig.setProperty("jdbc.driver", newDriver);
        connectionPoolConfig.setProperty("jdbc.user", newUser);
        connectionPoolConfig.setProperty("jdbc.pass", newPassword);
        connectionPool = new NSConnectionPool(connectionPoolConfig);
      }
    } catch (Throwable e) {
      logger.error("Could not init connection pool", e);
      connectionPool = null;
    }
  }

  public Config loadCurrentConfig() throws AdminException, IOException, SAXException, ParserConfigurationException
  {
    return new Config(new File(appContext.getHostsManager().getServiceInfo(SME_ID).getServiceFolder(), "conf" + File.separatorChar + "config.xml"));
  }
}
