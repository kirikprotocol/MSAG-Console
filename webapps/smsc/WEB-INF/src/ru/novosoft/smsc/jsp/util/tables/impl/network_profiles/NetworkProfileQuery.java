package ru.novosoft.smsc.jsp.util.tables.impl.network_profiles;

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

/**
 * @author Aleksandr Khalitov
 */
public class NetworkProfileQuery extends AbstractQueryImpl{
  public NetworkProfileQuery(int expectedResultsQuantity, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, new Filter() {
      public boolean isEmpty() {
        return true;
      }
      public boolean isItemAllowed(DataItem item) {
        return true;
      }
    }, sortOrder, startPosition);
  }
}
