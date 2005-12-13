package ru.novosoft.smsc.emailsme.backend;

import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.sibinco.util.conpool.ConnectionPool;

import java.sql.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 15.09.2003
 * Time: 16:40:02
 */
public class ProfilesDataSource extends AbstractDataSourceImpl
{
  public ProfilesDataSource()
  {
    super(new String[]{"addr", "userid", "dayLimit", "forward"});
  }

  public QueryResultSet query(ConnectionPool connectionPool, Query query_to_run) throws SQLException
  {
    clear();
    Connection connection = null;
    PreparedStatement statement = null;
    ResultSet resultSet = null;
    try {
      connection = connectionPool.getConnection();
      statement = connection.prepareStatement("select * from emlsme_profiles");
      resultSet = statement.executeQuery();
      while (resultSet.next()) {
        add(new ProfilesDataItem(resultSet.getString("address"), resultSet.getString("username"),
                                 resultSet.getInt("daily_limit"), resultSet.getString("forward")));
      }
    } finally {
      if( resultSet != null ) {
        try {
          resultSet.close();
        } catch (SQLException e) {}
      }
      if( statement != null ) {
        try {
          statement.close();
        } catch (SQLException e) {}
      }
      if( connection != null ) {
        try {
          connection.close();
        } catch (SQLException e) {}
      }
    }
    return super.query(query_to_run);
  }
}
