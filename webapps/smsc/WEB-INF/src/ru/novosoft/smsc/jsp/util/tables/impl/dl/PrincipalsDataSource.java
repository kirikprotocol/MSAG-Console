package ru.novosoft.smsc.jsp.util.tables.impl.dl;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.dl.Principal;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.List;

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

  public QueryResultSet query(Query query_to_run,DistributionListAdmin admin )
  {
    clear(); List list;
    try {
      final PrincipalsFilter filter = (PrincipalsFilter) query_to_run.getFilter();
      if (filter == null || filter.isEmpty())
          list=admin.principals();
      else {
        String f = filter.getFilterAddress().replace('*', '%');
        if (f.charAt(f.length() - 1) != '%')
          f += '%';
        if (f.charAt(0) == '+')
          f = ".1.1." + f;
        else if (f.charAt(0) != '.')
          f = ".0.1." + f;
        logger.debug("filter: \"" + f + "\"");
        list=admin.principals(f);
      }
      int i=0;
      while (i<list.size()) {
         add((Principal)list.get(i++));
      }
    } catch (AdminException e) {
          e.printStackTrace();
          logger.warn(e.getMessage());  //To change body of catch statement use File | Settings | File Templates.
      return new EmptyResultSet();
     }
    return super.query(query_to_run);
  }
}
