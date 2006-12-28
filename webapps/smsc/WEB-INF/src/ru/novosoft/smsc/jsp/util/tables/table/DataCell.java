package ru.novosoft.smsc.jsp.util.tables.table;

/**
 * User: artem
 * Date: 18.12.2006
 */

public class DataCell extends Cell{

  private final boolean selectable;

  protected DataCell(final Object value, final boolean selectable) {
    super(value);
    this.selectable = selectable;
  }

  public boolean isSelectable() {
    return selectable;
  }
}
