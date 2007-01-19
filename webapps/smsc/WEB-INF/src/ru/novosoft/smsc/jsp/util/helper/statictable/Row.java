package ru.novosoft.smsc.jsp.util.helper.statictable;

import ru.novosoft.smsc.jsp.util.helper.statictable.DataCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.Column;

import java.util.HashMap;

/**
 * User: artem
 * Date: 18.12.2006
 */
public class Row {

  private final HashMap cells = new HashMap();

  public Row() {
  }

  public void addCell(final Column column, final Cell cell) {
    cells.put(column.getId(), cell);
  }

  public Cell getCell(final Column column) {
    return (Cell)cells.get(column.getId());
  }
}
