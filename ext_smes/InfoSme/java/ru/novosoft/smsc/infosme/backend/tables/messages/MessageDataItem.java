package ru.novosoft.smsc.infosme.backend.tables.messages;

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;
import ru.novosoft.smsc.infosme.backend.Message;

import java.util.Date;

/**
 * User: artem
 * Date: 29.05.2008
 */

public class MessageDataItem extends AbstractDataItem {

  public MessageDataItem(long id, String taskId, int state, Date date, String msisdn, String region, String message) {
    values.put("id", new Long(id));
    values.put("taskId", taskId);
    values.put("state", Message.State.getById(state));
    values.put("date", date);
    values.put("msisdn", msisdn);
    values.put("region", region);
    values.put("message", message);
  }
}
