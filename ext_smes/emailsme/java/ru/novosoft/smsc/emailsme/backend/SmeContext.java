package ru.novosoft.smsc.emailsme.backend;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.util.conpool.NSConnectionPool;

import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.*;
import java.sql.SQLException;

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

  public static SmeContext getInstance(SMSCAppContext appContext) throws AdminException, IOException, SAXException, ParserConfigurationException, Config.ParamNotFoundException, Config.WrongParamTypeException, SQLException
  {
    return instance == null ? instance = new SmeContext(appContext) : instance;
  }


  private final SMSCAppContext appContext;
  private Config config;
  private NSConnectionPool connectionPool;
  private String sort = "addr";
  private int pageSize = 20;

  public SmeContext(SMSCAppContext appContext) throws AdminException, IOException, SAXException, ParserConfigurationException, Config.ParamNotFoundException, Config.WrongParamTypeException, SQLException
  {
    this.appContext = appContext;
    resetConfig();

    Properties connectionPoolConfig = new Properties();
    connectionPoolConfig.setProperty("jdbc.source", "jdbc:oracle:thin:@dark:1521:smsc"); //jdbc:oracle:thin:@dark:1521:smsc
    connectionPoolConfig.setProperty("jdbc.driver", "oracle.jdbc.driver.OracleDriver"); //oracle.jdbc.driver.OracleDriver
    connectionPoolConfig.setProperty("jdbc.user", config.getString("DataSource.dbUserName"));
    connectionPoolConfig.setProperty("jdbc.pass", config.getString("DataSource.dbUserPassword"));
    connectionPool = new NSConnectionPool(connectionPoolConfig);
  }

  public void resetConfig() throws AdminException, SAXException, ParserConfigurationException, IOException
  {
    config = new Config(new File(appContext.getHostsManager().getServiceInfo(SME_ID).getServiceFolder(), "conf" + File.separatorChar + "config.xml"));
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
}
