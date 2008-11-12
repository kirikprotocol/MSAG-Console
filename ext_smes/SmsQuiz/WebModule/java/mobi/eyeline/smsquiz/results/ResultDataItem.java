package mobi.eyeline.smsquiz.results;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

import java.util.Date;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ResultDataItem extends AbstractDataItem {
  public ResultDataItem(String quizId, Date replyDate, Date deliveryDate, String msisdn, String category, String message) {
    values.put(ResultDataSource.QUIZ_ID, quizId);
    values.put(ResultDataSource.REPLY_DATE, replyDate);
    values.put(ResultDataSource.MSISDN, msisdn);
    values.put(ResultDataSource.DELIVERY_DATE, deliveryDate);
    values.put(ResultDataSource.CATEGORY, category);
    values.put(ResultDataSource.MESSAGE, message);
  }
}
