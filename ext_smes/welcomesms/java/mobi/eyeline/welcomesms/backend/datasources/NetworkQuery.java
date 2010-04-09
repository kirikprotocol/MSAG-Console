package mobi.eyeline.welcomesms.backend.datasources;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.jsp.util.tables.Filter;

/**
 * author: alkhal
 */
public class NetworkQuery extends AbstractQueryImpl {
  public NetworkQuery(int expectedResultsQuantity, Filter filter, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}