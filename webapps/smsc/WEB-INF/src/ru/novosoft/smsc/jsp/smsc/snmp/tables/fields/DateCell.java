package ru.novosoft.smsc.jsp.smsc.snmp.tables.fields;

import ru.novosoft.smsc.jsp.smsc.snmp.tables.StatsTableCell;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * User: artem
 * Date: 18.10.2006
 */

public class DateCell implements StatsTableCell {

  private final Date value;
  private final SimpleDateFormat dateFormatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

  public DateCell(final Date value) {
    this.value = value;
  }

  public String getValueAsString() {
    return dateFormatter.format(value);
  }

  public Comparable getValue() {
    return value;
  }
}
