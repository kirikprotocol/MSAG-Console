package ru.novosoft.smsc.jsp.util.tables.table;

/**
 * User: artem
 * Date: 20.12.2006
 */

public class Cell {
  private final Object object;

  protected Cell(Object object) {
    this.object = object;
  }

  public Object getObject() {
    return object;
  }
}
