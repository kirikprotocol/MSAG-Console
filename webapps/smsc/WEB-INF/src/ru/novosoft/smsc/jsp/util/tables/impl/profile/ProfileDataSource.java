package ru.novosoft.smsc.jsp.util.tables.impl.profile;

/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 18:23:21
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.sibinco.util.conpool.ConnectionPool;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public class ProfileDataSource
{
  private static final String[] columnNames = {"Mask", "Codepage", "Report info", "Locale", "Alias hide", "Hide modifiable", "divert", "divert_act", "divert_mod", "ussd7bit"};

  ConnectionPool connectionPool = null;

  public ProfileDataSource(ConnectionPool connectionPool) throws AdminException
  {
    this.connectionPool = connectionPool;
  }

  public QueryResultSet query(ProfileQuery query_to_run) throws AdminException
  {
    String sort = query_to_run.getSortOrder();
    boolean isNegativeSort = false;
    if (sort != null && sort.charAt(0) == '-') {
      sort = sort.substring(1);
      isNegativeSort = true;
    }
    if (sort == null || sort.length() == 0)
      sort = "mask";

    Connection connection = null;
    Statement statement = null;
    ResultSet sqlResultSet = null;
    QueryResultSetImpl results = null;

    try {
      String sortOrder;
      if (!sort.equalsIgnoreCase("mask"))
        sortOrder = sort + (isNegativeSort ? " desc" : " asc") + ", mask asc";
      else
        sortOrder = sort + (isNegativeSort ? " desc" : " asc");

      // open connection to SQL server and run query
      connection = connectionPool.getConnection();
      statement = connection.createStatement();

      final String queryStr = "select mask, reportinfo, codeset, locale, hidden, hidden_mod, divert, divert_act, divert_mod, udhConcat, translit from sms_profile " +
              createWhereStatement(query_to_run.getFilter(), query_to_run.getShow()) +
              " order by " + sortOrder;
      sqlResultSet = statement.executeQuery(queryStr);
      System.out.println("queryStr = " + queryStr);

      int totalCount = 0;
      // skip lines to start position
      for (int i = 0; i < query_to_run.getStartPosition() && sqlResultSet.next(); i++, totalCount++) ;

      // retrieve data
      results = new QueryResultSetImpl(columnNames, query_to_run.getSortOrder());
      for (int i = 0; i < query_to_run.getExpectedResultsQuantity() && sqlResultSet.next(); i++, totalCount++) {
        //final String hidden = sqlResultSet.getString("hidden");
        results.add(new ProfileDataItem(new Profile(new Mask(sqlResultSet.getString("mask")),
                Profile.getCodepageString((byte) (sqlResultSet.getShort("codeset") & 0x7F)),
                String.valueOf((sqlResultSet.getShort("codeset") & 0x80) != 0),
                Profile.getReportOptionsString(sqlResultSet.getByte("reportinfo")),
                sqlResultSet.getString("locale"),
                sqlResultSet.getString("hidden"), //hidden != null && hidden.length() > 0 && Character.toUpperCase(hidden.charAt(0)) == 'Y',
                sqlResultSet.getString("hidden_mod"),
                sqlResultSet.getString("divert"),
                sqlResultSet.getString("divert_act"),
                sqlResultSet.getString("divert_mod"),
                sqlResultSet.getString("udhConcat"),
                sqlResultSet.getString("translit"))));
      }

      boolean isLast = true;
      while (sqlResultSet.next()) {
        totalCount++;
        isLast = false;
      }
      results.setTotalSize(totalCount);
      results.setLast(isLast);
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

  public QueryResultSet query(Query query_to_run) throws AdminException
  {
    return query((ProfileQuery) query_to_run);
  }

  private String createWhereStatement(String filter, byte show)
  {
    String result = "";

    if (filter != null && filter.length() > 0 && !filter.equals("*")) {
      String f = filter.replace('*', '%');
      if (f.charAt(f.length() - 1) != '%')
        f += '%';
      result += (result.length() > 0 ? " and " : "") + "mask like '" + f + '\'';
    }

    switch (show) {
      case ProfileQuery.SHOW_ADDRESSES:
        result += (result.length() > 0 ? " and " : "") + "mask not like '%?'";
        System.out.println("Profile filtered: addresses only");
        break;
      case ProfileQuery.SHOW_MASKS:
        result += (result.length() > 0 ? " and " : "") + "mask like '%?'";
        System.out.println("Profile filtered: masks only");
        break;
      default:
        System.out.println("Profile filtered: show all");
    }

    return (result.length() > 0 ? ("where " + result) : "");
  }

  public void delete(Mask profileToDelete) throws AdminException
  {
    try {
      Connection connection = connectionPool.getConnection();
      Statement statement = connection.createStatement();

      statement.executeUpdate("delete from sms_profile where MASK = '" + profileToDelete.getNormalizedMask() + "'");
      connection.commit();
      connection.close();
    } catch (SQLException e) {
      throw new AdminException("Couldn't delete profile \"" + profileToDelete.getNormalizedMask() + "\", nested: " + e.getMessage());
    }
  }
}
