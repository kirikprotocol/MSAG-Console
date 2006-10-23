package ru.novosoft.smsc.jsp.smsc.snmp.tables;

import java.util.HashMap;

/**
 * User: artem
 * Date: 16.10.2006
 */

public class StatsTableRow {

  private final HashMap fields = new HashMap();
  private final StatsTableScheme scheme;

  public StatsTableRow(final StatsTableScheme scheme) {
    this.scheme = scheme;
  }

  public StatsTableScheme getScheme() {
    return scheme;
  }

  public void addFieldValue(String fieldName, StatsTableCell value) {
    if (!scheme.hasField(fieldName))
      throw new IllegalArgumentException("Scheme does not contain field: " + fieldName);
    fields.put(fieldName, value);
  }

  public void clear() {
    fields.clear();
  }

  public Comparable getValue(final String fieldName) {
    if (!scheme.hasField(fieldName))
      throw new IllegalArgumentException("Scheme does not contain field: " + fieldName);
    final StatsTableCell cell = (StatsTableCell)fields.get(fieldName);
    return cell != null ? cell.getValue() : null;
  }

  public String getValueAsString(final String fieldName) {
    if (!scheme.hasField(fieldName))
      throw new IllegalArgumentException("Scheme does not contain field: " + fieldName);
    final StatsTableCell cell = (StatsTableCell)fields.get(fieldName);
    return (cell != null) ? cell.getValueAsString() : "";
  }
}
