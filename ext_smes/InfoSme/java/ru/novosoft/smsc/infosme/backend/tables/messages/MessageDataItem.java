package ru.novosoft.smsc.infosme.backend.tables.messages;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;
import ru.novosoft.smsc.infosme.backend.Message;

import java.util.Date;
import java.text.SimpleDateFormat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.05.2004
 * Time: 20:05:15
 * To change this template use Options | File Templates.
 */
public class MessageDataItem extends AbstractDataItem
{
  public MessageDataItem(Message message)
  {
    values.put("id", new Long(message.id));
    values.put("abonent", message.abonent);
    values.put("status", new Integer(message.status));
    values.put("sendDate", message.sendDate);
    values.put("message", message.message);
  }

  public MessageDataItem(long id, String abonent, String message, Date sendDate, byte status)
  {
    values.put("id", new Long(id));
    values.put("abonent", abonent);
    values.put("status", new Integer(status));
    values.put("sendDate", sendDate);
    values.put("message", message);
  }

  public long getId() {
    return ((Long)values.get("id")).longValue();
  }
  public String getIdString() {
    return (""+getId());
  }

  public String getAbonent() {
    return (String) values.get("abonent");
  }

  public String getStatus()
  {
    switch (((Integer)values.get("status")).intValue())
    {
      case Message.MESSAGE_NEW_STATE:       return "NEW";
      case Message.MESSAGE_WAIT_STATE:      return "WAIT";
      case Message.MESSAGE_ENROUTE_STATE:   return "ENROUTE";
      case Message.MESSAGE_DELIVERED_STATE: return "DELIVERED";
      case Message.MESSAGE_EXPIRED_STATE:   return "EXPIRED";
      case Message.MESSAGE_FAILED_STATE:    return "FAILED";
      case Message.MESSAGE_UNDEFINED_STATE:
      default:                              return "UNKNOWN";
    }
  }

  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
  public String getDate() {
    Date date = (Date) values.get("sendDate");
    if (date == null) return "<undefined>";
    SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
    return formatter.format(date);
  }

  public String getMessage() {
    return (String) values.get("message");
  }
}
