package mobi.eyeline.smsquiz.results;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.jsp.util.tables.Filter;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ResultQuery extends AbstractQueryImpl {
  public ResultQuery(int expectedResultsQuantity, Filter filter, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
