/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 13, 2003
 * Time: 5:14:21 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.util.conpool.NSConnectionPool;
import ru.novosoft.smsc.util.config.Config;

import javax.sql.DataSource;

import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import java.sql.PreparedStatement;
import java.sql.Connection;
import java.sql.ResultSet;
import java.io.File;

public class WSme extends WSmeTransport
{
  private DataSource ds;
  private File configFile;
  private Config config;

  private WSmeLock dsLock = new WSmeLock();

  public WSme(Service wsmeService) throws AdminException
  {
    super(wsmeService.getInfo());
    try
    {
      ServiceInfo info = wsmeService.getInfo();
      configFile = new File(new File(
          WebAppFolders.getServiceFolder(info.getHost(), info.getId()), "conf"), "config.xml");
      System.out.println("Config file '"+configFile.getAbsolutePath()+"'");
      config = new Config(configFile);
      loadup(null, config, false);
    }
    catch (Exception e) {
      AdminException ae = new AdminException("WSme admin init failed, cause: "+e.getMessage());
      throw ae;
    }
  }

  private final static String CONFIG_PARAM_BASE = "WSme.Admin.Web.jdbc.";
  private synchronized void loadup(Config oldConfig, Config newConfig, boolean check)
    throws AdminException
  {
    try
    {
      dsLock.acquireWrite();

      if (check && oldConfig != null &&
          oldConfig.getString(CONFIG_PARAM_BASE+"source").equals(
          newConfig.getString(CONFIG_PARAM_BASE+"source")) &&
          oldConfig.getString(CONFIG_PARAM_BASE+"driver").equals(
          newConfig.getString(CONFIG_PARAM_BASE+"driver")) &&
          oldConfig.getString(CONFIG_PARAM_BASE+"user").equals(
          newConfig.getString(CONFIG_PARAM_BASE+"user")) &&
          oldConfig.getString(CONFIG_PARAM_BASE+"password").equals(
          newConfig.getString(CONFIG_PARAM_BASE+"password"))
      ) return; // We don't need to recreate datasource;

      ds = null;
      Properties props = new Properties();
      props.setProperty("jdbc.source", newConfig.getString(CONFIG_PARAM_BASE+"source"));
      props.setProperty("jdbc.driver", newConfig.getString(CONFIG_PARAM_BASE+"driver"));
      props.setProperty("jdbc.user", newConfig.getString(CONFIG_PARAM_BASE+"user"));
      props.setProperty("jdbc.pass", newConfig.getString(CONFIG_PARAM_BASE+"password"));
      ds = new NSConnectionPool(props);
    }
    catch (Exception exc)
    {
      AdminException ae = new AdminException(
          "Init datasource failed, cause: "+exc.getMessage());
      System.out.println(ae.getMessage());
      throw ae;
    } finally {
      dsLock.release();
    }
  }
  public synchronized void reload(Config newConfig)
    throws AdminException
  {
    try {
      loadup(config, newConfig, true);
      newConfig.save();
      config = newConfig;
    } catch (Exception e) {
      throw new AdminException("Apply config failed, cause: "+e.getMessage());
    }
  }
  public synchronized Config getConfig()
      throws AdminException
  {
    try { return new Config(configFile); }
    catch (Exception e) {
      throw new AdminException("Load configuration failed, cause: "+e.getMessage());
    }
  }

  private final static String GET_VISITORS_SQL=
      "SELECT MASK FROM WSME_VISITORS ORDER BY MASK ASC";
  /**
   * Returns list of visitors' masks as Strings
   * @return list of visitors' masks as Strings
   */
  public List getVisitors() throws AdminException
  {
    ArrayList visitors = new ArrayList();

    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try
    {
      dsLock.acquireRead();
      if (ds == null)
        throw new AdminException("DataSource not inited.");
      connection = ds.getConnection();
      stmt = connection.prepareStatement(GET_VISITORS_SQL);
      rs = stmt.executeQuery();
      while (rs.next())
        visitors.add(rs.getString(1));
    }
    catch (Exception exc)
    {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    }
    finally
    {
      dsLock.release();
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
    return visitors;
  }

  private final static String GET_LANGS_SQL =
      "SELECT MASK, LANG FROM WSME_LANGS ORDER BY MASK ASC";
  /**
   * Returns list of pairs (mask, lang)
   * @return list of pairs (mask, lang) as LangRow
   */
  public List getLangs() throws AdminException
  {
    ArrayList langs = new ArrayList();
    /*langs.add(new LangRow("1111", "EN/EN"));
    langs.add(new LangRow("2222", "RU/RU"));
    langs.add(new LangRow("3333", "EN/US"));*/

    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try
    {
      dsLock.acquireRead();
      if (ds == null)
        throw new AdminException("DataSource not inited.");
      connection = ds.getConnection();
      stmt = connection.prepareStatement(GET_LANGS_SQL);
      rs = stmt.executeQuery();
      while (rs.next())
        langs.add(new LangRow(rs.getString(1), rs.getString(2)));
    }
    catch (Exception exc)
    {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    }
    finally
    {
      dsLock.release();
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
    return langs;
  }

  private final static String GET_ADS_SQL =
      "SELECT ID, LANG, AD FROM WSME_AD ORDER BY ID ASC";
  /**
   * Returns list of pairs (mask, lang)
   * @return list of pairs (mask, lang) as LangRow
   */
  public List getAds() throws AdminException
  {
    ArrayList ads = new ArrayList();

    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try
    {
      dsLock.acquireRead();
      if (ds == null)
        throw new AdminException("DataSource not inited.");
      connection = ds.getConnection();
      stmt = connection.prepareStatement(GET_ADS_SQL);
      rs = stmt.executeQuery();
      while (rs.next())
        ads.add(new AdRow(rs.getInt(1), rs.getString(2), rs.getString(3)));
    }
    catch (Exception exc)
    {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    }
    finally
    {
      dsLock.release();
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
    return ads;
  }
}
