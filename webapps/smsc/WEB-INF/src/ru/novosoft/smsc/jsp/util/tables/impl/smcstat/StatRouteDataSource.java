package ru.novosoft.smsc.jsp.util.tables.impl.smcstat;

/*
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.smsstat.RouteIdCountersSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;


public class StatRouteDataSource extends AbstractDataSourceImpl
{
  private static final String[] columnNames = {"Route ID", "accepted", "rejected", "delivered", "failed", "rescheduled",
                                               "temporal", "peak_i", "peak_o", "provider", "category", "errors"};


  public StatRouteDataSource()
  {
    super(columnNames);
  }

  public void add(RouteIdCountersSet r)
  {
    super.add(new StatRouteDataItem(r));
  }

  public void remove(RouteIdCountersSet r)
  {
    super.remove(new StatRouteDataItem(r));
  }

  public QueryResultSet query(StatRouteQuery query_to_run)
  {
    return super.query(query_to_run);
  }
}
