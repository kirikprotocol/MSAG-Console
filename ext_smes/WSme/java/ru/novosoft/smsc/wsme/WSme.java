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
import ru.novosoft.util.conpool.NSConnectionPool;

import javax.sql.DataSource;

import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import java.sql.PreparedStatement;
import java.sql.Connection;
import java.sql.ResultSet;

public class WSme extends WSmeTransport
{
  private DataSource ds;

  public WSme(Service wsmeService) throws AdminException
  {
    super(wsmeService.getInfo());

    // TODO: init ds somehow
    try
    {
      Properties props = new Properties();
      props.setProperty("jdbc.source", "jdbc:oracle:thin:@dbs.novosoft.ru:1521:ORCL");
      props.setProperty("jdbc.driver", "oracle.jdbc.driver.OracleDriver");
      props.setProperty("jdbc.user", "smsc");
      props.setProperty("jdbc.pass", "smsc");
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
