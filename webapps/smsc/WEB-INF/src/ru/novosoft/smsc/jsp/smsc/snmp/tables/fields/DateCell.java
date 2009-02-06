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
  private final SimpleDateFormat dateFormatter;

  public DateCell(Date value, SimpleDateFormat df) {
    this.value = value;
    this.dateFormatter = df;
  }

  public DateCell(final Date value) {
    this(value, new SimpleDateFormat("dd.MM.yyyy HH:mm:ss"));
  }

  public String getValueAsString() {
    return dateFormatter.format(value);
  }

  public Comparable getValue() {
    return value;
  }
}
