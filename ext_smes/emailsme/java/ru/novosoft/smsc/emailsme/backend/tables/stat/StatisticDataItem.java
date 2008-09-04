package ru.novosoft.smsc.emailsme.backend.tables.stat;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

import java.util.Date;

/**
 * User: artem
 * Date: 04.09.2008
 */

public class StatisticDataItem extends AbstractDataItem {

  public StatisticDataItem(Date date, int hour) {
    values.put("date", date);
    values.put("hour", new Integer(hour));
  }

  public void setValue(String key, Object value) {
    System.out.println("set " + key + " " + value);
    values.put(key, value);
  }

  public Date getDate() {
    return (Date)values.get("date");
  }

  public int getHour() {
    return ((Integer)values.get("hour")).intValue();
  }

  public int getReceivedOk() {
    Integer v = (Integer)values.get("sms_received_ok");
    return v == null ? 0 : v.intValue();
  }

  public int getReceivedFail() {
    Integer v = (Integer)values.get("sms_received_fail");
    return v == null ? 0 : v.intValue();
  }

  public int getTransmitedOk() {
    Integer v = (Integer)values.get("sms_transmitted_ok");
    return v == null ? 0 : v.intValue();
  }

  public int getTransmitedFail() {
    Integer v = (Integer)values.get("sms_transmitted_fail");
    return v == null ? 0 : v.intValue();
  }
}
