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
      load();
    }
    catch (Exception e) {
      AdminException ae = new AdminException("WSme admin init failed, cause: "+e.getMessage());
      throw ae;
    }
  }

  private final static String CONFIG_PARAM_BASE = "WSme.Admin.Web.jdbc.";
  private synchronized void load()
    throws AdminException
  {
    ds = null;
    try
    {
      Properties props = new Properties();
      props.setProperty("jdbc.source", config.getString(CONFIG_PARAM_BASE+"source"));
      props.setProperty("jdbc.driver", config.getString(CONFIG_PARAM_BASE+"driver"));
      props.setProperty("jdbc.user", config.getString(CONFIG_PARAM_BASE+"user"));
      props.setProperty("jdbc.pass", config.getString(CONFIG_PARAM_BASE+"password"));
      ds = new NSConnectionPool(props);
    }
    catch (Exception exc)
    {
      AdminException ae = new AdminException(
          "Init datasource failed, cause: "+exc.getMessage());
      System.out.println(ae.getMessage());
      throw ae;
    }
  }
  public synchronized void reload(Config newConfig)
    throws AdminException
  {
    // TODO save config for WSme here (& restart WSme ?)
    try {
      config = newConfig;
      config.save();
    } catch (Exception e) {
      throw new AdminException("Save config failed, cause: "+e.getMessage());
    }
    load();
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
    /*visitors.add("1111");
    visitors.add("2222");
    visitors.add("3333");
    visitors.add("4444");*/

    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try
    {
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
    /*ads.add(new AdRow(1, "EN/EN", "Hello 1"));
    ads.add(new AdRow(1, "EN/US", "Hy 1"));
    ads.add(new AdRow(1, "RU/RU", "Привет 1"));
    ads.add(new AdRow(2, "EN/EN", "Hello 2"));
    ads.add(new AdRow(2, "EN/US", "Hy 2"));
    ads.add(new AdRow(2, "RU/RU", "Привет 2"));*/

    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try
    {
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
