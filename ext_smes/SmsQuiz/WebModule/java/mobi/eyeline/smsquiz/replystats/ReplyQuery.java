package mobi.eyeline.smsquiz.replystats;

import mobi.eyeline.smsquiz.results.ResultFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.jsp.util.tables.Filter;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ReplyQuery extends AbstractQueryImpl {
  public ReplyQuery(int expectedResultsQuantity, Filter filter, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
