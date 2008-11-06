package mobi.eyeline.smsquiz.replystats;

import mobi.eyeline.smsquiz.results.ResultFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ReplyQuery extends AbstractQueryImpl  {
    public ReplyQuery(int expectedResultsQuantity, ReplyFilter filter, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
