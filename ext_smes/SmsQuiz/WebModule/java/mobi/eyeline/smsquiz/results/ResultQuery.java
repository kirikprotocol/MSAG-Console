package mobi.eyeline.smsquiz.results;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ResultQuery extends AbstractQueryImpl {
  public ResultQuery(int expectedResultsQuantity, ResultFilter filter, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
