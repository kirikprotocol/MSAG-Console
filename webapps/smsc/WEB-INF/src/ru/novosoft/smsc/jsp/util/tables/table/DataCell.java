package ru.novosoft.smsc.jsp.util.tables.table;



/**
 * User: artem
 * Date: 18.12.2006
 */

public class DataCell extends Cell{

  private final boolean selectable;

  protected DataCell(String id, final boolean selectable) {
    super(id);
    this.selectable = selectable;
  }

  public boolean isSelectable() {
    return selectable;
  }
}
