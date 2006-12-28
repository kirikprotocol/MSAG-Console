package ru.novosoft.smsc.jsp.util.tables;

import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;

import java.util.Comparator;

/**
 * User: artem
 * Date: 27.12.2006
 */
public class AggregatorMax implements Aggregator {
  private final Comparator comparator;

  public AggregatorMax(Comparator comparator) {
    this.comparator = comparator;
  }

  public QueryResultSet aggregateList(QueryResultSet list) {
    if (list == null || list.size() == 0)
      return list;

    DataItem max = list.get(0);
    for (int i = 1; i < list.size(); i++) {
      DataItem dataItem =  list.get(i);
      if (comparator.compare(max, dataItem) < 0)
        max = dataItem;
    }

    final QueryResultSetImpl rs = new QueryResultSetImpl();
    rs.add(max);
    return rs;
  }
}
