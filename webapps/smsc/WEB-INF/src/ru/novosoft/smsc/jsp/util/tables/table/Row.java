package ru.novosoft.smsc.jsp.util.tables.table;

import ru.novosoft.smsc.jsp.util.tables.table.DataCell;
import ru.novosoft.smsc.jsp.util.tables.table.Column;

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
