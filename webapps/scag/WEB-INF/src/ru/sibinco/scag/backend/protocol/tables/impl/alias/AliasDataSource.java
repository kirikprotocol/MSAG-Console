package ru.sibinco.scag.backend.protocol.tables.impl.alias;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 19:28:28
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.scag.backend.protocol.alias.Alias;
import ru.sibinco.scag.backend.protocol.tables.QueryResultSet;
import ru.sibinco.scag.backend.protocol.tables.impl.AbstractDataSourceImpl;


public class AliasDataSource extends AbstractDataSourceImpl
{
  private static final String aliasColumnName = "Alias";
  private static final String hideColumnName = "Hide";
  private static final String[] columnNames = {aliasColumnName, "Address", hideColumnName};

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
    QueryResultSet result = super.query(query_to_run);
    final String sortOrder = (String) query_to_run.getSortOrder().get(0);
    if (sortOrder != null && sortOrder.endsWith(hideColumnName)) {
      result.additionalSort(aliasColumnName);
    }
    return result;
  }
}