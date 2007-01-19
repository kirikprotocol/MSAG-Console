package ru.novosoft.smsc.jsp.util.helper.dynamictable;

import java.util.HashMap;

/**
 * User: artem
 * Date: 16.01.2007
 */
public class Row {
  private final HashMap cells = new HashMap();

  public void addValue(Column column, Object value) {
    if (value != null && column != null)
      cells.put(column, value);
  }

  public Object getValue(Column column) {
    return (Object)cells.get(column);
  }

  public boolean isEmpty() {
    return cells.isEmpty();
  }
}
