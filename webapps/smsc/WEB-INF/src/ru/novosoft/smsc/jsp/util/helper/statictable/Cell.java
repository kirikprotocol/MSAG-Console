package ru.novosoft.smsc.jsp.util.helper.statictable;



/**
 * User: artem
 * Date: 20.12.2006
 */

public abstract class Cell {

  private final String id;

  protected Cell(String id) {
    this.id = (id == null) ? "" : id;
  }

  public String getId() {
    return id;
  }

}
