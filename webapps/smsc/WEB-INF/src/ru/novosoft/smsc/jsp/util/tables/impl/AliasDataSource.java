/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 16:59:45
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;


public class AliasDataSource extends AbstractDataSourceImpl
{
  private static final String[] columnNames = {"Alias", "Address"};

  public AliasDataSource()
  {
    super(columnNames);
  }

  public void add(Alias alias)
  {
    super.add(new AliasDataItem(alias));
  }

  public void remove(Alias alias)
  {
    super.remove(new AliasDataItem(alias));
  }

  public QueryResultSet query(AliasQuery query_to_run)
  {
    return super.query(query_to_run);
  }
}
