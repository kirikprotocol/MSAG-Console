package ru.novosoft.smsc.jsp.smsc.snmp.tables.fields;

import ru.novosoft.smsc.jsp.smsc.snmp.tables.StatsTableCell;

/**
 * User: artem
 * Date: 18.10.2006
 */

public class IntCell implements StatsTableCell {

  private final int value;

  public IntCell(final int value) {
    this.value = value;
  }

  public String getValueAsString() {
    return String.valueOf(value);
  }

  public Comparable getValue() {
    return new Integer(value);
  }
}
