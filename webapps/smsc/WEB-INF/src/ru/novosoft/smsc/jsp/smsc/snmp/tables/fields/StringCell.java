package ru.novosoft.smsc.jsp.smsc.snmp.tables.fields;

import ru.novosoft.smsc.jsp.smsc.snmp.tables.StatsTableCell;

/**
 * User: artem
 * Date: 18.10.2006
 */

public class StringCell implements StatsTableCell {

  private final String value;

  public StringCell(final String value) {
    this.value = value;
  }

  public String getValueAsString() {
    return value;
  }

  public Comparable getValue() {
    return value;
  }
}
