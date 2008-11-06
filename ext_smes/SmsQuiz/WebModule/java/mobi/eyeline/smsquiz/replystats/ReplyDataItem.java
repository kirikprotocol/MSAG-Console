package mobi.eyeline.smsquiz.replystats;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

import java.util.Date;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ReplyDataItem extends AbstractDataItem  {
    public ReplyDataItem(Date replyDate, String msisdn, String message) {
      values.put("replyDate", replyDate);
      values.put("msisdn", msisdn);
      values.put("message", message);
  }
}
