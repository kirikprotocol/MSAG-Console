/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 18:23:21
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.DataSource;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.util.conpool.NSConnectionPool;
import org.w3c.dom.Element;

import java.util.*;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.Connection;
import java.sql.ResultSet;

public class ProfileDataSource implements DataSource
{
  private static final String[] columnNames = {"Mask", "Codepage", "Report info"};
  private static final Map sortColumnNames;
  static
  {
    sortColumnNames = new HashMap();
    sortColumnNames.put("Mask", "mask");
    sortColumnNames.put("Codepage", "codeset");
    sortColumnNames.put("Report info", "reportinfo");
  }

  NSConnectionPool connectionPool = null;

  public ProfileDataSource(String jdbcSource, String jdbcDriver, String jdbcUser, String jdbcPassword)
      throws AdminException
  {
    Properties props = new Properties();
    props.setProperty("jdbc.source", jdbcSource);
    props.setProperty("jdbc.driver", jdbcDriver);
    props.setProperty("jdbc.user", jdbcUser);
    props.setProperty("jdbc.pass", jdbcPassword);
    try {
      connectionPool = new NSConnectionPool(props);
    } catch (SQLException e) {
      throw new AdminException("Couldn't connect to profiler database: " + e.getMessage());
    }
  }

  public QueryResultSet query(ProfileQuery query_to_run)
      throws AdminException
  {
    String sort = (String) query_to_run.getSortOrder().get(0);
    boolean isNegativeSort = false;
    if (sort.charAt(0) == '-') {
      sort = sort.substring(1);
      isNegativeSort = true;
    }

    Connection connection = null;
    Statement statement = null;
    ResultSet sqlResultSet = null;
    QueryResultSetImpl results = null;

    try {
      String sortOrder;
      if (!sort.equalsIgnoreCase("mask"))
        sortOrder = ((String)sortColumnNames.get(sort)) + (isNegativeSort ? " desc" : " asc") + ", mask asc";
      else
        sortOrder = ((String)sortColumnNames.get(sort)) + (isNegativeSort ? " desc" : " asc");

      // open connection to SQL server and run query
      connection = connectionPool.getConnection();
      statement = connection.createStatement();

      sqlResultSet = statement.executeQuery("select mask, reportinfo, codeset from sms_profile order by " + sortOrder);

      // skip lines to start position
      for (int i = 0; i < query_to_run.getStartPosition() && sqlResultSet.next(); i++) ;

      // retrieve data
      results = new QueryResultSetImpl(columnNames, query_to_run.getSortOrder());
      for (int i = 0; i < query_to_run.getExpectedResultsQuantity() && sqlResultSet.next(); i++) {
        //System.out.println("Query: ");
        String maskString = sqlResultSet.getString("mask");
        //System.out.println("maskString = " + maskString);
        byte reportinfo = sqlResultSet.getByte("reportinfo");
        //System.out.println("reportinfo = " + reportinfo);
        byte codeset = sqlResultSet.getByte("codeset");
        //System.out.println("codeset = " + codeset);
        results.add(new ProfileDataItem(new Profile(new Mask(maskString), codeset, reportinfo)));
      }
    } catch (SQLException e) {
      throw new AdminException("Couldn't retrieve data from profiles database: " + e.getMessage());
    } finally {
      // close SQL connection
      if (sqlResultSet != null)
        try {
          sqlResultSet.close();
        } catch (SQLException e) {
        }
      if (statement != null)
        try {
          statement.close();
        } catch (SQLException e) {
        }
      if (connection != null)
        try {
          connection.close();
        } catch (SQLException e) {
        }
    }
    if (results != null)
      return results;
    else
      throw new AdminException("Couldn't retrieve data from profiles database");
  }

  public QueryResultSet query(Query query_to_run)
      throws AdminException
  {
    return query((ProfileQuery) query_to_run);
  }
}
