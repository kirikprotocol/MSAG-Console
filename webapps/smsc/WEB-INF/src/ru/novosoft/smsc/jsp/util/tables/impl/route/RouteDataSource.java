package ru.novosoft.smsc.jsp.util.tables.impl.route;

/*
 * Author: igork
 * Date: 10.06.2002
 * Time: 17:09:45
 */

import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;


public class RouteDataSource extends AbstractDataSourceImpl
{
  private static final String[] columnNames = {"Route ID", "providerId", "categoryId", "isEnabling", "isBilling", "isTransit", "isArchiving", "active", "suppressDeliveryReports"};


  public RouteDataSource()
  {
    super(columnNames);
  }

  public void add(Route route)
  {
    super.add(new RouteDataItem(route));
  }

  public void remove(Route route)
  {
    super.remove(new RouteDataItem(route));
  }

  public QueryResultSet query(RouteQuery query_to_run)
  {
    return super.query(query_to_run);
  }
}
