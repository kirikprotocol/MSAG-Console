package ru.novosoft.smsc.jsp.util.tables.impl.dl;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.dl.Principal;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 22.09.2003
 * Time: 16:00:00
 */
public class PrincipalsDataSource extends AbstractDataSourceImpl
{
  private Category logger = Category.getInstance(this.getClass());
  private final SMSCAppContext appContext;

  public PrincipalsDataSource(SMSCAppContext appContext)
  {
    super(new String[]{"address", "max_lst", "max_el"});
    this.appContext = appContext;
  }

  public QueryResultSet query(Query query_to_run)
  {
    clear();
    Connection connection = null;
    try {
      connection = appContext.getConnectionPool().getConnection();
      PreparedStatement statement;
      final PrincipalsFilter filter = (PrincipalsFilter) query_to_run.getFilter();
      if (filter == null || filter.isEmpty())
        statement = connection.prepareStatement("select * from dl_principals");
      else {
        String f = filter.getFilterAddress().replace('*', '%');
        if (f.charAt(f.length() - 1) != '%')
          f += '%';
        if (f.charAt(0) == '+')
          f = ".1.1." + f;
        else if (f.charAt(0) != '.')
          f = ".0.1." + f;
        logger.debug("filter: \"" + f + "\"");
        statement = connection.prepareStatement("select * from dl_principals where address like ?");
        statement.setString(1, f);
      }
      ResultSet resultSet = statement.executeQuery();
      while (resultSet.next()) {
        add(new Principal(resultSet.getString("address"), resultSet.getInt("max_lst"), resultSet.getInt("max_el")));
      }
    } catch (SQLException e) {
      logger.error("Could not query principals", e);
      return new EmptyResultSet();
    } finally {
      try {
        if (connection != null)
          connection.close();
      } catch (SQLException e) {
        logger.error("Could not close connection", e);
        return new EmptyResultSet();
      }
    }
    return super.query(query_to_run);
  }
}
