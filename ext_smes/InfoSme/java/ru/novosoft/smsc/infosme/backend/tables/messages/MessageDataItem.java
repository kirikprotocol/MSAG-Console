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
    values.put(MessageDataSource.ID, new Long(id));
    values.put(MessageDataSource.TASK_ID, taskId);
    values.put(MessageDataSource.STATE, Message.State.getById(state));
    values.put(MessageDataSource.DATE, date);
    values.put(MessageDataSource.MSISDN, msisdn);
    values.put(MessageDataSource.REGION, region);
    values.put(MessageDataSource.MESSAGE, message);
  }
}
