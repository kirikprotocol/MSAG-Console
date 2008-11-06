package mobi.eyeline.smsquiz.results;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

import java.util.Date;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ResultDataItem extends AbstractDataItem {
    public ResultDataItem(String quizId, Date replyDate, Date deliveryDate, String msisdn, String category, String message) {
      values.put("quizId", quizId);
      values.put("replyDate", replyDate);
      values.put("msisdn", msisdn);
      values.put("deliveryDate", deliveryDate);
      values.put("category", category);
      values.put("message", message);
  }
}
