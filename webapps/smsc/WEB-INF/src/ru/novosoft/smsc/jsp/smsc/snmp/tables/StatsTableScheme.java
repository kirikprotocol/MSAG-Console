package ru.novosoft.smsc.jsp.smsc.snmp.tables;

import java.util.Set;
import java.util.TreeSet;
import java.util.Iterator;
import java.util.ArrayList;

/**
 * User: artem
 * Date: 16.10.2006
 */

public class StatsTableScheme {

  private final ArrayList fieldNames = new ArrayList();

  protected String addField(String field) {
    fieldNames.add(field);
    return field;
  }

  public boolean hasField(String name) {
    return fieldNames.contains(name);
  }

  public Iterator getFields() {
    return fieldNames.iterator();
  }
}
