package ru.novosoft.smsc.emailsme.backend;

import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.util.conpool.NSConnectionPool;

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

  public QueryResultSet query(NSConnectionPool connectionPool, Query query_to_run) throws SQLException
  {
    clear();
    Connection connection = connectionPool.getConnection();
    PreparedStatement statement = connection.prepareStatement("select * from emlsme_profiles");
    ResultSet resultSet = statement.executeQuery();
    while (resultSet.next()) {
      add(new ProfilesDataItem(resultSet.getString("address"), resultSet.getString("username"),
                               resultSet.getInt("daily_limit"), resultSet.getString("forward")));
    }
    connection.close();
    return super.query(query_to_run);
  }
}
